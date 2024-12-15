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

INST=$(pwd)/${DIR_INSTALLED_FDPP}

if false ; then
  # Old Makefile based build
  make clean all prefix=${INST}
else
  # Ubuntu 22.04 has too old a Meson, so pip install it
  sudo apt install ninja-build pipx
  pipx install meson
  export PATH=${HOME}/.local/bin:${PATH}

  ./configure.meson --prefix ${INST} build
  meson compile --verbose -C build
fi
