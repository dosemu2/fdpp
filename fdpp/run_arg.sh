#!/bin/sh

if [ "$1" = "-i" ]; then
  cat >$2
  shift 2
fi
OUT=$1
shift
PRG=$1
shift
$PRG $* 1>&2
cat $OUT
