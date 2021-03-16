/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 NJ
 * Copyright (C) 2007 takka <takka@tfact.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/******************************************************************************
 * adhoc.c
 * PSP ad hoc communication control
 ******************************************************************************/

/******************************************************************************
 * List of includes
 ******************************************************************************/

#include <pspkernel.h>
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <psputility_netmodules.h>
#include <pspnet_adhocctl.h>
#include <pspwlan.h>
#include <pspnet_adhocmatching.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "adhoc.h"
#include "common.h"
#include "memory.h"
#include "draw.h"
#include "main.h"
#include "gu.h"
#include "fbm_print.h"
#include "input.h"

/******************************************************************************
 * Definition of macros, etc.
 ******************************************************************************/
#define NUM_ENTRIES 16

#define MODE_LOBBY 0
#define MODE_P2P 1

#define PSP_LISTING 1
#define PSP_SELECTED 2
#define PSP_SELECTING 3
#define PSP_WAIT_EST 4
#define PSP_ESTABLISHED 5

#define ADHOC_BUFFER_SIZE 0x400
#define PDP_BUFFER_SIZE (ADHOC_BUFFER_SIZE * 2)
#define PDP_PORT (0x309)

#define ADHOC_TIMEOUT 30 * 1000000
#define ADHOC_BLOCKSIZE 0x400

#define PRODUCT "UOgpSPkai"

/*--------------------------------------------------------
  Function parameter definition
--------------------------------------------------------*/
#define MATCHING_CREATE_PARAMS \
  3,                           \
      0xa,                     \
      0x22b,                   \
      0x800,                   \
      0x2dc6c0,                \
      0x5b8d80,                \
      3,                       \
      0x7a120,                 \
      (pspAdhocMatchingCallback)matching_callback

#define MATCHING_START_PARAMS    \
  matching_id,                   \
      0x10,                      \
      0x2000,                    \
      0x10,                      \
      0x2000,                    \
      strlen(matching_data) + 1, \
      (char *)matching_data

u32 g_multi_id;
u32 g_adhoc_transfer_flag;
u32 g_adhoc_link_flag;

static u32 s_multi_mode;

/***************************************************************************
 Static wide area variable
 ***************************************************************************/

static int s_mode;
static int server;
static int pdp_id;

static unsigned char mac[6];
static char mymac[6];
static unsigned char ssid[8];
static u32 unk1;
static u32 match_event;
static u32 match_opt_len;
static char match_opt_data[1000];
static char matching_data[32];
static u32 match_changed;
static int matching_id;

static struct psplist_t
{
  char name[48];
  char mac[6];
} psplist[NUM_ENTRIES];

static int max;
static int pos;

static int adhoc_initialized = 0;
static unsigned char adhoc_buffer[ADHOC_BUFFER_SIZE];
static unsigned char adhoc_work[ADHOC_BUFFER_SIZE];

static SceUID network_thread;
volatile static u32 s_net_thread_exit_flag; // Net thread termination flag.

/***************************************************************************
 Local function
 ***************************************************************************/
static int net_thread(SceSize args, void *argp);
void adhoc_multi();

/*--------------------------------------------------------
 Progress bar initialization
 --------------------------------------------------------*/
static void adhoc_init_progress(u32 total, char *text)
{
  char buf[MAX_FILE];

  // Screen settings
  //    load_background(WP_LOGO);
  //    video_copy_rect(work_frame, draw_frame, &full_rect, &full_rect);

  // Icon display
  //    small_icon(6, 3, UI_COLOR(UI_PAL_TITLE), ICON_SYSTEM);
  sprintf(buf, "AdHoc - %s", gamepak_title);
  // Character display
  //    uifont_print(32, 5, UI_COLOR(UI_PAL_TITLE), buf);

  // Screen settings
  //    video_copy_rect(draw_frame, work_frame, &full_rect, &full_rect);

  // Progress bar display
  init_progress(total, text);
}

/*--------------------------------------------------------
 Clear list
 --------------------------------------------------------*/
static void clear_psp_list(void)
{
  max = 0;
  pos = 0;
  memset(&psplist, 0, sizeof(psplist));
}

/*--------------------------------------------------------
 Add to list
 --------------------------------------------------------*/
static u32 add_psp(unsigned char *l_mac, char *name, u32 length)
{
  u32 i;

  if (max == NUM_ENTRIES)
    return 0;
  if (length == 1)
    return 0;

  for (i = 0; i < max; i++)
  {
    if (memcmp(psplist[i].mac, l_mac, 6) == 0)
      return 0;
  }

  memcpy(psplist[max].mac, l_mac, 6);

  if (length)
  {
    if (length < 47)
      strcpy(psplist[max].name, name);
    else
      strncpy(psplist[max].name, name, 47);
  }
  else
    psplist[max].name[0] = '\0';

  max++;

  return 1;
}

/*--------------------------------------------------------
 Remove from list
 --------------------------------------------------------*/
static u32 del_psp(unsigned char *l_mac)
{
  u32 i, j;

  for (i = 0; i < max; i++)
  {
    if (memcmp(psplist[i].mac, l_mac, 6) == 0)
    {
      if (i != max - 1)
      {
        for (j = i + 1; j < max; j++)
        {
          memcpy(psplist[j - 1].mac, psplist[j].mac, 6);
          strcpy(psplist[j - 1].name, psplist[j].name);
        }
      }

      if (pos == i)
        pos = 0;
      if (pos > i)
        pos--;
      max--;

      return 0;
    }
  }

  return -1;
}

/*--------------------------------------------------------
 Show list
 --------------------------------------------------------*/
static void display_psp_list(u32 top, u32 rows)
{
  if (max == 0)
  {
    // Character display
    msg_printf("WAITING_FOR_ANOTHER_PSP_TO_JOIN\n");
  }
  else
  {
    u32 i;
    char temp[20];

    // Screen display
    msg_screen_draw();

    scrollbar(470, 26, 479, 270, max, rows, pos);

    for (i = 0; i < rows; i++)
    {
      if ((top + i) >= max)
        break;

      sceNetEtherNtostr((u8 *)psplist[top + i].mac, temp);

      if ((top + i) == pos)
      {
        // Character display
        PRINT_STRING(temp, COLOR16(31, 0, 0), 24, 40 + (i + 2) * 17);
        PRINT_STRING(psplist[top + i].name, COLOR16(31, 0, 0), 190, 40 + (i + 2) * 17);
      }
      else
      {
        // Character display
        PRINT_STRING(temp, COLOR16(31, 0, 0), 24, 40 + (i + 2) * 17);
        PRINT_STRING(psplist[top + i].name, COLOR16(31, 0, 0), 190, 40 + (i + 2) * 17);
      }
    }

    // Screen switching
    flip_screen();
  }
}

/*--------------------------------------------------------
 Get information on selected PSP
 --------------------------------------------------------*/
static u32 GetPspEntry(unsigned char *l_mac, char *name)
{
  if (max == 0)
    return -1;

  memcpy(l_mac, psplist[pos].mac, 6);
  strcpy(name, psplist[pos].name);

  return 1;
}

/*--------------------------------------------------------
 Matching callback
 --------------------------------------------------------*/
static void matching_callback(int unk1, int event, unsigned char *l_mac, int optLen, char *optData)
{
  switch (event)
  {
  case PSP_ADHOC_MATCHING_EVENT_JOIN:
    add_psp(l_mac, optData, optLen);
    break;

  case PSP_ADHOC_MATCHING_EVENT_DISCONNECT:
    del_psp(l_mac);
    break;

  default:
    unk1 = unk1;
    match_event = event;
    match_opt_len = optLen;
    strncpy(match_opt_data, optData, optLen);
    memcpy(mac, l_mac, sizeof(char) * 6);
    match_changed = 1;
    break;
  }
}

/***************************************************************************
 AdHoc interface function
 ***************************************************************************/

/*--------------------------------------------------------
 Module loading
 --------------------------------------------------------*/
u32 load_adhoc_modules(void)
{
  g_adhoc_link_flag = 0;
  g_multi_id = 0;

  if (sceKernelDevkitVersion() >= 0x02000010)
  {
    int error;

    if ((error = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON)) < 0)
      return error;

    if ((error = sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC)) < 0)
      return error;

    return 0;
  }
  return -1;
}

/*--------------------------------------------------------
 Initialize
 --------------------------------------------------------*/
u32 adhoc_init(const char *l_matching_data)
{
  struct productStruct product;
  int error = 0, state = 0;
  unsigned char l_mac[6];
  const char *unknown = "";
  char message[256];

  g_adhoc_link_flag = 0;

  // Creating a communication thread
  network_thread = sceKernelCreateThread("Net thread", net_thread, 0x13, 0x2000, 0, NULL);
  if (network_thread < 0)
  {
    quit(0);
  }

  //Start thread
  sceKernelStartThread(network_thread, 0, 0);

  s_mode = MODE_LOBBY;
  server = 0;
  adhoc_initialized = 0;

  unk1 = 0;
  match_event = 0;
  match_opt_len = 0;
  match_changed = 0;
  memset(mac, 0, sizeof(mac));
  memset(mymac, 0, sizeof(mymac));

  sprintf((char *)product.product, PRODUCT);
  product.unknown = 0;

  clear_psp_list();

  if (strlen(l_matching_data) == 0)
    return -1;

  strcpy(matching_data, l_matching_data);

  sprintf(message, "CONNECTING_TO_%s", "LOBBY");
  adhoc_init_progress(10, message);

  if ((error = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000)) == 0)
  {
    update_progress();
    if ((error = sceNetAdhocInit()) == 0)
    {
      update_progress();
      if ((error = sceNetAdhocctlInit(0x2000, 0x20, &product)) == 0)
      {
        update_progress();
        if ((error = sceNetAdhocctlConnect((char *)unknown)) == 0)
        {
          update_progress();
          do
          {
            if ((error = sceNetAdhocctlGetState(&state)) != 0)
              break;
            sceKernelDelayThread(1000000 / 60);
          } while (state != 1);
          if (!error)
          {
            update_progress();
            sceWlanGetEtherAddr(l_mac);
            update_progress();
            if ((pdp_id = sceNetAdhocPdpCreate(l_mac, PDP_PORT, PDP_BUFFER_SIZE, 0)) > 0)
            {
              update_progress();
              if ((error = sceNetAdhocMatchingInit(0x20000)) == 0)
              {
                update_progress();
                if ((matching_id = sceNetAdhocMatchingCreate(MATCHING_CREATE_PARAMS)) >= 0)
                {
                  update_progress();
                  if ((error = sceNetAdhocMatchingStart(MATCHING_START_PARAMS)) == 0)
                  {
                    update_progress();
                    show_progress("CONNECTED");
                    return 0;
                  }
                  sceNetAdhocMatchingDelete(matching_id);
                }
                error = 2;
                sceNetAdhocMatchingTerm();
              }
              sceNetAdhocPdpDelete(pdp_id, 0);
            }
            error = 1;
          }
          sceNetAdhocctlDisconnect();
        }
        sceNetAdhocctlTerm();
      }
      sceNetAdhocTerm();
    }
    sceNetTerm();
  }

  switch (error)
  {
  case 1:
    sprintf(message, "%s (PDP ID = %08x)", "failed", pdp_id);
    break;
  case 2:
    sprintf(message, "%s (Matching ID = %08x)", "failed", matching_id);
    break;
  default:
    sprintf(message, "%s (Error Code = %08x)", "failed", error);
    break;
  }

  show_progress(message);
  error_msg("");
  return -1;
}

/*--------------------------------------------------------
 Disconnect
 --------------------------------------------------------*/
u32 adhoc_term(void)
{
  if (adhoc_initialized > 0)
  {
    char message[256];

    sprintf(message, "DISCONNECTING %s", server ? "Client" : "Server");
    adhoc_init_progress(5, message);

    sceNetAdhocctlDisconnect();
    update_progress();

    sceNetAdhocPdpDelete(pdp_id, 0);
    update_progress();

    sceNetAdhocctlTerm();
    update_progress();

    sceNetAdhocTerm();
    update_progress();

    sceNetTerm();
    update_progress();

    show_progress("DISCONNECTED");

    adhoc_initialized = 0;
  }

  g_adhoc_link_flag = 0;
  g_multi_id = 0;

  return 0;
}

/*--------------------------------------------------------
 Disconnect from the lobby
 --------------------------------------------------------*/
static void adhoc_disconnect(void)
{
  char message[256];

  sprintf(message, "DISCONNECTING_FROM_%s", "LOBBY");
  adhoc_init_progress(8, message);

  sceNetAdhocMatchingStop(matching_id);
  update_progress();

  sceNetAdhocMatchingDelete(matching_id);
  update_progress();

  sceNetAdhocMatchingTerm();
  update_progress();

  sceNetAdhocctlDisconnect();
  update_progress();

  sceNetAdhocPdpDelete(pdp_id, 0);
  update_progress();

  sceNetAdhocctlTerm();
  update_progress();

  sceNetAdhocTerm();
  update_progress();

  sceNetTerm();
  update_progress();

  show_progress("DISCONNECTED");
}

/*--------------------------------------------------------
  Disconnect from the lobby and start P2P
--------------------------------------------------------*/
static int adhoc_start_p2p(void)
{
  int error = 0, state = 1;
  unsigned char l_mac[6];
  char message[256];

  sprintf(message, "DISCONNECTING_FROM_%s", "LOBBY");
  adhoc_init_progress(6, message);

  sceNetAdhocMatchingStop(matching_id);
  update_progress();

  sceNetAdhocMatchingDelete(matching_id);
  update_progress();

  sceNetAdhocMatchingTerm();
  update_progress();

  sceNetAdhocPdpDelete(pdp_id, 0);
  update_progress();

  sceNetAdhocctlDisconnect();
  update_progress();

  do
  {
    if ((error = sceNetAdhocctlGetState(&state)) != 0)
      break;
    sceKernelDelayThread(1000000 / 60);
  } while (state == 1);

  update_progress();
  show_progress("DISCONNECTED");

  s_mode = MODE_P2P;
  sprintf(message, "CONNECTING_TO_%s", server ? "CLIENT" : "SERVER");
  adhoc_init_progress(4, message);

  if ((error = sceNetAdhocctlConnect((char *)ssid)) == 0)
  {
    update_progress();
    do
    {
      if ((error = sceNetAdhocctlGetState(&state)) != 0)
        break;
      sceKernelDelayThread(1000000 / 60);
    } while (state != 1);

    if (!error)
    {
      update_progress();

      sceWlanGetEtherAddr(l_mac);
      memcpy(mymac, l_mac, 6);
      update_progress();

      if ((pdp_id = sceNetAdhocPdpCreate(l_mac, PDP_PORT, PDP_BUFFER_SIZE, 0)) > 0)
      {
        update_progress();
        adhoc_initialized = 2;

        show_progress("WAITING_FOR_SYNCHRONIZATION");
        if ((error = adhoc_sync()) == 0)
        {
          g_adhoc_link_flag = 1;
          return server;
        }
      }
      else
      {
        error = 1;
      }
    }
    sceNetAdhocctlDisconnect();

    if (state == 1)
    {
      do
      {
        if ((error = sceNetAdhocctlGetState(&state)) != 0)
          break;
        sceKernelDelayThread(1000000 / 60);
      } while (state == 1);
    }
  }

  sceNetAdhocctlTerm();
  sceNetAdhocTerm();
  sceNetTerm();

  adhoc_initialized = 0;
  g_adhoc_link_flag = 0;
  g_multi_id = 0;

  switch (error)
  {
  case 1:
    sprintf(message, "%s (PDP ID = %08x)", "FAILED", pdp_id);
    break;
  default:
    sprintf(message, "%s (Error Code = %08x)", "FAILED", error);
    break;
  }

  show_progress(message);

  //wait;

  return -1;
}

/*--------------------------------------------------------
  Select connection destination
--------------------------------------------------------*/
u32 adhoc_select(void)
{
  int top = 0;
  int rows = 11;
  int currentState = PSP_LISTING;
  int prev_max = 0;
  int update = 1;
  unsigned char l_mac[6];
  char name[64];
  char temp[64];
  char title[256];
  gui_action_type button;

  sprintf(title, "AdHoc - %s", gamepak_title);
  msg_screen_init(title);

  while (1)
  {
    button = get_gui_input();

    msg_set_text_color(COLOR16(0, 0, 0));

    switch (currentState)
    {
    case PSP_LISTING:
      server = 0;
      g_multi_id = 1;
      if (update)
      {
        msg_screen_init(title);
        msg_printf("SELECT_A_SERVER_TO_CONNECT_TO");
        msg_printf("\n");
        display_psp_list(top, rows);
        update = 0;
      }
      if (button == CURSOR_UP)
      {
        if (pos > 0)
          pos--;
        update = 1;
      }
      else if (button == CURSOR_DOWN)
      {
        if (pos < max - 1)
          pos++;
        update = 1;
      }
      else if (button == CURSOR_SELECT)
      {
        if (GetPspEntry(l_mac, name) > 0)
        {
          if (strcmp(name, matching_data) == 0)
          {
            currentState = PSP_SELECTING;
            sceNetAdhocMatchingSelectTarget(matching_id, l_mac, 0, NULL);
            update = 1;
          }
        }
      }
      else if (button == CURSOR_EXIT)
      {
        msg_set_text_color(COLOR16(0, 0, 0));
        adhoc_disconnect();
        //        pad_wait_clear();
        return -1;
      }
      if (match_changed)
      {
        if (match_event == PSP_ADHOC_MATCHING_EVENT_ACCEPT)
        {
          memcpy(l_mac, mac, 6);
          strcpy(name, match_opt_data);
          currentState = PSP_SELECTED;
        }
        update = 1;
      }
      break;

    case PSP_SELECTING:
      if (update)
      {
        msg_screen_init(title);
        sceNetEtherNtostr(l_mac, temp);
        msg_printf("WAITING_FOR_%s_TO_ACCEPT_THE_CONNECTION\n", temp);
        msg_printf("TO_CANCEL_PRESS_CROSS");
        update = 0;
      }
      if (button == CURSOR_EXIT)
      {
        sceNetAdhocMatchingCancelTarget(matching_id, l_mac);
        currentState = PSP_LISTING;
        update = 1;
      }
      if (match_changed)
      {
        switch (match_event)
        {
        case PSP_ADHOC_MATCHING_EVENT_ACCEPT:
          sceNetAdhocMatchingCancelTarget(matching_id, l_mac);
          break;

        case PSP_ADHOC_MATCHING_EVENT_COMPLETE:
          currentState = PSP_ESTABLISHED;
          break;

        case PSP_ADHOC_MATCHING_EVENT_REJECT:
          currentState = PSP_LISTING;
          break;
        }
        update = 1;
      }
      break;

    case PSP_SELECTED:
      server = 1;
      g_multi_id = 0;
      if (update)
      {
        msg_screen_init(title);
        sceNetEtherNtostr(l_mac, temp);
        msg_printf("%s_HAS_REQUESTED_A_CONNECTION\n", temp);
        msg_printf("TO_ACCEPT_THE_CONNECTION_PRESS_CIRCLE\nTO_CANCEL_PRESS_CROSS\n");
        update = 0;
      }
      if (button == CURSOR_EXIT)
      {
        sceNetAdhocMatchingCancelTarget(matching_id, l_mac);
        currentState = PSP_LISTING;
        update = 1;
      }
      else if (button == CURSOR_SELECT)
      {
        sceNetAdhocMatchingSelectTarget(matching_id, l_mac, 0, NULL);
        currentState = PSP_WAIT_EST;
        update = 1;
      }
      if (match_changed)
      {
        if (match_event == PSP_ADHOC_MATCHING_EVENT_CANCEL)
        {
          currentState = PSP_LISTING;
        }
        update = 1;
      }
      break;

    case PSP_WAIT_EST:
      if (match_changed)
      {
        if (match_event == PSP_ADHOC_MATCHING_EVENT_COMPLETE)
        {
          currentState = PSP_ESTABLISHED;
        }
        update = 1;
      }
      break;
    }

    match_changed = 0;
    if (currentState == PSP_ESTABLISHED)
      break;

    if (top > max - rows)
      top = max - rows;
    if (top < 0)
      top = 0;
    if (pos >= top + rows)
      top = pos - rows + 1;
    if (pos < top)
      top = pos;

    if (max != prev_max)
    {
      prev_max = max;
      update = 1;
    }

    sceDisplayWaitVblankStart();
  }

  msg_set_text_color(COLOR16(0, 0, 0));

  if (server)
    sceWlanGetEtherAddr(l_mac);

  sceNetEtherNtostr(l_mac, temp);

  ssid[0] = temp[9];
  ssid[1] = temp[10];
  ssid[2] = temp[12];
  ssid[3] = temp[13];
  ssid[4] = temp[15];
  ssid[5] = temp[16];
  ssid[6] = '\0';

  return adhoc_start_p2p();
}

/*--------------------------------------------------------
  Send data
--------------------------------------------------------*/
u32 adhocSend(void *buffer, u32 length, u32 type)
{
  int error;

  memset(adhoc_buffer, 0, ADHOC_BUFFER_SIZE);

  adhoc_buffer[0] = type;
  memcpy(&adhoc_buffer[1], buffer, length);

  if ((error = sceNetAdhocPdpSend(pdp_id, mac, PDP_PORT, adhoc_buffer, length + 1, 0, 1)) < 0)
    return error;

  return length;
}

/*--------------------------------------------------------
  Receive data
--------------------------------------------------------*/
u32 adhocRecv(void *buffer, u32 timeout, u32 type)
{
  int error;
  int length = ADHOC_BUFFER_SIZE;
  unsigned short port = 0;
  unsigned char l_mac[6];

  memset(adhoc_buffer, 0, ADHOC_BUFFER_SIZE);

  if ((error = sceNetAdhocPdpRecv(pdp_id, l_mac, &port, adhoc_buffer, &length, timeout, 0)) < 0)
    return error;

  if (adhoc_buffer[0] & type)
  {
    memcpy(buffer, &adhoc_buffer[1], length - 1);
    return length - 1;
  }

  return -1;
}

/*--------------------------------------------------------
  Send data and wait until it receives an ack
--------------------------------------------------------*/
int adhocSendRecvAck(void *buffer, int length, int timeout, int type)
{
  int temp_length = length;
  int sent_length = 0;
  int error = 0;
  unsigned char *buf = (unsigned char *)buffer;

  do
  {
    if (temp_length > ADHOC_BUFFER_SIZE - 1)
      temp_length = ADHOC_BUFFER_SIZE - 1;

    adhocSend(buf, temp_length, type);

    if ((error = adhocRecv(adhoc_work, timeout, ADHOC_DATATYPE_ACK)) != 4)
      return error;

    if (*(int *)adhoc_work != sent_length + temp_length)
      return -1;

    buf += temp_length;
    sent_length += temp_length;
    temp_length = length - sent_length;

  } while (sent_length < length);

  return sent_length;
}

/*--------------------------------------------------------
  Wait for data to be received and send ack
--------------------------------------------------------*/
int adhocRecvSendAck(void *buffer, int length, int timeout, int type)
{
  int temp_length = length;
  int rcvd_length = 0;
  int error = 0;
  unsigned char *buf = (unsigned char *)buffer;

  do
  {
    if (temp_length > ADHOC_BUFFER_SIZE - 1)
      temp_length = ADHOC_BUFFER_SIZE - 1;

    if ((error = adhocRecv(buf, timeout, type)) != temp_length)
      return error;

    *(int *)adhoc_work = rcvd_length + temp_length;
    adhocSend(adhoc_work, 4, ADHOC_DATATYPE_ACK);

    buf += temp_length;
    rcvd_length += temp_length;
    temp_length = length - rcvd_length;

  } while (rcvd_length < length);

  return rcvd_length;
}

/*--------------------------------------------------------
  Wait for synchronization with the other party
--------------------------------------------------------*/
int adhoc_sync(void)
{
  int size = 0;
  int retry = 60;

  if (server)
  {
    while (retry--)
    {
      adhocSend(adhoc_work, 1, ADHOC_DATATYPE_SYNC);

      if (adhocRecv(adhoc_work, 1000000, ADHOC_DATATYPE_SYNC) == 1)
        goto check_packet;
    }
  }
  else
  {
    while (retry--)
    {
      if (adhocRecv(adhoc_work, 1000000, ADHOC_DATATYPE_SYNC) == 1)
      {
        adhocSend(adhoc_work, 1, ADHOC_DATATYPE_SYNC);
        goto check_packet;
      }
    }
  }

  return -1;

check_packet:
  while (1)
  {
    pdpStatStruct pdpStat;

    size = sizeof(pdpStat);

    if (sceNetAdhocGetPdpStat(&size, &pdpStat) >= 0)
    {
      // Discard extra packets
      if (pdpStat.rcvdData == ADHOC_DATASIZE_SYNC)
        adhocRecv(adhoc_work, 10, ADHOC_DATATYPE_SYNC);
      else
        break;
    }

    //    if (Loop != LOOP_EXEC) return 0;

    sceKernelDelayThread(100);
  }

  return 0;
}

/*--------------------------------------------------------
Receives the specified size of data or the buffer is empty
   Wait until
--------------------------------------------------------*/
void adhoc_wait(int data_size)
{
  pdpStatStruct pdpStat;
  int size = sizeof(pdpStat);

  if (data_size > ADHOC_BUFFER_SIZE)
    data_size = ADHOC_BUFFER_SIZE;

  while (1)
  {
    if (sceNetAdhocGetPdpStat(&size, &pdpStat) >= 0)
    {
      if (pdpStat.rcvdData == 0 || (int)pdpStat.rcvdData == data_size)
        break;
      else
        adhocRecv(adhoc_work, 0, ADHOC_DATATYPE_ANY);
    }

    //    if (Loop != LOOP_EXEC) break;

    sceKernelDelayThread(100);
  }
}

#define MAX_MULTI_ID 2

/*--------------------------------------------------------
 通信 スレッド // Communication thread
 --------------------------------------------------------*/
// 通信データはmulti_id + コマンド + word data の4バイト

// g_multi_mode = 0x00 NOP 非通信時はこのモード
//  親機は子機にNOPを送信、最終子機からACKを受け取った後、再度NOPモードに移行する
//  最大３回送信を行う
//
//  子機はNOPを受取り後、ACKを返し、再度NOPモードに移行する
//  １回受信を行う
//
// g_multi_mode = 0x01 START マルチ通信開始時のモード
//  親機は子機にSTARTを送信、受信データをすべて0xFFFFに設定、最終子機からACKを受け取った後、SNEDモードに移行する
//  最大３回送信を行う
//
//  子機はSTARTを受け取り後、ACKを返し、受信データをすべて0xFFFFに設定、RECVモードに移行する
//  １回受信を行う
//
// g_multi_mode = 0x02 END マルチ通信終了
//  親機は子機にENDを送信、最終子機からACKを受け取った後、通信ビットを０にして、NOPモードに移行する
//  最大３回送信を行う
//
//  子機はENDを受け取り後、ACKを返し、NOPモードに移行する
//  １回受信を行う
//
// g_multi_mode = 0x1? SEND マルチデータ送信
//  親機は子機に実データを送信、子機からACKを受け取った後、RECVモードに移行する
//
//  子機は他の機に実データを送信、ACKを受信したあと、RECVモードに移行
//  自信が最終idの場合はENDモードに移行する
//
// recv_multi = 0x2? RECV マルチデータ受信
//  親機は子機から実データを受信、ACKを送信したあと、下８ビットが最終idと同じ場合ENDモードに移行、
//  そのほかは再度RECVモード
//
//  子機は親機から実データを受信、ACKを送信したあと、下８ビットが最終idと同じ場合ENDモードに移行、
//  そのほかは再度RECVモードに移行

//  送信関数と受信関数を作成
//  メインルーチン内で親子のフラグとidにてswitchで振り分け
//
//データの送受信について
//
//　送信
//　　必ず(通信台数-1)回送信を行う
//　　エラーの場合はOKになるまで送信を行う
//　　※ACKにエラーコードを持たせる様に修正必要
//　　ただし、合計バイト数程度のチェックしか行わない
//
//　受信
//　　必ず(通信台数-1)回受信を行う
////////////////////////////////////////////////////////////////////////////
// Communication data is 4 bytes of multi_id + command + word data

// g_multi_mode = 0x00 NOP This mode when not communicating
// The master unit sends NOP to the slave unit, receives an ACK from the final slave unit, and then shifts to NOP mode again.
// Send up to 3 times
// //
// After receiving NOP, the slave unit returns ACK and shifts to NOP mode again.
// Receive once
// //
// g_multi_mode = 0x01 START Mode at start of multi-communication
// The master unit sends START to the slave unit, sets all received data to 0xFFFF, receives ACK from the final slave unit, and then shifts to SNED mode.
// Send up to 3 times
// //
// After receiving START, the slave unit returns ACK, sets all received data to 0xFFFF, and shifts to RECV mode.
// Receive once
// //
// g_multi_mode = 0x02 END Multi communication end
// The master unit sends END to the slave unit, receives ACK from the final slave unit, sets the communication bit to 0, and shifts to NOP mode.
// Send up to 3 times
// //
// After receiving END, the slave unit returns ACK and shifts to NOP mode.
// Receive once
// //
// g_multi_mode = 0x1? SEND Multi data transmission
// The master unit sends the actual data to the slave unit, receives an ACK from the slave unit, and then shifts to RECV mode.
// //
// The slave unit sends actual data to another unit, receives an ACK, and then shifts to RECV mode.
// Move to END mode if confidence is final id
// //
// recv_multi = 0x2? RECV multi data reception
// The master unit receives the actual data from the slave unit, sends an ACK, and then shifts to END mode if the lower 8 bits are the same as the final id.
// Other than that, RECV mode again
// //
// The slave unit receives the actual data from the master unit, sends an ACK, and then shifts to END mode if the lower 8 bits are the same as the final id.
// Other than that, switch to RECV mode again

// Create send and receive functions
// Sort by switch with parent and child flags and id in the main routine
// //
// About sending and receiving data
// //
// Send
// Be sure to send (number of communications-1) times
// In case of error, send until OK
// * Need to be modified so that ACK has an error code
// However, only check the total number of bytes
// //
// Receive
// Be sure to receive (number of communications-1) times

u32 multi_send(u32 id, u32 command, u8 data1, u8 data2);
u32 multi_recv(u8 *data);

static int net_thread(SceSize args, void *argp)
{
  s_net_thread_exit_flag = 0;
  g_multi_mode = MULTI_NOP;

  // メインループ
  while (s_net_thread_exit_flag == 0)
  {
    if (g_adhoc_link_flag == 1) // Communication is established
    {
      adhoc_multi();
    }
    sceKernelDelayThread(100); // TODO Weight value adjustment
  }

  sceKernelExitThread(0);
  return 0;
}

#define MULTI_REG_0 0x0120
#define MULTI_REG_1 0x0122
#define MULTI_REG_2 0x0124
#define MULTI_REG_3 0x0126
#define MULTI_DATA 0x012A

void adhoc_multi()
{
  u32 value;
  u32 length;
  u8 data1, data2;
  u8 work[ADHOC_BUFFER_SIZE];
  switch (g_multi_id)
  {
  case 0: // In the case of the master unit, actively process
    switch (g_multi_mode)
    {
    case MULTI_NOP: // Basically loop in NOP mode
      multi_send(g_multi_id, MULTI_NOP, 0x00, 0x00);
      g_multi_mode = MULTI_NOP;
      break;

    case MULTI_START: // Shift to START mode by bit operation of communication register
      multi_send(g_multi_id, MULTI_START, 0x00, 0x00);
      // Transfer flag set to 1
      g_adhoc_transfer_flag = 1;
      // Set all communication data registers to 0xFFFF
      ADDRESS16(io_registers, MULTI_REG_0) = 0xffff;
      ADDRESS16(io_registers, MULTI_REG_1) = 0xffff;
      ADDRESS16(io_registers, MULTI_REG_2) = 0xffff;
      ADDRESS16(io_registers, MULTI_REG_3) = 0xffff;
      // Shift to SEND mode
      g_multi_mode = MULTI_SEND;
      break;

    case MULTI_END: // After post-processing, go to NOP mode
      multi_send(g_multi_id, MULTI_END, 0x00, 0x00);
      // Transfer flag to 0
      g_adhoc_transfer_flag = 0;
      // Set the communication bit to 0
      value = ADDRESS16(io_registers, 0x128);
      value &= 0xff7f;
      ADDRESS16(io_registers, 0x128) = value;
      // Shift to NOP mode
      g_multi_mode = MULTI_NOP;
      break;

    case MULTI_SEND: // Send data to the handset
      value = ADDRESS16(io_registers, MULTI_DATA);
      data2 = value & 0xff;
      data1 = (value >> 8) & 0xff;
      multi_send(g_multi_id, MULTI_RECV, data1, data2);
      // Write your own value to the communication data register TODO
      ADDRESS16(io_registers, MULTI_REG_0) = value;
      g_multi_mode = MULTI_RECV; // Receive data from the handset in RECV mode
      break;

    case MULTI_RECV:
      multi_recv(work);
      value = (work[3] << 8) & work[4];
      // Write the value of the slave unit to the communication data register
      ADDRESS16(io_registers, MULTI_REG_1) = value;
      g_multi_mode = MULTI_END; // Shift to END mode
      break;

    case MULTI_KILL: // Send an end sign to the handset when physical communication is disconnected
      multi_send(g_multi_id, MULTI_KILL, 0x00, 0x00);
      g_adhoc_link_flag = 0;    // Turn off the flag and terminate the thread
      g_multi_mode = MULTI_NOP; // To NOP mode for the time being
      break;
    }
    break;

  case 1 ... 3:                     // In the case of a slave unit, process passively
    if (g_multi_mode != MULTI_SEND) // Receive data except in SEND mode
    {
      multi_recv(work);
      g_multi_mode = work[1]; // Retrieving a command
    }

    // Sort processing according to the received command
    switch (g_multi_mode)
    {
    case MULTI_NOP:
      break;

    case MULTI_START:
      // Set all communication data registers to 0xFFFF
      ADDRESS16(io_registers, MULTI_REG_0) = 0xffff;
      ADDRESS16(io_registers, MULTI_REG_1) = 0xffff;
      ADDRESS16(io_registers, MULTI_REG_2) = 0xffff;
      ADDRESS16(io_registers, MULTI_REG_3) = 0xffff;
      break;

    case MULTI_RECV:
      // Write the value of the master unit to the communication data register
      value = (work[3] << 8) & work[4];
      ADDRESS16(io_registers, MULTI_REG_0) = value;
      break;

    case MULTI_KILL:
      g_adhoc_link_flag = 0;    // Turn off the flag and terminate the thread
      g_multi_mode = MULTI_NOP; // To NOP mode for the time being
      break;

    case MULTI_SEND:
      value = ADDRESS16(io_registers, MULTI_DATA);
      data2 = value & 0xff;
      data1 = (value >> 8) & 0xff;
      multi_send(g_multi_id, MULTI_RECV, data1, data2);
      // Write your own value to the communication data register
      ADDRESS16(io_registers, MULTI_REG_1) = value;
      g_multi_mode = MULTI_NOP; // Temporarily switch to NOP mode
      break;
    }
    break;
  }
}

// command specifies the mode to the slave unit
u32 multi_send(u32 id, u32 command, u8 data1, u8 data2)
{
  u32 length;
  u8 work[ADHOC_BUFFER_SIZE];
  work[0] = id;
  work[1] = command;
  work[2] = data1;
  work[3] = data2;
  length = adhocSendRecvAck(work, MULTI_DATASIZE, 1000000, ADHOC_DATATYPE_ANY);
  return length;
}

u32 multi_recv(u8 *data)
{
  u32 length;
  u8 work[ADHOC_BUFFER_SIZE];
  length = adhocRecvSendAck(work, MULTI_DATASIZE, 1000000, ADHOC_DATATYPE_ANY);
  data[0] = work[0];
  data[1] = work[1];
  data[2] = work[2];
  data[3] = work[3];
  return length;
}

void adhoc_exit()
{
  adhoc_term();
  g_adhoc_link_flag = 0;
  s_net_thread_exit_flag = 1;
}
