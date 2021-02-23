#!/bin/sh

if [ "x${TRAVIS}" = "xtrue" ] ; then
  export CI=true
fi

if [ "x${CI}" = "xtrue" ] ; then
  git config user.email "cibuild@example.com"
  git config user.name "CI build"
  git tag tmp -m "make git-describe happy"
fi

make clean all
