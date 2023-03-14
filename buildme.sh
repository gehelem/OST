#!/bin/bash
sudo apt-add-repository --yes ppa:mutlaqja/ppa
sudo apt-get update
sudo apt upgrade
sudo apt-get install -y \
build-essential \
cmake \
git \
libindi-dev \
indi-dbg \
libstellarsolver-dev \
libnova-dev \
libgsl-dev \
wcslib-dev \
libqt5websockets5-dev \
libcfitsio-dev

cd
git clone https://github.com/gehelem/OST.git
cd OST
mkdir build
cd build
cmake ..
make -j4


