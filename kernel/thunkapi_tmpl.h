_THUNK_API_v(enable);
_THUNK_API_v(disable);
_THUNK_API_2(uint8_t, peekb, uint16_t, seg, uint16_t, ofs);
_THUNK_API_2(uint16_t, peekw, uint16_t, seg, uint16_t, ofs);
_THUNK_API_2(uint32_t, peekl, uint16_t, seg, uint16_t, ofs);
_THUNK_API_3v(pokeb, uint16_t, seg, uint16_t, ofs, uint8_t, b);
_THUNK_API_3v(pokew, uint16_t, seg, uint16_t, ofs, uint16_t, w);
_THUNK_API_3v(pokel, uint16_t, seg, uint16_t, ofs, uint32_t, l);
_THUNK_API_0(uint16_t, getCS);
_THUNK_API_0(uint16_t, getSS);
_THUNK_API_1(void *, short_ptr, uint16_t, offs);
_THUNK_API_2(uint16_t, mk_offs, uint8_t *, data, uint16_t, len);
_THUNK_API_1(uint32_t, getlong, void *, addr);
_THUNK_API_1(uint16_t, getword, void *, addr);
_THUNK_API_1(uint8_t, getbyte, void *, addr);
_THUNK_API_1(uint32_t, fgetlong, void *, faddr);
_THUNK_API_1(uint16_t, fgetword, void *, faddr);
_THUNK_API_1(uint8_t, fgetbyte, void *, faddr);
_THUNK_API_2v(fputlong, void *, faddr, uint32_t, l);
_THUNK_API_2v(fputword, void *, faddr, uint16_t, w);
_THUNK_API_2v(fputbyte, void *, faddr, uint8_t, b);