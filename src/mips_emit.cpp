//Put in here the definitions of the functions defined in mips_emit.h
//Avoids multiple declaration...

#include "mips_emit.h"

//Global variables
u8 swi_hle_handle[256][2] =
    {
        /* use bios , emu bios */
        {0x0, 0x0}, // SWI 0:  SoftReset
        {0x0, 0x0}, // SWI 1:  RegisterRAMReset
        {0x0, 0x0}, // SWI 2:  Halt
        {0x0, 0x0}, // SWI 3:  Stop/Sleep
        {0x0, 0x0}, // SWI 4:  IntrWait
        {0x0, 0x0}, // SWI 5:  VBlankIntrWait
        {0x1, 0x1}, // SWI 6:  Div
        {0x1, 0x1}, // SWI 7:  DivArm
        {0x0, 0x1}, // SWI 8:  Sqrt
        {0x0, 0x0}, // SWI 9:  ArcTan
        {0x0, 0x0}, // SWI A:  ArcTan2
        {0x0, 0x1}, // SWI B:  CpuSet
        {0x0, 0x1}, // SWI C:  CpuFastSet
        {0x0, 0x0}, // SWI D:  GetBIOSCheckSum
        {0x0, 0x1}, // SWI E:  BgAffineSet
        {0x0, 0x1}, // SWI F:  ObjAffineSet
        {0x0, 0x0}, // SWI 10: BitUnpack
        {0x0, 0x0}, // SWI 11: LZ77UnCompWram
        {0x0, 0x0}, // SWI 12: LZ77UnCompVram
        {0x0, 0x0}, // SWI 13: HuffUnComp
        {0x0, 0x0}, // SWI 14: RLUnCompWram
        {0x0, 0x0}, // SWI 15: RLUnCompVram
        {0x0, 0x0}, // SWI 16: Diff8bitUnFilterWram
        {0x0, 0x0}, // SWI 17: Diff8bitUnFilterVram
        {0x0, 0x0}, // SWI 18: Diff16bitUnFilter
        {0x0, 0x0}, // SWI 19: SoundBias
        {0x0, 0x0}, // SWI 1A: SoundDriverInit
        {0x0, 0x0}, // SWI 1B: SoundDriverMode
        {0x0, 0x0}, // SWI 1C: SoundDriverMain
        {0x0, 0x0}, // SWI 1D: SoundDriverVSync
        {0x0, 0x0}, // SWI 1E: SoundChannelClear
        {0x0, 0x0}, // SWI 20: SoundWhatever0
        {0x0, 0x0}, // SWI 21: SoundWhatever1
        {0x0, 0x0}, // SWI 22: SoundWhatever2
        {0x0, 0x0}, // SWI 23: SoundWhatever3
        {0x0, 0x0}, // SWI 24: SoundWhatever4
        {0x0, 0x0}, // SWI 25: MultiBoot
        {0x0, 0x0}, // SWI 26: HardReset
        {0x0, 0x0}, // SWI 27: CustomHalt
        {0x0, 0x0}, // SWI 28: SoundDriverVSyncOff
        {0x0, 0x0}, // SWI 29: SoundDriverVSyncOn
        {0x0, 0x0}  // SWI 2A: SoundGetJumpList
};

u32 arm_to_mips_reg[] =
    {
        reg_r0,
        reg_r1,
        reg_r2,
        reg_r3,
        reg_r4,
        reg_r5,
        reg_r6,
        reg_r7,
        reg_r8,
        reg_r9,
        reg_r10,
        reg_r11,
        reg_r12,
        reg_r13,
        reg_r14,
        reg_a0,
        reg_a1,
        reg_a2,
        reg_temp};

// It should be okay to still generate result flags, spsr will overwrite them.
// This is pretty infrequent (returning from interrupt handlers, et al) so
// probably not worth optimizing for.
u32 execute_spsr_restore_body(u32 address)
{
    set_cpu_mode((CPU_MODE_TYPE)cpu_modes[reg[REG_CPSR] & 0x1F]);
    if ((io_registers[REG_IE] & io_registers[REG_IF]) &&
        (io_registers[REG_IME] & 0x01) && ((reg[REG_CPSR] & 0x80) == 0))
    {
        reg_mode[MODE_IRQ][6] = address + 4;
        spsr[MODE_IRQ] = reg[REG_CPSR];
        reg[REG_CPSR] = /*(reg[REG_CPSR] & ~0xFF) |*/ 0xD2;
        address = 0x00000018;
        set_cpu_mode(MODE_IRQ);
    }

    if (reg[REG_CPSR] & 0x20)
        address |= 0x01;

    return address;
}

u32 execute_store_cpsr_body(u32 _cpsr, u32 store_mask, u32 address)
{
    reg[REG_CPSR] = _cpsr;
    if (store_mask & 0xFF)
    {
        set_cpu_mode((CPU_MODE_TYPE)cpu_modes[_cpsr & 0x1F]);
        if ((io_registers[REG_IE] & io_registers[REG_IF]) &&
            (io_registers[REG_IME] & 0x01) && ((_cpsr & 0x80) == 0))
        {
            reg_mode[MODE_IRQ][6] = address + 4;
            spsr[MODE_IRQ] = _cpsr;
            reg[REG_CPSR] = /*(reg[REG_CPSR] & ~0xFF) |*/ 0xD2;
            set_cpu_mode(MODE_IRQ);
            return 0x00000018;
        }
    }

    return 0;
}