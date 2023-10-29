/* linker script for fdpp project */

SECTIONS
  {
    _LOADADDR = DEFINED(loadaddr) ? loadaddr : .;
    /* these GROUPs play the same role as GROUPS (segments) in OMF */
    /* TINY model, all segments are equal */
    _LGROUP = ABSOLUTE(.);
    _DGROUP = ABSOLUTE(.);
    _IGROUP = ABSOLUTE(.);
    _LGROUP~ = . + _LOADADDR;
    _DGROUP~ = . + _LOADADDR;
    _IGROUP~ = . + _LOADADDR;

    . = SIZEOF_HEADERS;
    . = ALIGN(0x100);
    /* Target PSP section.  */
    .ptext : {
		_PGROUP = ABSOLUTE(.);
		_PGROUP~ = . + _LOADADDR;
		*(PSP)
		. = ALIGN(0x100);
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
		__bss_start = .;
		*(_BSS)
		__bss_end = .;
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

    /* Target init data sections.  */
    .ibss (NOLOAD) : {
		__ibss_start = .;
		*(IB)
		__ibss_end = .;
		*(IB_E)
	}

    /* We do not DISCARD any sections. But we still need to list them
     * explicitly to prevent linker from trying to put some _before_ our
     * sections. */
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
    /* for build-id */
    .note.gnu.build-id  : {
		*(.note.gnu.build-id)
	}
    /* for solib */
    .dynsym         : {
		*(.dynsym)
	}
    .dynstr         : {
		*(.dynstr)
	}
    .dynamic        : {
		*(.dynamic)
	}
    .hash           : {
		*(.hash)
	}
    .gnu.hash       : {
		*(.gnu.hash)
	}
    .interp : { *(.interp) }
  }
