#!/bin/bash
#
# Quick local dev build: clone the repo and compile it, with the same
# dependencies as the CI build image (CI/Dockerfile.build) - kept in
# sync with it by hand, there are only two places that list these.
#
set -e

sudo apt-get update
sudo apt-get install -y software-properties-common
sudo add-apt-repository -y universe

sudo apt-get install -y \
    build-essential cmake git npm file dpkg-dev ca-certificates wget \
    qt6-base-dev qt6-scxml-dev qt6-tools-dev-tools qt6-l10n-tools libgl1-mesa-dev \
    libnova-dev libgsl-dev wcslib-dev libcfitsio-dev \
    libavahi-client-dev libavahi-common-dev \
    zlib1g-dev libeigen3-dev libraw-dev libsecret-1-dev libopencv-dev \
    extra-cmake-modules

# qt6-websockets-dev only exists from Ubuntu 24.04 on; 22.04 needs the
# older package name.
. /etc/os-release
if dpkg --compare-versions "${VERSION_ID}" lt 24.04; then
    sudo apt-get install -y libqt6websockets6-dev
else
    sudo apt-get install -y qt6-websockets-dev
fi

# INDI + XISF + GSC: never shipped in Ubuntu's own archives, and
# ppa:mutlaqja/ppa only publishes for 26.04 since August 2026 - OST
# ships a frozen 2.2.4 bundle in its own registry for all three LTS
# releases instead (see documentation/content/install/fresh.en.md §2).
#
# Pick the one for your release:
INDI_TAR=indi-deps-noble.tar      # Ubuntu 24.04 (Noble)
# INDI_TAR=indi-deps-jammy.tar    # Ubuntu 22.04 (Jammy)
# INDI_TAR=indi-deps-resolute.tar # Ubuntu 26.04 (Resolute)

wget "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/indi-deps/2.2.4/${INDI_TAR}"
tar xf "${INDI_TAR}"
sudo apt-get install -y ./indi-deps/*.deb

cd
git clone https://gitlab.ostserver.fr/ost/ostserver.git
cd ostserver
mkdir build
cd build
cmake ..
make -j"$(nproc)"
# sudo cmake --install . --prefix=/
