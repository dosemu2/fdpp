/****************************************************************/
/*                                                              */
/*                          chario.c                            */
/*                           DOS-C                              */
/*                                                              */
/*    Character device functions and device driver interface    */
/*                                                              */
/*                      Copyright (c) 1994                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/*                                                              */
/****************************************************************/

#include "portab.h"

#ifdef VERSION_STRINGS
static BYTE *charioRcsId =
    "$Id: chario.c 1413 2009-06-01 13:41:03Z bartoldeman $";
#endif

#include "globals.h"
#include "init-mod.h"

STATIC int CharRequest(struct dhdr FAR *dev, unsigned command)
{
  ____R(CharReqHdr.r_command) = command;
  ____R(CharReqHdr.r_unit) = 0;
  ____R(CharReqHdr.r_status) = 0;
  ____R(CharReqHdr.r_length) = sizeof(request);
  execrh(&CharReqHdr, dev);
  if (CharReqHdr.r_status & S_ERROR)
  {
    for (;;) {
      switch (char_error(&CharReqHdr, dev))
      {
      case ABORT:
      case FAIL:
        return DE_INVLDACC;
      case CONTINUE:
        ____R(CharReqHdr.r_count) = 0;
        return 0;
      case RETRY:
        return 1;
      }
    }
  }
  return SUCCESS;
}

long BinaryCharIO(struct dhdr FAR *pdev, size_t n, void FAR * bp,
                  unsigned command)
{
  int err;
  do
  {
    ____R(CharReqHdr.r_count) = n;
    CharReqHdr.r_trans = bp;
    err = CharRequest(pdev, command);
  } while (err == 1);
  return err == SUCCESS ? (long)CharReqHdr.r_count : err;
}

STATIC int CharIO(struct dhdr FAR *pdev, unsigned char ch, unsigned command)
{
  int err = (WORD)BinaryCharIO(pdev, 1, MK_FAR_SCP(ch), command);
  if (err == 0)
    return 256;
  if (err < 0)
    return err;
  return ch;
}

STATIC int CharIOfd(UCOUNT fd, unsigned char ch, unsigned command)
{
  int sft_idx = get_sft_idx(fd);
  sft FAR *s = idx_to_sft(sft_idx);

  if (!s || !(s->sft_flags & SFT_FDEVICE) || !s->sft_dev)
    return -1;
  return CharIO(s->sft_dev, ch, command);
}

/* STATE FUNCTIONS */

STATIC void CharCmd(struct dhdr FAR *pdev, unsigned command)
{
  while (CharRequest(pdev, command) == 1);
}

STATIC int Busy(struct dhdr FAR *pdev)
{
  CharCmd(pdev, C_ISTAT);
  return CharReqHdr.r_status & S_BUSY;
}

void con_flush(struct dhdr FAR *pdev)
{
  CharCmd(pdev, C_IFLUSH);
}

void con_flush_stdin(void)
{
  struct dhdr FAR *dev = sft_to_dev(get_sft(STDIN));
  if (dev)
    con_flush(dev);
}

/* if the sft is invalid, then we just monitor syscon */
struct dhdr FAR *sft_to_dev(sft FAR *s)
{
  if (FP_OFF(s) == (UWORD) -1)
    return syscon;
  if (s->sft_flags & SFT_FDEVICE)
    return s->sft_dev;
  return NULL;
}

int StdinBusy(void)
{
  sft FAR *s = get_sft(STDIN);
  struct dhdr FAR *dev = sft_to_dev(s);

  if (dev)
    return Busy(dev);

  return s->sft_posit >= s->sft_size;
}

/* get character from the console - this is how DOS gets
   CTL_C/CTL_S/CTL_P when outputting */
int ndread(struct dhdr FAR *pdev)
{
  CharCmd(pdev, C_NDREAD);
  if (CharReqHdr.r_status & S_BUSY)
    return -1;
  return CharReqHdr.r_ndbyte;
}

/* OUTPUT FUNCTIONS */

#ifdef __WATCOMC__
void fast_put_char(char c);
#pragma aux fast_put_char = "int 29h" parm[al] modify exact [bx]
#else

/* writes a character in raw mode using int29 for speed */
STATIC void fast_put_char(unsigned char chr)
{
#if defined(__TURBOC__)
    _AL = chr;
    __int__(0x29);
#elif defined(I86)
    asm
    {
      mov al, byte ptr chr;
      int 0x29;
    }
#else
  iregs r = {};
  r.a.b.l = chr;
  call_intr(0x29, MK_FAR_SCP(r));
#endif
}
#endif

void update_scr_pos(unsigned char c, unsigned char count)
{
  unsigned char scrpos = scr_pos;

  if (c == CR)
    scrpos = 0;
  else if (c == BS) {
    if (scrpos > 0)
      scrpos--;
  } else if (c != LF && c != BELL) {
    scrpos += count;
  }
  scr_pos = scrpos;
}

STATIC int raw_get_char(struct dhdr FAR *pdev, BOOL check_break);

long cooked_write(struct dhdr FAR *pdev, size_t n, __XFAR(const char)bp)
{
  size_t xfer;

  /* bit 7 means fastcon; low 5 bits count number of characters */
  unsigned char fast_counter = (pdev->dh_attr & ATTR_FASTCON) << 3;

  for (xfer = 0; xfer < n; xfer++)
  {
    int err;
    unsigned char count = 1, c = *bp++;

    if (c == CTL_Z)
      break;

    /* write a character in cooked mode; maybe with printer echo;
       handles TAB expansion */
    if (c == HT) {
      count = 8 - (scr_pos & 7);
      c = ' ';
    }
    update_scr_pos(c, count);
    do {
      /* if not fast then < 0x80; always check
         otherwise check every 32 characters */
      if (fast_counter <= 0x80 && ndread(pdev) == CTL_S)
        raw_get_char(pdev, TRUE); /* Test for hold char and ctl_c */
      fast_counter++;
      fast_counter &= 0x9f;
      if (PrinterEcho)
        CharIOfd(STDPRN, c, C_OUTPUT);
      if (fast_counter & 0x80)
        fast_put_char(c);
      else
      {
        err = CharIO(pdev, c, C_OUTPUT);
        if (err < 0)
          return err;
      }
    } while (--count != 0);
  }
  return xfer;
}

/* writes character for disk file or device */
void write_char(int c, int sft_idx)
{
  unsigned char ch = (unsigned char)c;
  DosWriteSftUnchecked(sft_idx, 1, MK_FAR_SCP(ch));
}

void write_char_stdout(int c)
{
  unsigned char count = 1;
  unsigned flags = get_sft(STDOUT)->sft_flags;

  /* ah=2, ah=9 should expand tabs even for raw devices and disk files */
  if ((flags & (SFT_FDEVICE|SFT_FBINARY)) != SFT_FDEVICE)
  {
    if (c == HT) {
      count = 8 - (scr_pos & 7);
      c = ' ';
    }
    /* for raw CONOUT devices already updated in dosfns.c */
    if ((flags & (SFT_FDEVICE|SFT_FCONOUT)) != (SFT_FDEVICE|SFT_FCONOUT))
      update_scr_pos(c, count);
  }

  do {
    write_char(c, get_sft_idx(STDOUT));
  } while (--count != 0);
}

#define iscntrl(c) ((unsigned char)(c) < ' ')

/* this is for handling things like ^C, mostly used in echoed input */
STATIC int echo_char(int c, int sft_idx)
{
  int out = c;
  if (iscntrl(c) && c != HT && c != LF && c != CR)
  {
    write_char('^', sft_idx);
    out += '@';
  }
  write_char(out, sft_idx);
  return c;
}

STATIC void destr_bs(int sft_idx)
{
  write_char(BS, sft_idx);
  write_char(' ', sft_idx);
  write_char(BS, sft_idx);
}

/* READ FUNCTIONS */

long cooked_read(struct dhdr FAR *pdev, size_t n, char FAR *bp,
    BOOL check_break)
{
  unsigned xfer = 0;
  int c;
  while(n--)
  {
    c = raw_get_char(pdev, check_break);
    if (c < 0)
      return c;
    if (c == 256)
      break;
    *bp++ = c;
    xfer++;
    if ((unsigned char)c == CTL_Z)
      break;
  }
  return xfer;
}


STATIC unsigned do_read_char_dev(struct dhdr FAR *pdev, BOOL check_break)
{
  unsigned c;

  FOREVER
  {
    if (!Busy(pdev))
    {
      c = CharIO(pdev, 0, C_INPUT);
      if (c == CTL_C && !check_break && !break_ena)
        clear_break();
      break;
    }
    if (pdev != syscon)
    {
      if (check_break)
      {
        check_handle_break(syscon);
      }
      else
      {
        unsigned char c1 = ndread(syscon);
        if (c1 == CTL_C)
        {
          con_flush(syscon);
          c = -1;  // EOF
          break;
        }
      }
    }
    /* the idle int is only safe if we're using the character stack */
    if (user_r->AH < 0xd)
      DosIdle_int();
  }

  return c;
}

STATIC unsigned read_char_sft_dev(struct dhdr FAR *pdev,
                                  BOOL check_break)
{
  unsigned c = do_read_char_dev(pdev, check_break);
  if (!c)
    return 0;
  /* check for break or stop on sft_in, echo to sft_out */
  if (check_break && (c == CTL_C || c == CTL_S))
  {
    if (c == CTL_S)
      c = do_read_char_dev(pdev, FALSE);
    if (c == CTL_C)
      handle_break(pdev);
    /* DOS oddity: if you press ^S somekey ^C then ^C does not break */
    c = do_read_char_dev(pdev, FALSE);
  }
  return c;
}

STATIC int raw_get_char(struct dhdr FAR *pdev, BOOL check_break)
{
  return read_char_sft_dev(pdev, check_break);
}

static unsigned char dev_read_char(int sft_in, BOOL check_break)
{
  struct dhdr FAR *dev = sft_to_dev(idx_to_sft(sft_in));
  return read_char_sft_dev(dev, check_break);
}

/* reads a line (buffered, called by int21/ah=0ah, 3fh) */
static void do_read_line(int sft_in, kbd0a FAR *kp, BOOL check_break,
    unsigned char (*do_read_char)(int, BOOL))
{
  unsigned c;
  unsigned cu_pos = scr_pos;
  unsigned count = 0, stored_pos = 0;
  unsigned size = kp->kb_size, stored_size = kp->kb_count;
  BOOL insert = FALSE, first = TRUE;
  int sft_out = sft_in;

  if (size == 0)
    return;

  /* the stored line is invalid unless it ends with a CR */
  if (kp->_kb_buf[stored_size] != CR)
    stored_size = 0;

  do
  {
    unsigned new_pos = stored_size;

    c = do_read_char(sft_in, check_break);
    if (c == 0)
      c = (unsigned)do_read_char(sft_in, check_break) << 8;
    switch (c)
    {
      case 0:
        return;
      case LF:
        /* show LF if it's not the first character. Never store it */
        if (!first)
        {
          write_char(CR, sft_out);
          write_char(LF, sft_out);
        }
        break;

      case CTL_F:
        break;

      case RIGHT:
      case F1:
        if (stored_pos < stored_size && count < size - 1)
          local_buffer[count++] = echo_char(kp->_kb_buf[stored_pos++], sft_out);
        break;

      case F2:
      case F4:
        /* insert/delete up to character c */
        {
          unsigned char c2 = do_read_char(sft_in, check_break);
          new_pos = stored_pos;
          if (c2 == 0)
          {
            do_read_char(sft_in, check_break);
          }
          else
          {
            char *sp = (char *)memchr(&kp->_kb_buf[stored_pos],
                                   c2, stored_size - stored_pos);
            if (sp != NULL)
                new_pos = (sp - &kp->_kb_buf[stored_pos]) + 1;
          }
        }
        /* fall through */
      case F3:
        if (c != F4) /* not delete */
        {
          while (stored_pos < new_pos && count < size - 1)
              local_buffer[count++] = echo_char(kp->_kb_buf[stored_pos++], sft_out);
        }
        stored_pos = new_pos;
        break;

      case F5:
        memcpy(kp->_kb_buf, local_buffer, count);
        stored_size = count;
        write_char('@', sft_out);
        goto start_new_line;

      case INS:
        insert = !insert;
        break;

      case DEL:
        stored_pos++;
        break;

      case LEFT:
      case CTL_BS:
      case BS:
        if (count > 0)
        {
          unsigned new_pos;
          char c2 = local_buffer[--count];
          if (c2 == HT)
          {
            unsigned i;
            new_pos = cu_pos;
            for (i = 0; i < count; i++)
            {
              if (local_buffer[i] == HT)
                new_pos = (new_pos + 8) & ~7;
              else if (iscntrl(local_buffer[i]))
                new_pos += 2;
              else
                new_pos++;
            }
            do
              destr_bs(sft_out);
            while (scr_pos > new_pos);
          }
          else
          {
            if (iscntrl(c2))
              destr_bs(sft_out);
            destr_bs(sft_out);
          }
        }
        if (stored_pos > 0)
          stored_pos--;
        break;

      case ESC:
        write_char('\\', sft_out);
    start_new_line:
        write_char(CR, sft_out);
        write_char(LF, sft_out);
        for (count = 0; count < cu_pos; count++)
          write_char(' ', sft_out);
        count = 0;
        stored_pos = 0;
        insert = FALSE;
        break;

      case F6:
        c = CTL_Z;
        /* fall through */

      default:
        if (count < size - 1 || c == CR)
          local_buffer[count++] = echo_char(c, sft_out);
        else
          write_char(BELL, sft_out);
        if (stored_pos < stored_size && !insert)
          stored_pos++;
        break;
    }
    first = FALSE;
  } while (c != CR);
  memcpy(kp->_kb_buf, local_buffer, count);
  /* if local_buffer overflows into the CON default buffer we
     must invalidate it */
  if (count > LINEBUFSIZECON)
    ____R(kb_buf.kb_size) = 0;
  /* kb_count does not include the final CR */
  kp->kb_count = count - 1;
}

void read_line(int sft_in, kbd0a FAR *kp, BOOL check_break)
{
  do_read_line(sft_in, kp, check_break, read_char);
}

/* called by handle func READ (int21/ah=3f) */
size_t read_line_handle(int sft_idx, size_t n, char FAR * bp, BOOL check_break)
{
  size_t chars_left;

  if (inputptr == NULL)
  {
    /* can we reuse kb_buf or was it overwritten? */
    if (kb_buf.kb_size != LINEBUFSIZECON)
    {
      ____R(kb_buf.kb_count) = 0;
      ____R(kb_buf.kb_size) = LINEBUFSIZECON;
    }
    do_read_line(sft_idx, (kbd0a FAR *)&kb_buf, check_break, dev_read_char);
    kb_buf._kb_buf[kb_buf.kb_count + 1] = echo_char(LF, sft_idx);
    inputptr = kb_buf._kb_buf;
    if (*inputptr == CTL_Z)
    {
      inputptr = NULL;
      return 0;
    }
  }

  chars_left = &kb_buf._kb_buf[kb_buf.kb_count + 2] - inputptr;
  if (n > chars_left)
    n = chars_left;

  n_fmemcpy(bp, inputptr, n);
  inputptr += n;
  if (n == chars_left)
    inputptr = NULL;
  return n;
}
