ENTRY(_start)
SECTIONS
{
    . = 0x0;

    .text : {
        KEEP(*(.text.boot))
        *(.text .text.* .gnu.linkonce.t*)
    }

    .rodata : {
        KEEP(*(.rodata.defaults))
        *(.rodata .rodata.* .gnu.linkonce.r*)
    }

    PROVIDE(_data = .);
    .data : {
        *(.data .data.* .gnu.linkonce.d*)
    }

    .bss(NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
