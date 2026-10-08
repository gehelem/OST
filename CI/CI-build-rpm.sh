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

# Package definitions (names, Requires, %post/%preun scripts) now live
# in cmake/cpack/RpmPackage.cmake, same as the Debian side's own
# DebianPackage.cmake - only the generator selection stays here.
cd build
cpack -G RPM
cd ..

# Fixed names (CPACK_RPM_<COMPONENT>_FILE_NAME in RpmPackage.cmake) -
# the default would have been CPACK_PACKAGE_NAME-based ("OST-...-Linux-
# <Component>.rpm"), not matching any of these, confirmed the hard way.
RPM_RUNTIME="build/ostserver.rpm"
RPM_DEV="build/ostserver-devel.rpm"
RPM_COMPLETE="build/ostserver-complete.rpm"
echo "RPMs built: ${RPM_RUNTIME}  ${RPM_DEV}  ${RPM_COMPLETE}"
rpm -qpi "${RPM_RUNTIME}"
rpm -qp --requires "${RPM_RUNTIME}"

cp "${RPM_RUNTIME}"  "./${RPM_FILE_LATEST}"
cp "${RPM_DEV}"      "./${RPM_FILE_DEV_LATEST}"
cp "${RPM_COMPLETE}" "./${RPM_FILE_COMPLETE_LATEST}"
if [ -n "${CI_COMMIT_TAG}" ]; then
    cp "${RPM_RUNTIME}"  "./${RPM_FILE_TAG}"
    cp "${RPM_DEV}"      "./${RPM_FILE_DEV_TAG}"
    cp "${RPM_COMPLETE}" "./${RPM_FILE_COMPLETE_TAG}"
fi

echo "Build complete: ${RPM_RUNTIME}  ${RPM_DEV}  ${RPM_COMPLETE}"
