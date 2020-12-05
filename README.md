# OST
Observatoire Sans Tête
======================

install first from fresh ubuntu server install
==============================================

sudo apt-add-repository ppa:mutlaqja/ppa
sudo apt-get update
sudo apt upgrade
sudo apt-get install build-essential cmake git libstellarsolver-dev libeigen3-dev libcfitsio-dev zlib1g-dev libindi-dev extra-cmake-modules libkf5plotting-dev libqt5svg5-dev libkf5xmlgui-dev libkf5kio-dev kinit-dev libkf5newstuff-dev kdoctools-dev libkf5notifications-dev qtdeclarative5-dev libkf5crash-dev gettext libnova-dev libgsl-dev libraw-dev libkf5notifyconfig-dev wcslib-dev libqt5websockets5-dev xplanet xplanet-images qt5keychain-dev libsecret-1-dev breeze-icon-theme indi-full libindi-dev cimg-dev libnova-dev build-essential nginx libgsl-dev wcslib-dev libcfitsio-dev qt5-default libcurl4-gnutls-dev libfftw3-dev

get build and install indi from source to get missing baseclientqt missing header and library
=============================================================================================

see
https://indilib.org/forum/development/8316-baseclientqt-header-and-lib-missing-in-dev-ppa.html


git clone https://github.com/indilib/indi.git
cd indi
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug -DINDI_BUILD_QT5_CLIENT=ON  ..
make -j4
sudo make install

add this at the end of .bashrc :
export QT_QPA_PLATFORM=offscreen



