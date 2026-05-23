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
    qtbase5-dev libqt5websockets5-dev libqt5scxml5-dev qttools5-dev-tools \
    qtdeclarative5-dev libqt5svg5-dev \
    libindi-dev libstellarsolver-dev libnova-dev libgsl-dev \
    wcslib-dev libcfitsio-dev libavahi-client-dev libavahi-common-dev \
    zlib1g-dev libeigen3-dev libraw-dev libsecret-1-dev libopencv-dev \
    libkf5doctools-dev libkf5config-dev libkf5guiaddons-dev libkf5i18n-dev \
    libkf5newstuff-dev libkf5notifications-dev libkf5xmlgui-dev \
    libkf5plotting-dev libkf5crash-dev libkf5notifyconfig-dev \
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
