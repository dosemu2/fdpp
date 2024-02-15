#!/bin/sh

sudo add-apt-repository ppa:stsp-0/nasm-segelf
sudo add-apt-repository ppa:stsp-0/thunk-gen
sudo apt update -q
sudo apt install -y \
    bison \
    flex \
    sed \
    bash \
    clang \
    nasm-segelf \
    thunk-gen \
    binutils \
    pkgconf \
    autoconf \
    libelf-dev \
    git \
    diffutils
