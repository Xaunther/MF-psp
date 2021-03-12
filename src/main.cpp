#include <pspkernel.h>
#include <pspdebug.h>
#include "main.h"

PSP_MODULE_INFO("gpSP", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_PRIORITY(0x11);
PSP_MAIN_THREAD_STACK_SIZE_KB(640);
PSP_HEAP_SIZE_MAX();

/******************************************************************************
 * Global variables
 ******************************************************************************/
TIMER_TYPE timer[4];

//u32 global_cycles_per_instruction = 1;
//u64 frame_count_initial_timestamp = 0;
//u64 last_frame_interval_timestamp;
u32 psp_fps_debug = 0;
u32 skip_next_frame_flag = 0;
//u32 frameskip_counter = 0;

u32 cpu_ticks = 0;
u32 frame_ticks = 0;

u32 g_execute_cycles = 960;
s32 video_count = 960;
//u32 ticks;

//u32 arm_frame = 0;
//u32 thumb_frame = 0;
u32 last_frame = 0;

u32 g_synchronize_flag = 1;

char main_path[MAX_PATH];
char rom_path[MAX_PATH];

//vu32 quit_flag;
vu32 power_flag;

vu32 real_frame_count = 0;
u32 virtual_frame_count = 0;
vu32 vblank_count = 0;
u32 num_skipped_frames = 0;
u32 frames;

//List of languages
const char *lang[12] =
    {
        "japanese",            // 0
        "english",             // 1
        "french",              // 2
        "spanish",             // 3
        "german",              // 4
        "italian",             // 5
        "dutch",               // 6
        "portuguese",          // 7
        "russian",             // 8
        "korean",              // 9
        "chinese_traditional", // 10
        "chinese_simplified"   // 11
};

#define MAX_LANG_NUM 11

int date_format;

u32 prescale_table[] = {0, 6, 8, 10};

const char *file_ext[] = {".gba", ".bin", ".zip", NULL};

/******************************************************************************
 * Even more macro definitions. Maybe move to .h?
 ******************************************************************************/

//Determine emulation cycle
#define CHECK_COUNT(count_var)          \
    if ((count_var) < g_execute_cycles) \
        g_execute_cycles = count_var;

#define CHECK_TIMER(timer_number)                     \
    if (timer[timer_number].status == TIMER_PRESCALE) \
        CHECK_COUNT(timer[timer_number].count);

// PSP model
MODEL_TYPE psp_model;
u32 g_use_home;

int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

int CallbackThread(SceSize args, void *argp)
{
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();

    return 0;
}

int SetupCallbacks(void)
{
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
    {
        sceKernelStartThread(thid, 0, 0);
    }
    return thid;
}

int main()
{ //In C++ `auto main() -> int` is also valid.
    SetupCallbacks();
    pspDebugScreenInit();
    pspDebugScreenPrintf("Hello World!");
    return 0;
}