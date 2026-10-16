#!/bin/bash
#
# CI - indi-3rdparty RPM build (manual job, see CI/indi-3rdparty.yml)
#
# indi-3rdparty has no CPack/RPM packaging of its own (checked directly
# in its CMakeLists.txt - no include(CPack), no CPACK_* vars) - the
# COPR build of it relies on separately maintained .spec files instead.
# fpm wraps a plain `make install DESTDIR=` tree into an RPM here rather
# than maintaining our own .spec for a driver set we don't ship.
#
set -e

dnf -y install \
    gcc-c++ cmake make git rpm-build \
    libindi-devel libnova-devel libusb1-devel curl-devel jsoncpp-devel \
    rubygems ruby-devel redhat-rpm-config
gem install --no-document fpm

echo "libindi-devel $(rpm -q --qf '%{VERSION}' libindi-devel)"

git clone --depth 1 --branch "${INDI_3RDPARTY_TAG}" \
    https://github.com/indilib/indi-3rdparty.git /tmp/indi-3rdparty

cmake -B /tmp/indi-3rdparty/build -S /tmp/indi-3rdparty \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
    ${INDI_3RDPARTY_CMAKE_FLAGS}
cmake --build /tmp/indi-3rdparty/build -j"$(nproc)"
DESTDIR=/tmp/indi-3rdparty-stage cmake --install /tmp/indi-3rdparty/build

fpm -s dir -t rpm \
    -n indi-3rdparty-custom \
    -v "${INDI_3RDPARTY_TAG#v}" \
    --depends libindi \
    -C /tmp/indi-3rdparty-stage usr \
    -p "./${RPM_FILE_INDI_3RDPARTY}"

echo "Built: ${RPM_FILE_INDI_3RDPARTY}"
rpm -qpi "${RPM_FILE_INDI_3RDPARTY}"
