// Entry point for the ARMv7-A kernel

.syntax unified
.cpu cortex-a5
.arm

.extern main

// Extern variables from the linker script
.extern _svc_stack_e
.extern _kernel_size

// Extern variables from the targer configuration file
.extern ddr_size
.extern ddr_start

// This is where u-boot (or any bootloader) will hand off execution
.section .kernel_entry, "ax", %progbits
kernel_entry:

    // Get the program load address
    sub r0, pc, #8
    ldr r1, =ddr_start

    // Turn off all interrupts
    cpsid afi

    // Check if a relocation is needed
    cmp r0, r1
    beq skip_kernel_relocation

relocate_kernel:
    // Check if the relocation will overwrite executing code
    ldr r2, =_kernel_size
    add r3, r2, #4
    add r4, r1, r2                     // r4 hold the upper address affected by the relocation
    cmp r4, r0
    bls .

    lsr r2, r2, #2
    add r2, r2, #1                     // Kernel size in words

    // Relocate the kernel to the start of DDR memory
1:  ldr r3, [r0], #4
    str r3, [r1], #4
    subs r2, r2, #1
    bne 1b
    b skip_invalidate_icache

    // Check if the dcache is enabled
    mrc p15, 0, r0, c1, c0, 0
    tst r0, #(1 << 2)
    beq skip_disable_dcache

    // Get the cache size
    mrc p15, 1, r5, c0, c0, 0
    lsr r3, r5, #3
    mov r6, #0x3FF
    and r3, r3, r6
    add r3, r3, #1                     // Number of ways

    mov r6, #0x7FFF
    lsr r4, r5, #13
    and r4, r4, r6
    add r4, r4, #1                     // Number of sets

    // Disable and clean the dcache
    mov r0, #0                         // Way index
1:  mov r1, #0                         // Set index
2:  mov r2, #0
    orr r2, r2, r1, LSL #5
    orr r2, r2, r0, LSL #30
    mcr p15, 0, r2, c7, c10, 2
    add r1, r1, #1
    cmp r1, r4
    bne 2b
    add r0, r0, #1
    cmp r0, r3
    bne 1b
    dsb
    isb

    // Disable the D-cache
    mrc p15, 0, r0, c1, c0, 0
    bic r0, #(1 << 2)
    mcr p15, 0, r0, c1, c0, 0
    dmb
    isb
    
skip_disable_dcache:
    // Check if the icache is enabled
    mrc p15, 0, r0, c1, c0, 0
    tst r0, #(1 << 12)
    beq skip_invalidate_icache

    // Invalidate the icache
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0
    dsb
    isb

skip_invalidate_icache:

    // Relocation is complete so we can jump to the beginning of DDR
    ldr r1, =ddr_start
    dsb
    isb
    bx r1

skip_kernel_relocation:
    // We are done relocating the kernel. We require that MMU, interrupt and D-cache is
    // disabled at this point. This will be the main entry point for the kernel

    // Setup the stack for the SVC mode
    ldr sp, =_svc_stack_e
    isb

    // Remember to init the .bss

    // Setup early kernel pagetables for upper 2 GB

    // Might have to do a physical to virtual address switch before the branch

    ldr r0, =main
    bx r0
