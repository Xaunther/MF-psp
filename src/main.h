/* code from unofficial gamesplaySP kai
 * This is just me trying to understand how to do things for psp
 * No intention to take credit from original gpSP author nor takka
*/

/******************************************************************************
 * main.h header
 ******************************************************************************/
#ifndef MAIN_H
#define MAIN_H
#include <psptypes.h>
#include "common.h"
#include "cpu.h"
/******************************************************************************
 * Definition of structures, functions...
 ******************************************************************************/
enum TIMER_STATUS_TYPE
{
    TIMER_INACTIVE,
    TIMER_PRESCALE,
    TIMER_CASCADE
};

enum TIMER_IRQ_TYPE
{
    TIMER_NO_IRQ,
    TIMER_TRIGGER_IRQ
};

enum TIMER_DS_CHANNEL_TYPE
{
    TIMER_DS_CHANNEL_NONE,
    TIMER_DS_CHANNEL_A,
    TIMER_DS_CHANNEL_B,
    TIMER_DS_CHANNEL_BOTH,
};

struct TIMER_TYPE
{
    s32 count;
    u32 reload;
    u32 prescale;
    u32 stop_cpu_ticks; /* NOT USE */
    u32 frequency_step;
    TIMER_DS_CHANNEL_TYPE direct_sound_channels;
    TIMER_IRQ_TYPE irq;
    TIMER_STATUS_TYPE status;
};

enum FRAMESKIP_TYPE
{
    auto_frameskip,
    manual_frameskip,
    no_frameskip
};

//PSP model
enum MODEL_TYPE
{
    PSP_1000, /* PSP-1000 all CFW or PSP-2000*/
    PSP_2000  /* PSP-2000 CFW 3.71 M33-3 or higher */
};

// TODO (Translated from Japanese with translate) Reexamine the processing around the timer counter
// Counter setting when reloading the timer (not set to the timer at this point)
// Set to counter when timer starts
// However, when accessing 32bit, it will be set to the timer immediately.
// 0 ~ 0xFFFF in the actual machine, but takes the value of (0x10000 ~ 1) << prescale (0,6,8,10) inside gpSP
// Shifted by prescale when reloading to each counter
// TODO: It is necessary to separate processing for 32bit access and 8 / 16bit access
// 8/16 No need to call for bit access?

#define COUNT_TIMER(timer_number)                      \
    timer[timer_number].reload = 0x10000 - value;      \
    if (timer_number < 2)                              \
    {                                                  \
        u32 timer_reload = timer[timer_number].reload; \
        SOUND_UPDATE_FREQUENCY_STEP(timer_number);     \
    }

// Adjust timer value
// TODO: Adjustment required
#define ADJUST_SOUND_BUFFER(timer_number, channel)                           \
    if (timer[timer_number].direct_sound_channels & (0x01 << channel))       \
    {                                                                        \
        direct_sound_channel[channel].buffer_index = gbc_sound_buffer_index; \
    }

//Timer access and count start processing

#define TRIGGER_TIMER(timer_number)                                                                       \
    if (value & 0x80)                                                                                     \
    {                                                                                                     \
        /*When the start bit is 1*/                                                                       \
        if (timer[timer_number].status == TIMER_INACTIVE)                                                 \
        {                                                                                                 \
            /* If the timer is stopped */                                                                 \
            /* Make various settings and activate the timer */                                            \
            /* Read reload value */                                                                       \
            u32 timer_reload = timer[timer_number].reload;                                                \
            /* Determine if it is in cascade mode (other than timer 0) */                                 \
            if (((value >> 2) & 0x01) && (timer_number != 0))                                             \
            {                                                                                             \
                /* Cascade mode */                                                                        \
                timer[timer_number].status = TIMER_CASCADE;                                               \
                /* Prescale settings */                                                                   \
                timer[timer_number].prescale = 0;                                                         \
            }                                                                                             \
            else                                                                                          \
            {                                                                                             \
                /* Prescale mode */                                                                       \
                timer[timer_number].status = TIMER_PRESCALE;                                              \
                u32 prescale = prescale_table[value & 0x03];                                              \
                /* Prescale settings */                                                                   \
                timer[timer_number].prescale = prescale;                                                  \
            }                                                                                             \
                                                                                                          \
            /* IRQ settings */                                                                            \
            timer[timer_number].irq = (value >> 6) & 0x01;                                                \
                                                                                                          \
            /* Set counter */                                                                             \
            timer[timer_number].count = timer_reload << timer[timer_number].prescale;                     \
            ADDRESS16(io_registers, 0x100 + (timer_number * 4)) = 0x10000 - timer_reload;                 \
                                                                                                          \
            if (timer[timer_number].count < g_execute_cycles)                                             \
                g_execute_cycles = timer[timer_number].count;                                             \
                                                                                                          \
            if (timer_number < 2)                                                                         \
            {                                                                                             \
                /* Since the decimal point was truncated, the processing was the same as for GBC sound */ \
                SOUND_UPDATE_FREQUENCY_STEP(timer_number);                                                \
                ADJUST_SOUND_BUFFER(timer_number, 0);                                                     \
                ADJUST_SOUND_BUFFER(timer_numberm1);                                                      \
            }                                                                                             \
        }                                                                                                 \
    }                                                                                                     \
    else                                                                                                  \
    {                                                                                                     \
        if (timer[timer_number].status != TIMER_INACTIVE)                                                 \
        {                                                                                                 \
            timer[timer_number].status = TIMER_INACTIVE;                                                  \
        }                                                                                                 \
    }                                                                                                     \
    ADDRESS16(io_registers, 0x102 + (timer_number * 4)) = value;

/******************************************************************************
 * Global Variables
 ******************************************************************************/
extern u32 g_execute_cycles;
//extern u32 global_cycles_per_instruction;
extern u32 g_synchronize_flag;
extern u32 skip_next_frame_flag;
extern TIMER_TYPE timer[4];
extern u32 prescale_table[];
extern char main_path[MAX_PATH];
extern char rom_path[MAX_PATH];
extern volatile u32 real_frame_count;
extern u32 virtual_frame_count;
extern int date_format;
extern MODEL_TYPE psp_model;
extern char *lang[12];
extern u32 g_use_home;

/******************************************************************************
 * "Global" functions
 ******************************************************************************/
void set_cpu_clock(u32 clock);
u32 update_gba();
void reset_gba();
void synchronize();
void quit(u32 mode);
void game_name_ext(u8 *src, u8 *buffer, u8 *extension);
void main_read_mem_savestate(u32 ver);
void main_write_mem_savestate(u32 ver);
void main_get_size_savestate(u32 ver);

void error_msg(char *text);
void set_cpu_mode(CPU_MODE_TYPE new_mode);
void raise_interrupt(IRQ_TYPE irq_raised);
void change_ext(char *src, char *buffer, char *extension);
u32 file_length(const char *filename);
MODEL_TYPE get_model();

#endif /* MAIN_H */