/* General linker script intended to work on all ARMv7-A chips */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm","elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR(.)

MEMORY {
    ddr (rwx) : ORIGIN = link_location, LENGTH = 32M
}

ddr_size = ddr_size_macro;
ddr_start = ddr_start_macro;

USER_STACK  = 512;
FIQ_STACK   = 512;
IRQ_STACK   = 512;
ABORT_STACK = 512;
SVC_STACK   = 512;
UNDEF_STACK = 512;

SECTIONS {

    .kernel_entry : {
        _kernel_s = .;
        KEEP(*(.kernel_entry))
        . = ALIGN(4);
    } > ddr

    .text : {
        . = ALIGN(4);
        _text_s = .;
        *(.text)
        *(.text.*)
        
        . = ALIGN(4);
        _text_e = .;
    } > ddr

    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > ddr

    .ARM.exidx : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > ddr

    .rodata : {
        . = ALIGN(4);
        _rodata_s = .;
        KEEP(*(.rodata))
        KEEP(*(.rodata*))
        . = ALIGN(4);
        _rodata_e = .;
    } > ddr

    .data : {
        . = ALIGN(4);
        _data_s = .;
        KEEP(*(.data))
        KEEP(*(.data*))
        KEEP(*(.ramfunc))
        KEEP(*(.ramfunc*))
        . = ALIGN(4);
        _data_e = .;
    } > ddr
    
    _kernel_e = .;

    .bss (NOLOAD) : {
        . = ALIGN(4);
        _bss_s = .;
        *(.bss)
        *(.bss*)
        . = ALIGN(4);
        _bss_e = .; 
    } > ddr

    .stack (NOLOAD) : {
        . += USER_STACK;
        . = ALIGN(8);
        _user_stack_e = .;

        . += FIQ_STACK;
        . = ALIGN(8);
        _fiq_stack_e = .;

        . += IRQ_STACK;
        . = ALIGN(8);
        _irq_stack_e = .;

        . += ABORT_STACK;
        . = ALIGN(8);
        _abort_stack_e = .;

        . += SVC_STACK;
        . = ALIGN(8);
        _svc_stack_e = .;

        . += UNDEF_STACK;
        . = ALIGN(8);
        _undef_stack_e = .;
    } > ddr
}

_kernel_size = _kernel_e - _kernel_s;
_end = .;
