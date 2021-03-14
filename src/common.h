/* code from unofficial gamesplaySP kai
 * This is just me trying to understand how to do things for psp
 * No intention to take credit from original gpSP author nor takka
*/

/******************************************************************************
 * common.h
 * Definitions commonly used all around
 * Used to include EVERYTHING here, but I believe we can do better than that
 ******************************************************************************/

#pragma once

#define OLD_COUNT

//Includes
#include <psptypes.h>
#include <pspkerneltypes.h>

/******************************************************************************
 * Macro definitions, many aliases too
 ******************************************************************************/
typedef SceUID FILE_TAG_TYPE;
typedef u32 FIXED16_16; // 32-bit integer: 16bit real 16-bit fixed point
typedef u32 FIXED8_24;  // 32-bit integer: 8b-it real 24-bit fixed point

#define MAX_PATH 512
#define MAX_FILE 256
#define FILE_ID SceUID

#define SYS_CLOCK (16777216.0)

#define ROR(dest, value, shift) \
    dest = ((u32)((value) >> (shift))) | ((u32)((value) << (32 - (shift))))

#define PSP_FILE_OPEN_APPEND (PSP_O_CREAT | PSP_O_APPEND | PSP_O_TRUNC)

#define PSP_FILE_OPEN_READ PSP_O_RDONLY

#define PSP_FILE_OPEN_WRITE (PSP_O_CREAT | PSP_O_WRONLY | PSP_O_TRUNC)

#define FILE_OPEN(filename_tag, filename, mode) \
    filename_tag = sceIoOpen(filename, PSP_FILE_OPEN_##mode, 0777)

#define FILE_CHECK_VALID(filename_tag) \
    (filename_tag >= 0)

#define FILE_CLOSE(filename_tag) \
    sceIoClose(filename_tag)

#define FILE_DELETE(filename) \
    sceIoRemove(filename)

#define FILE_READ(filename_tag, buffer, size) \
    sceIoRead(filename_tag, buffer, size)

#define FILE_WRITE(filename_tag, buffer, size) \
    sceIoWrite(filename_tag, buffer, size)

#define FILE_READ_MEM(ptr, buffer, size) \
    {                                    \
        memcpy(buffer, ptr, size);       \
        ptr += size;                     \
    }

#define FILE_WRITE_MEM(ptr, buffer, size) \
    {                                     \
        memcpy(ptr, buffer, size);        \
        ptr += size;                      \
    }

#define FILE_GET_SIZE(ptr, buffer, size) \
    ptr += size

#define FILE_SEEK(filename_tag, offset, type) \
    sceIoLseek(filename_tag, offset, type)

// These must be variables, not constants.

#define FILE_READ_VARIABLE(filename_tag, variable) \
    FILE_READ(filename_tag, &variable, sizeof(variable))

#define FILE_WRITE_VARIABLE(filename_tag, variable) \
    FILE_WRITE(filename_tag, &variable, sizeof(variable))

#define FILE_READ_MEM_VARIABLE(ptr, variable) \
    FILE_READ_MEM(ptr, &variable, sizeof(variable))

#define FILE_WRITE_MEM_VARIABLE(ptr, variable) \
    FILE_WRITE_MEM(ptr, &variable, sizeof(variable))

#define FILE_GET_SIZE_VARIABLE(ptr, variable) \
    ptr += sizeof(variable)

// These must be statically declared arrays (ie, global or on the stack,
// not dynamically allocated on the heap)

#define FILE_READ_ARRAY(filename_tag, array) \
    FILE_READ(filename_tag, array, sizeof(array))

#define FILE_WRITE_ARRAY(filename_tag, array) \
    FILE_WRITE(filename_tag, array, sizeof(array))

#define FILE_READ_MEM_ARRAY(ptr, array) \
    FILE_READ_MEM(ptr, array, sizeof(array))

#define FILE_WRITE_MEM_ARRAY(ptr, array) \
    FILE_WRITE_MEM(ptr, array, sizeof(array))

#define FILE_GET_SIZE_ARRAY(ptr, array) \
    ptr += sizeof(array)

#define FLOAT_TO_FP16_16(value) \
    (FIXED16_16)((value)*65536.0)

#define FP16_16_TO_FLOAT(value) \
    (float)((value) / 65536.0)

#define U32_TO_FP16_16(value) \
    ((value) << 16)

#define FP16_16_TO_U32(value) \
    ((value) >> 16)

#define FP16_16_FRACTIONAL_PART(value) \
    ((value)&0xFFFF)

#define FIXED_DIV(numerator, denominator, bits) \
    ((((numerator) * (1 << (bits))) + ((denominator) / 2)) / (denominator))

#define ADDRESS8(base, offset) \
    *((u8 *)((u8 *)(base) + (offset)))

#define ADDRESS16(base, offset) \
    *((u16 *)((u8 *)(base) + (offset)))

#define ADDRESS32(base, offset) \
    *((u32 *)((u8 *)(base) + (offset)))

#define printf pspDebugScreenPrintf

#define USE_BIOS 0
#define EMU_BIOS 1

#define NO 0
#define YES 1

#ifdef USE_DEBUG
// デバッグ用の設定
#define DBG_FILE_NAME "dbg_msg.txt"
#define DBGOUT(...)                     \
    if (g_gpsp_config.debug_flag != NO) \
    fprintf(g_dbg_file, __VA_ARGS__)
#define DBGOUT_1(...)                  \
    if (g_gpsp_config.debug_flag >= 1) \
    fprintf(g_dbg_file, __VA_ARGS__)
#define DBGOUT_2(...)                  \
    if (g_gpsp_config.debug_flag >= 2) \
    fprintf(g_dbg_file, __VA_ARGS__)
#define DBGOUT_3(...)                  \
    if (g_gpsp_config.debug_flag >= 3) \
    fprintf(g_dbg_file, __VA_ARGS__)
#define DBGOUT_4(...)                  \
    if (g_gpsp_config.debug_flag >= 4) \
    fprintf(g_dbg_file, __VA_ARGS__)
#define DBGOUT_5(...)                  \
    if (g_gpsp_config.debug_flag >= 5) \
    fprintf(g_dbg_file, __VA_ARGS__)
FILE *g_dbg_file;

u64 dbg_time_1;
u64 dbg_time_2;
#define GET_TIME_1() sceRtcGetCurrentTick(&dbg_time_1);
#define GET_TIME_2() sceRtcGetCurrentTick(&dbg_time_2);
#define WRITE_TIME(msg) DBGOUT("%s: %d μs\n", msg, (int)(dbg_time_2 - dbg_time_1));
#else
#define DBGOUT(...)
#endif
