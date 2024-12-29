#!/bin/sh

cat >$1
shift
OUT=$1
shift
PRG=$1
shift
$PRG $* 1>&2
cat $OUT
