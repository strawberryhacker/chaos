// Main entry point for the ARMv7-A kernel

.syntax unified
.cpu cortex-a5
.arm

.extern main
.extern _svc_stack_e

.section .kernel_entry, "ax", %progbits
kernel_entry:

    // Setup the stack for the kernel SVC entry
    ldr sp, =_svc_stack_e
    isb

    b main
