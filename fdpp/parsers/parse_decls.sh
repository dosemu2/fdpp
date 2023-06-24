#!/usr/bin/env bash

set -e
set -o pipefail

gen_calls_tmp() {
	grep ASMCFUNC $1 | $CPP -P -I $srcdir -include unfar.h - | nl -n ln -v 0
}

gen_plt_inc() {
	grep ") FAR " $1 | sed -E 's/([0-9]+).+ (SEGM\((.+)\)).* ([^ \(]+) *\(.*/asmcfunc_f \4, \1, \3/'
	grep -v ") FAR " $1 | sed -E 's/([0-9]+).+ (SEGM\((.+)\)).* ([^ \(]+) *\(.*/asmcfunc_n \4, \1, \3/'
}

gen_asms_tmp() {
	grep 'ASMFUNC\|ASMPASCAL' $1 | grep -v "//" | \
		$CPP -P -I $srcdir -include unfar.h - | nl -n ln -v 0
}

gen_plt_asmc() {
	grep ASMFUNC $1 | \
		sed -E 's/([0-9]+)[[:blank:]]+([^[:blank:]\(]+[[:blank:]]+)+([^ \(]+) *\(.+/asmcsym \3, \1/'
}

gen_plt_asmp() {
	grep ASMPASCAL $1 | tr '[:lower:]' '[:upper:]' | \
		sed -E 's/([0-9]+)[[:blank:]]+([^[:blank:]\(]+[[:blank:]]+)+([^ \(]+) *\(.+/asmpsym \3, \1/'
}

case "$1" in
1)
	gen_calls_tmp $2
	;;
2)
	gen_asms_tmp $2
	;;
3)
	gen_plt_inc $2
	;;
4)
	gen_plt_asmc $2
	;;
5)
	gen_plt_asmp $2
	;;
esac
