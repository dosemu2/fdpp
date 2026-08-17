#!/bin/sh

set -e

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
  export PYTHONUNBUFFERED=1
  export SKIP_EXPENSIVE=1

  cat >&2 << EOF
===============================================================
=      Processor tests run on emulated CPU, KVM and VM86      =
===============================================================
EOF
  env NO_FAILFAST=1 python3 test/test_processor.py

cat >&2 << EOF2
===============================================================
=                    Other tests run on KVM                   =
===============================================================
EOF2
  test/test_dosemu.py PPDOSGITTestCase
)
