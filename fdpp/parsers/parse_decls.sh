#!/bin/bash

set -e
set -o pipefail

gen_calls_tmp() {
	grep ASMCFUNC $1 | cpp -P -I $3 -include unfar.h | nl -n ln -v 0 >$2
}

gen_plt_inc() {
	grep ") FAR " $1 | sed 's/\([0-9]\+\).\+ \(SEGM(\(.\+\))\).* \([^ (]\+\) *(.*/asmcfunc_f \4,\t\t\1,\t\3/' >$2
	grep -v ") FAR " $1 | sed 's/\([0-9]\+\).\+ \(SEGM(\(.\+\))\).* \([^ (]\+\) *(.*/asmcfunc_n \4,\t\t\1,\t\3/' >>$2
}

gen_asms_tmp() {
	grep 'ASMFUNC\|ASMPASCAL' $1 | grep -v "//" | \
		cpp -P -I $3 -include unfar.h | nl -n ln -v 0 >$2
}

gen_plt_asmc() {
	grep ASMFUNC $1 | sed 's/\([0-9]\+\).\+ \([^ (]\+\) *(.\+/asmcsym \2,\t\t\1/' >$2
}

gen_plt_asmp() {
	grep ASMPASCAL $1 | sed 's/\([0-9]\+\).\+ \([^ (]\+\) *(.\+/asmpsym \U\2,\t\t\1/' >$2
}


gen_calls_tmp $1 $5 $7
gen_plt_inc $5 $2
gen_asms_tmp $1 $6 $7
gen_plt_asmc $6 $3
gen_plt_asmp $6 $4
