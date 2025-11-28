#!/bin/bash
#
# CI - build job
# 
apt-get update
apt-get clean
apt-get install -y software-properties-common
apt-add-repository ppa:mutlaqja/ppa
apt-get install -y build-essential cmake git npm qtbase5-dev libqt5websockets5-dev libindi-dev libstellarsolver-dev libnova-dev libgsl-dev wcslib-dev libcfitsio-dev libavahi-client-dev libavahi-common-dev zlib1g-dev dpkg-dev qtdeclarative5-dev libqt5svg5-dev libkf5doctools-dev libkf5config-dev libkf5guiaddons-dev libkf5i18n-dev libkf5newstuff-dev libkf5notifications-dev libkf5xmlgui-dev libkf5plotting-dev libkf5crash-dev libkf5notifyconfig-dev libeigen3-dev libraw-dev libsecret-1-dev debhelper devscripts lintian autotools-dev intltool-debian extra-cmake-modules
mkdir media
dpkg-buildpackage -us -uc 
mv ../ostserver_0.0.0_amd64.deb  ./${DEB_FILE_LATEST}
cp ${DEB_FILE_LATEST} ${DEB_FILE_TAG}
dpkg -i ${DEB_FILE_LATEST}
echo "Quick test..."
export QT_QPA_PLATFORM=offscreen
ostserver --webroot=media > server.log 2>&1 &
SERVER_PID=$!
sleep 7
ps aux | grep ostserver | grep -v grep
node test-ws.js 2>&1 | tee ws.log
echo "Server logs:"
tail -50 server.log
kill $SERVER_PID || true

