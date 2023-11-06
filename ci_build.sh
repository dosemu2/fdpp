#!/bin/sh

if [ "x${TRAVIS}" = "xtrue" ] ; then
  export CI=true
fi

if [ "x${CI}" = "xtrue" ] ; then
  git config user.email "cibuild@example.com"
  git config user.name "CI build"
  git tag tmp -m "make git-describe happy"
fi

if [ -z "${DIR_INSTALLED_FDPP}" ] ; then
   echo env var "DIR_INSTALLED_FDPP" is empty or missing
   exit 1
fi
make clean all PREFIX=`pwd`/${DIR_INSTALLED_FDPP}
