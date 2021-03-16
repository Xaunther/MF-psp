/* code from unofficial gamesplaySP kai
 * This is just me trying to understand how to do things for psp
 * No intention to take credit from original gpSP author nor takka
*/

/******************************************************************************
 * cpu.h
 ******************************************************************************/

#pragma once

#include <psptypes.h>
// System mode and user mode are represented as the same here

//Enumerations
enum CPU_MODE_TYPE
{
    MODE_USR,
    MODE_IRQ,
    MODE_FIQ,
    MODE_SVC,
    MODE_ABT,
    MODE_UND,
    MODE_INV
};

enum CPU_ALERT_TYPE
{
    CPU_ALERT_NONE,
    CPU_ALERT_HALT,
    CPU_ALERT_SMC,
    CPU_ALERT_IRQ
};

enum CPU_HALT_TYPE
{
    CPU_ACTIVE,
    CPU_HALT,
    CPU_STOP
};

enum IRQ_TYPE
{
    IRQ_NONE = 0x0000,
    IRQ_VBLANK = 0x0001,
    IRQ_HBLANK = 0x0002,
    IRQ_VCOUNT = 0x0004,
    IRQ_TIMER0 = 0x0008,
    IRQ_TIMER1 = 0x0010,
    IRQ_TIMER2 = 0x0020,
    IRQ_TIMER3 = 0x0040,
    IRQ_SERIAL = 0x0080,
    IRQ_DMA0 = 0x0100,
    IRQ_DMA1 = 0x0200,
    IRQ_DMA2 = 0x0400,
    IRQ_DMA3 = 0x0800,
    IRQ_KEYPAD = 0x1000,
    IRQ_GAMEPAK = 0x2000,
};

enum EXT_REG_NUMBERS
{
    REG_SP = 13,
    REG_LR = 14,
    REG_PC = 15,
    REG_N_FLAG = 16,
    REG_Z_FLAG = 17,
    REG_C_FLAG = 18,
    REG_V_FLAG = 19,
    REG_CPSR = 20,
    REG_SAVE = 21,
    REG_SAVE2 = 22,
    REG_SAVE3 = 23,
    CPU_MODE = 29,
    CPU_HALT_STATE = 30,
    CHANGED_PC_STATUS = 31
};

enum TRANSLATION_REGION_TYPE
{
    TRANSLATION_REGION_RAM,
    TRANSLATION_REGION_ROM,
    TRANSLATION_REGION_BIOS
};

//Function definitions
void execute_arm(u32 cycles);

void init_translater();
void cpu_read_mem_savestate(u32 ver);
void cpu_write_mem_savestate(u32 ver);
void cpu_get_size_savestate(u32 ver);

//Assembly functions
#ifdef __cplusplus
extern "C"
{
#endif
    void execute_arm_translate(u32 cycles);
    u32 execute_load_u8(u32 address);
    u32 execute_load_u16(u32 address);
    u32 execute_load_u32(u32 address);
    u32 execute_load_s8(u32 address);
    u32 execute_load_s16(u32 address);
    void execute_store_u8(u32 address, u32 source);
    void execute_store_u16(u32 address, u32 source);
    void execute_store_u32(u32 address, u32 source);

    void invalidate_icache_region(u8 *addr, u32 length);
    void invalidate_all_cache();

    u8 *block_lookup_address_arm(u32 pc);
    u8 *block_lookup_address_thumb(u32 pc);
    u8 *block_lookup_address_dual(u32 pc);

    void flush_translation_cache_ram();
#ifdef __cplusplus
}
#endif

//Common operations (macros) for ROM, RAM and BIOS
#define ROM_TRANSLATION_CACHE_SIZE (1024 * 512 * 3)  /* 2048 KB 0x20 0000 */
#define RAM_TRANSLATION_CACHE_SIZE (1024 * 128 * 1)  /*  128 KB 0x06 0000 There is no situation exceeding 0x020000 so far*/
#define BIOS_TRANSLATION_CACHE_SIZE (1024 * 128 * 1) /*   32 KB 0x00 8000 There is no situation exceeding 0x008000 so far*/
#define TRANSLATION_CACHE_LIMIT_THRESHOLD (1024)
//Global variables for rom, ram, bios
extern u8 rom_translation_cache[ROM_TRANSLATION_CACHE_SIZE];
extern u8 ram_translation_cache[RAM_TRANSLATION_CACHE_SIZE];
extern u8 bios_translation_cache[BIOS_TRANSLATION_CACHE_SIZE];
extern u8 *rom_translation_ptr;
extern u8 *ram_translation_ptr;
extern u8 *bios_translation_ptr;

//Translation global variables and macros
#define MAX_TRANSLATION_GATES 8
#define MAX_IDLE_LOOPS 8

extern u32 idle_loop_targets;
extern u32 idle_loop_target_pc[MAX_IDLE_LOOPS];
extern u32 force_pc_update_target;
extern u32 iwram_stack_optimize;
//extern u32 allow_smc_ram_u8;
//extern u32 allow_smc_ram_u16;
//extern u32 allow_smc_ram_u32;
extern u32 direct_map_vram;
extern u32 translation_gate_targets;
extern u32 translation_gate_target_pc[MAX_TRANSLATION_GATES];

extern u32 in_interrupt;

extern u32 bios_mode;

#define ROM_BRANCH_HASH_SIZE (1024 * 64)

extern u32 *rom_branch_hash[ROM_BRANCH_HASH_SIZE];

void flush_translation_cache_rom();
void flush_translation_cache_bios();
void dump_translation_cache();

extern u32 reg_mode[7][7];
extern u32 spsr[6];

extern u32 cpu_modes[32];

void init_cpu();
