/* linker script for fdpp project. Derived from the one of freedos */

DOS_PSP = 0x60;
MEMOFS = DOS_PSP * 16;

/* these GROUPs play the same role as GROUPS (segments) in OMF */
PGROUP = (MEMOFS + LOADADDR(.ptext)) / 16;
LGROUP = (MEMOFS + LOADADDR(.ltext)) / 16;
DGROUP = (MEMOFS + LOADADDR(.data)) / 16;
TGROUP = (MEMOFS + LOADADDR(.text)) / 16;
IGROUP = (MEMOFS + LOADADDR(.itext)) / 16;
I_GROUP = (MEMOFS + LOADADDR(.idata)) / 16;

INITSIZE = SIZEOF(.itext) + SIZEOF(.idata) + SIZEOF(.ibss);

SECTIONS
  {
    /* Target PSP section.  */
    .ptext 0 : AT (0) {
		*(PSP)
    }

    /* Target low data+text sections.  */
    .ltext 0 : AT (ALIGN(LOADADDR(.ptext) + SIZEOF(.ptext), 16)) {
		*(_IRQTEXT)
		*(_LOWTEXT)
		*(_IO_TEXT)
		*(_IO_FIXED_DATA)
    }

    /* Target data sections.  */
    .data 0 : AT (ALIGN(LOADADDR(.ltext) + SIZEOF(.ltext), 16)) {
		*(_FIXED_DATA)
		*(_BSS)
		*(_DATA)
		*(_DATAEND)
		*(CONST)
		*(CONST2)
		*(DYN_DATA)
		ASSERT(. <= 0xfff8,
		    "Error: too large for a small-model .exe file.");
	}

    /* Target text sections.  */
    .text 0 : AT (ALIGN(LOADADDR(.data) + SIZEOF(.data), 16)) {
		*(HMA_TEXT_START)
		*(HMA_TEXT)
		*(HMA_TEXT_END)
		ASSERT(. <= 0x10000,
		    "Error: too large for a small-model .exe file.");
	}

    /* Target init text sections. */
    .itext 0 : AT (ALIGN(LOADADDR(.text) + SIZEOF(.text), 16)) {
		*(INIT_TEXT_START)
		*(INIT_TEXT)
		*(INIT_TEXT_END)
		ASSERT(. <= 0x10000,
		    "Error: too large for a small-model .exe file.");
	}

    /* Target init data sections.  */
    .idata 0 : AT (ALIGN(LOADADDR(.itext) + SIZEOF(.itext), 16)) {
		*(ID_B)
		*(ID_E)
	}
    .ibss SIZEOF(.idata) (NOLOAD) : AT (LOADADDR(.idata) + SIZEOF(.idata)) {
		*(IB_B)
		*(IB_E)
	}
    .istack 0 (NOLOAD) : AT (ALIGN(LOADADDR(.ibss) + SIZEOF(.ibss), 16)) {
		*(_STACK)
		. = 0x1000;
	}

    /*
     * Discarding .shstrtab is not supported by LLD (LLVM LD) and will trigger an
     * error. Also keep the rest of the control sections to match GNU LD behavior.
     */
    .shstrtab : {
		*(.shstrtab)
	}
    .strtab : {
		*(.strtab)
	}
    .symtab : {
		*(.symtab)
	}

    /DISCARD/ : { *(.*) }
  }
