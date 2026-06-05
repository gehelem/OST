#!/bin/bash
#
# CI - build job
#
set -e
apt-get update
apt-get clean
apt-get install -y software-properties-common
apt-add-repository ppa:mutlaqja/ppa
apt-get update
apt-get install -y build-essential cmake git npm file dpkg-dev \
    qt6-base-dev qt6-websockets-dev qt6-scxml-dev qt6-tools-dev-tools \
    libindi-dev libnova-dev libgsl-dev \
    wcslib-dev libcfitsio-dev libavahi-client-dev libavahi-common-dev \
    zlib1g-dev libeigen3-dev libraw-dev libsecret-1-dev libopencv-dev \
    extra-cmake-modules

mkdir -p build media
cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
cpack -G DEB
cd ..

DEB_RUNTIME=$(ls build/ostserver_*.deb | head -1)
DEB_DEV=$(ls build/ostserver-dev_*.deb | head -1)
echo "Packages built: ${DEB_RUNTIME}  ${DEB_DEV}"
cp "${DEB_RUNTIME}" "./${DEB_FILE_LATEST}"
cp "${DEB_RUNTIME}" "./${DEB_FILE_TAG}"
cp "${DEB_DEV}" "./${DEB_FILE_DEV_LATEST}"
cp "${DEB_DEV}" "./${DEB_FILE_DEV_TAG}"

echo "Build complete: ${DEB_RUNTIME}  ${DEB_DEV}"
