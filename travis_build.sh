#!/bin/sh

if [ "x${TRAVIS}" = "xtrue" ] ; then
  git tag tmp -m "make git-describe happy"
fi

make clean all
