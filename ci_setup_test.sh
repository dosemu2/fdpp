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

test -d ${DIR_TEST_DOSEMU} && (echo $DIR_TEST_DOSEMU already exists && exit 1)

DIR_ROOT="$(pwd)"

# Grab the dosemu2 source
git clone --depth 1 --no-single-branch https://github.com/dosemu2/dosemu2.git ${DIR_TEST_DOSEMU}

# Install fdpp into local directory
if false ; then
  make install prefix="${DIR_ROOT}/${DIR_INSTALLED_FDPP}"
else
  meson install -C ${DIR_ROOT}/build
fi

(
  cd ${DIR_TEST_DOSEMU} || exit 2
  [ -z "${USE_DOSEMU_BRANCH}" ] || git checkout "${USE_DOSEMU_BRANCH}"

  # Remove fdpp-dev from debian build dependencies (use bash as padding)
  sed -i -e 's/fdpp-dev,/bash,/' debian/control

  # Use debian package build deps so we don't have to maintain a list here
  sudo add-apt-repository ppa:stsp-0/dj64
  sudo add-apt-repository -y ppa:dosemu2/ppa
  mk-build-deps --install --root-cmd sudo

  CC=clang ./default-configure -d \
    --with-fdpp-lib-dir=${DIR_ROOT}/${DIR_INSTALLED_FDPP}/lib/fdpp \
    --with-fdpp-include-dir=${DIR_ROOT}/${DIR_INSTALLED_FDPP}/include \
    --with-fdpp-pkgconf-dir=${DIR_ROOT}/${DIR_INSTALLED_FDPP}/lib/pkgconfig
  make

  # Install the FAT mount helper
  sudo cp test/dosemu_fat_mount.sh /bin/.
  sudo chown root.root /bin/dosemu_fat_mount.sh
  sudo chmod 755 /bin/dosemu_fat_mount.sh

  # Install the TAP helper
  sudo cp test/dosemu_tap_interface.sh /bin/.
  sudo chown root.root /bin/dosemu_tap_interface.sh
  sudo chmod 755 /bin/dosemu_tap_interface.sh

  # Install the extra packages needed to run the test suite
  sudo add-apt-repository -y ppa:dosemu2/ppa
  sudo add-apt-repository -y ppa:jwt27/djgpp-toolchain
  sudo add-apt-repository -y ppa:stsp-0/gcc-ia16
  sudo apt update -q

  sudo apt install -y \
    comcom32 \
    python3-cpuinfo \
    python3-pexpect \
    mtools \
    gcc-djgpp \
    djgpp-dev \
    qemu-system-common \
    gcc-ia16-elf \
    libi86-ia16-elf \
    libi86-testsuite-ia16-elf \
    nasm \
    gcc-multilib \
    dos2unix \
    bridge-utils \
    libvirt-daemon \
    libvirt-daemon-system

  # Prepare the cputests
  make -C test/cpu clean all

  # Get the test binaries
  test/test_dos.py --get-test-binaries
)
