/* linker script for fdpp project. Derived from the one of freedos */

DOS_PSP = 0x60;

/* these GROUPs play the same role as GROUPS (segments) in OMF */
/* TINY model, all segments are equal */
PGROUP = DOS_PSP;
LGROUP = DOS_PSP;
DGROUP = DOS_PSP;
TGROUP = DOS_PSP;
IGROUP = DOS_PSP;
I_GROUP = DOS_PSP;

SECTIONS
  {
    /* Target PSP section.  */
    .ptext : {
		*(PSP)
    }

    /* Target low data+text sections.  */
    .ltext : {
		*(_IRQTEXT)
		*(_LOWTEXT)
		*(_IO_TEXT)
		*(_IO_FIXED_DATA)
    }

    /* Target data sections.  */
    .data : {
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
    .text : {
		*(HMA_TEXT_START)
		*(HMA_TEXT)
		*(HMA_TEXT_END)
		ASSERT(. <= 0x10000,
		    "Error: too large for a small-model .exe file.");
	}

    /* Target init text sections. */
    .itext : {
		*(INIT_TEXT_START)
		*(INIT_TEXT)
		*(INIT_TEXT_END)
		ASSERT(. <= 0x10000,
		    "Error: too large for a small-model .exe file.");
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
    .rel.plt : {
		*(.rel.plt)
	}
    .rela.plt : {
		*(.rela.plt)
	}

    /DISCARD/ : { *(.*) }
  }
