#!/bin/sh

sudo apt-get update
sudo apt install -y \
  devscripts \
  equivs \
  git

sudo add-apt-repository ppa:stsp-0/nasm-segelf
sudo add-apt-repository ppa:stsp-0/thunk-gen
mk-build-deps --install --root-cmd sudo
