#!/bin/bash
#
# CI - Fedora RPM build job
#
set -e

dnf -y install \
    gcc-c++ make cmake git file rpm-build \
    qt6-qtbase-devel qt6-qtwebsockets-devel qt6-qtscxml-devel qt6-qttools-devel \
    mesa-libGL-devel \
    libindi-devel libnova-devel gsl-devel wcslib-devel cfitsio-devel \
    avahi-devel zlib-devel eigen3-devel LibRaw-devel libsecret-devel opencv-devel \
    extra-cmake-modules

echo "libindi-devel $(rpm -q --qf '%{VERSION}' libindi-devel)"

cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j"$(nproc)"

cd build
cpack -G RPM \
    -D CPACK_RPM_COMPONENT_INSTALL=ON \
    -D CPACK_RPM_RUNTIME_PACKAGE_NAME=ostserver \
    -D CPACK_RPM_DEVELOPMENT_PACKAGE_NAME=ostserver-devel \
    -D CPACK_RPM_PACKAGE_URL=https://gitlab.ostserver.fr/ost/ostserver/
cd ..

RPM_DEV=$(ls build/*devel*.rpm | head -1)
RPM_RUNTIME=$(ls build/*.rpm | grep -v devel | head -1)
echo "RPMs built: ${RPM_RUNTIME}  ${RPM_DEV}"
rpm -qpi "${RPM_RUNTIME}"
rpm -qp --requires "${RPM_RUNTIME}"

cp "${RPM_RUNTIME}" "./${RPM_FILE_LATEST}"
cp "${RPM_DEV}"     "./${RPM_FILE_DEV_LATEST}"
if [ -n "${CI_COMMIT_TAG}" ]; then
    cp "${RPM_RUNTIME}" "./${RPM_FILE_TAG}"
    cp "${RPM_DEV}"     "./${RPM_FILE_DEV_TAG}"
fi

echo "Build complete: ${RPM_RUNTIME}  ${RPM_DEV}"
