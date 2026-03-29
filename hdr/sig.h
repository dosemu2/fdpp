/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2017-2026  @stsp
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

typedef struct {
  UWORD sig_off;
  UWORD sig_seg;
  UBYTE sig_num;
  UBYTE sig_action;
} sigact;
ANNOTATE_SIZE(sigact, 6);

enum { _SIG_DFL, _SIG_IGN, _SIG_GET };
enum { _SIGINTR = 1, _SIGHUP, _SIGTERM = 8, _SIGPIPE, _SIGUSER1 = 0xd,
       _SIGUSER2 };
