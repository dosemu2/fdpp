#!/bin/sh

sudo apt update -q
sudo apt install -y \
    bison \
    flex \
    sed \
    bash \
    clang \
    nasm \
    lld \
    binutils \
    autoconf \
    git
