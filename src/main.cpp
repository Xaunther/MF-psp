#include <unistd.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <psppower.h>
#include <pspdisplay.h>
#include "main.h"
#include "gu.h"

PSP_MODULE_INFO("gpSP", PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_PRIORITY(0x11);
PSP_MAIN_THREAD_STACK_SIZE_KB(640);
PSP_HEAP_SIZE_MAX();

/******************************************************************************
 * Global variables. Keep same order as in main.h
 ******************************************************************************/
u32 g_execute_cycles = 960;
u32 g_synchronize_flag = 1;
u32 skip_next_frame_flag = 0;
TIMER_TYPE timer[4];
u32 prescale_table[] = {0, 6, 8, 10};
char main_path[MAX_PATH];
char rom_path[MAX_PATH];
vu32 real_frame_count = 0;
u32 virtual_frame_count = 0;
int date_format;
MODEL_TYPE psp_model;
const char *lang[] =
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
u32 g_use_home;

#define MAX_LANG_NUM 11

// Variables for this file (not extern)
//u32 global_cycles_per_instruction = 1;
//u64 frame_count_initial_timestamp = 0;
//u64 last_frame_interval_timestamp;
u32 psp_fps_debug = 0;
//u32 frameskip_counter = 0;
u32 cpu_ticks = 0;
u32 frame_ticks = 0;
s32 video_count = 960;
//u32 ticks;
//u32 arm_frame = 0;
//u32 thumb_frame = 0;
u32 last_frame = 0;
//vu32 quit_flag;
vu32 power_flag;
vu32 vblank_count = 0;
u32 num_skipped_frames = 0;
u32 frames;
const char *file_ext[] = {".gba", ".bin", ".zip", NULL};

//Start implementing functions
int main(int argc, char *argv[])
{
    char load_filename[MAX_FILE];
    char filename[MAX_FILE];

    sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
    pspDebugScreenInit();
    // Copy the directory path of the executable into main_path
    getcwd(main_path, MAX_FILE);
    chdir(main_path);
#ifdef USE_DEBUG
    // デバッグ出力ファイルのオープン
    g_dbg_file = fopen(DBG_FILE_NAME, "awb");
    DBGOUT("\nStart gpSP\n");
#endif
    pspDebugScreenPrintf("Holiwis uWu");
    return 0;
}

void set_cpu_clock(u32 num)
{
    const u32 clock_speed_table[11] = {33, 66, 100, 133, 166, 200, 233, 266, 300, 333, 222};
    num = num % 11;
    scePowerSetClockFrequency(clock_speed_table[num], clock_speed_table[num], clock_speed_table[num] / 2);
}
//
//
//
//
//
//
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