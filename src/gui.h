/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
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
 * gui.h
 * gui processing
 ******************************************************************************/

#pragma once

#include <psptypes.h>
#include "cheats.h"
#include "common.h"
#include "input.h"

#define MAX_GAMEPAD_CONFIG_MAP 16

struct gpsp_config_t
{
  u32 screen_mode;
  u32 screen_filter;
  u32 screen_ratio;
  u32 screen_scale;
  u32 screen_interlace;
  u32 enable_audio;
  u32 enable_analog;
  u32 analog_sensitivity_level;
  u32 enable_home;
  u32 gamepad_config_map[MAX_GAMEPAD_CONFIG_MAP];
  u32 language;
  u32 emulate_core;
  u32 debug_flag;
  u32 fake_fat;
  u32 boot_mode;
#if 0
  u32 solar_level;
#endif
};

struct game_config_t
{
  u32 frameskip_type;
  u32 frameskip_value;
  u32 random_skip;
  u32 clock_speed_number;
  u32 audio_buffer_size_number;
  u32 update_backup_flag;
  cheat_t cheats_flag[MAX_CHEATS];
  u32 gamepad_config_map[MAX_GAMEPAD_CONFIG_MAP];
  u32 use_default_gamepad_map;
  u32 allocate_sensor;
};

#define ASM_CORE 0
#define C_CORE 1

#define GAME_CART 0
#define GBA_BIOS 1

/******************************************************************************
 * Declaration of global variables
 ******************************************************************************/
extern u32 g_savestate_slot_num;
extern char g_default_rom_dir[MAX_PATH];
extern char g_default_save_dir[MAX_PATH];
extern char g_default_cfg_dir[MAX_PATH];
extern char g_default_ss_dir[MAX_PATH];
extern char g_default_cheat_dir[MAX_PATH];

extern gpsp_config_t g_gpsp_config;
extern game_config_t g_game_config;

/******************************************************************************
 * Decalaration of "global" functions
 ******************************************************************************/
s32 load_file(char **wildcards, char *result, char *default_dir_name);
void load_game_config_file(void);
s32 load_config_file();
s32 save_game_config_file();
s32 save_config_file();

u32 load_dircfg(char *file_name);
u32 load_fontcfg(char *file_name);
u32 load_msgcfg(char *file_name);
u32 load_font();
void get_savestate_filename_noshot(u32 slot, char *name_buffer);
void init_gpsp_config();
void init_game_config();

//Declaration of menu functor
class menu
{
public:
  //Default constructor
  menu();

  //Make a functor
  u32 operator()(u16 *original_screen);

private:
  enum MENU_OPTION_TYPE_ENUM
  {
    NUMBER_SELECTION_TYPE = 0x01,
    STRING_SELECTION_TYPE = 0x02,
    SUBMENU_TYPE = 0x04,
    ACTION_TYPE = 0x08
  };
  struct _MENU_TYPE;
  struct _MENU_OPTION_TYPE
  {
    void (menu::*action_function)(_MENU_TYPE *, u16 *);
    void (menu::*passive_function)(_MENU_TYPE *, u16 *);
    struct _MENU_TYPE *sub_menu;
    char *display_string;
    void *options;
    u32 *current_option;
    u32 num_options;
    char *help_string;
    u32 line_number;
    MENU_OPTION_TYPE_ENUM option_type;
  };

  struct _MENU_TYPE
  {
    void (menu::*init_function)(_MENU_TYPE *, u16 *);
    void (menu::*passive_function)(_MENU_TYPE *, u16 *);
    struct _MENU_OPTION_TYPE *options;
    u32 num_options;
  };

  typedef struct _MENU_OPTION_TYPE MENU_OPTION_TYPE;
  typedef struct _MENU_TYPE MENU_TYPE;

  //Variables
  gui_action_type gui_action;
  u32 i;
  u32 repeat;
  u32 return_value;
  u32 first_load;
  char current_savestate_filename[MAX_FILE];
  char line_buffer[256];
  char cheat_format_str[MAX_CHEATS][41 * 4];

  char *gamepad_help[22];
  char *yes_no_options[2];
  char *enable_disable_options[2];
  char *scale_options[5];
  char *frameskip_options[3];
  char *frameskip_variation_options[2];
  char *audio_buffer_options[11];
  char *update_backup_options[2];
  char *clock_speed_options[10];
  char *gamepad_config_buttons[22];
  char *ratio_options[2];
  char *interlace_options[2];
  const char *language_options[12];
  char *boot_mode_options[2];
#ifdef USE_C_CORE
  char *emulate_core_options[2];
#endif

  MENU_TYPE *current_menu;
  MENU_OPTION_TYPE *current_option;
  MENU_OPTION_TYPE *display_option;
  u32 current_option_num;

  //Private functions
  void choose_menu();
  void choose_menu(MENU_TYPE *new_menu, MENU_TYPE *main_menu, u16 *original_screen);
  //All thse set of functions must have the same parameters
  void menu_exit(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_quit(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_load(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_restart(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_save_ss(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_change_state(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_save_state(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_load_state(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_load_state_file(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_load_cheat_file(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_fix_gamepad_help(MENU_TYPE *main_menu, u16 *original_screen);
  void menu_fix_analog_help(MENU_TYPE *main_menu, u16 *original_screen);
  void submenu_graphics_sound(MENU_TYPE *main_menu, u16 *original_screen);
  void submenu_cheats(MENU_TYPE *main_menu, u16 *original_screen);
  void submenu_misc(MENU_TYPE *main_menu, u16 *original_screen);
  void submenu_gamepad(MENU_TYPE *main_menu, u16 *original_screen);
  void submenu_analog(MENU_TYPE *main_menu, u16 *original_screen);
  void submenu_savestate(MENU_TYPE *main_menu, u16 *original_screen);
  void submenu_main(MENU_TYPE *main_menu, u16 *original_screen);
  void reload_cheats_page(MENU_TYPE *main_menu, u16 *original_screen);
  void home_mode(MENU_TYPE *main_menu, u16 *original_screen);
  void set_gamepad(MENU_TYPE *main_menu, u16 *original_screen);
#ifdef USE_ADHOC
  void submenu_adhoc(MENU_TYPE *main_menu, u16 *original_screen);
  void adhoc_connect_menu(MENU_TYPE *main_menu, u16 *original_screen);
  void adhoc_disconnect_menu(MENU_TYPE *main_menu, u16 *original_screen);
#endif
  //Different menus created go here. So that they can be accessed by any member function if required
  //If a menu contains compiler flags, it is declared in the () functor itself
  MENU_OPTION_TYPE graphics_sound_options[12];
  MENU_OPTION_TYPE cheats_options[13];
  MENU_OPTION_TYPE savestate_options[5];
  MENU_OPTION_TYPE gamepad_config_options[14];
  MENU_OPTION_TYPE analog_config_options[9];
};