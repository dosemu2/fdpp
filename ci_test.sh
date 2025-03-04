#!/bin/sh

set -e

. ./ci_test_prereq.sh

if [ -z "${DIR_TEST_DOSEMU}" ] ; then
   echo env var "DIR_TEST_DOSEMU" is empty or missing
   exit 1
fi

if [ -z "${DIR_INSTALLED_FDPP}" ] ; then
   echo env var "DIR_INSTALLED_FDPP" is empty or missing
   exit 1
fi

DIR_ROOT="$(pwd)"

(
  cd ${DIR_TEST_DOSEMU} || exit 2

  # Now do the tests
  export LD_LIBRARY_PATH=${DIR_ROOT}/${DIR_INSTALLED_FDPP}/lib/fdpp
  export SKIP_EXPENSIVE=1
  test/test_dosemu.py PPDOSGITTestCase
)
