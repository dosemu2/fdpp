_THUNK_API_v(enable) SEMIC
_THUNK_API_v(disable) SEMIC
_THUNK_API_2(UBYTE, peekb, UWORD, seg, UWORD, ofs) SEMIC
_THUNK_API_2(UWORD, peekw, UWORD, seg, UWORD, ofs) SEMIC
_THUNK_API_2(ULONG, peekl, UWORD, seg, UWORD, ofs) SEMIC
_THUNK_API_3v(pokeb, UWORD, seg, UWORD, ofs, UBYTE, b) SEMIC
_THUNK_API_3v(pokew, UWORD, seg, UWORD, ofs, UWORD, w) SEMIC
_THUNK_API_3v(pokel, UWORD, seg, UWORD, ofs, ULONG, l) SEMIC
_THUNK_API_0(UWORD, getCS) SEMIC
_THUNK_API_0(UWORD, getSS) SEMIC
_THUNK_API_1(void *, short_ptr, UWORD, offs) SEMIC
_THUNK_API_2(UWORD, mk_offs, UBYTE *, data, UWORD, len) SEMIC
