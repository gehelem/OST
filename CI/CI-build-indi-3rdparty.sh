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

# Even though the build below is narrowed to INDI_3RDPARTY_DRIVERS,
# CMake still configures every driver's own CMakeLists before anything
# gets built - so this needs the project's FULL dependency list (see
# its own README's apt-get line), mapped to Fedora -devel package
# names, or an unrelated, unwanted driver's missing REQUIRED dependency
# still aborts configure before it ever reaches the ones actually
# wanted.
# --skip-unavailable: a few of these (limesuite, rtl-sdr) may not exist
# on every Fedora release - skip them rather than fail the whole job,
# same as FFMPEG/libudev already being silently skipped by cmake itself
# further down (optional, not REQUIRED there).
dnf -y install --skip-unavailable \
    gcc-c++ cmake make git rpm-build \
    libindi-devel libnova-devel libusb1-devel curl-devel jsoncpp-devel \
    gsl-devel cfitsio-devel zlib-devel libjpeg-turbo-devel libtiff-devel \
    fftw-devel libftdi-devel gpsd-devel LibRaw-devel libdc1394-devel \
    libgphoto2-devel boost-devel rtl-sdr-devel limesuite-devel \
    zeromq-devel systemd-devel hidapi-devel json-devel \
    rubygems ruby-devel redhat-rpm-config
gem install --no-document fpm

echo "libindi-devel $(rpm -q --qf '%{VERSION}' libindi-devel)"

git clone --depth 1 --branch "${INDI_3RDPARTY_TAG}" \
    https://github.com/indilib/indi-3rdparty.git /tmp/indi-3rdparty

# Every WITH_<driver> option defaults On in this project's own
# CMakeLists - there's no "build only these" switch, so this turns all
# of them Off except the ones listed in INDI_3RDPARTY_DRIVERS, rather
# than hand-maintaining a disable list here that silently goes stale
# whenever upstream adds a new driver (that driver's own dependencies
# would then need adding above too, same as eqmod/sx/weewx-json did).
DRIVER_FLAGS=""
for opt in $(grep -oP '^option\(\K(WITH_[A-Za-z0-9_-]+)' /tmp/indi-3rdparty/CMakeLists.txt); do
    if printf '%s\n' "${INDI_3RDPARTY_DRIVERS}" | grep -qw "${opt}"; then
        DRIVER_FLAGS="${DRIVER_FLAGS} -D${opt}=1"
    else
        DRIVER_FLAGS="${DRIVER_FLAGS} -D${opt}=0"
    fi
done
echo "Driver flags: ${DRIVER_FLAGS}"

# indi-3rdparty builds in two disjoint passes, picked by BUILD_LIBS:
# Off (default) builds the actual INDI drivers (indi_asi_ccd etc.), but
# each one's find_package(ASI)/find_package(QHY) only looks at paths a
# SYSTEM install would already provide (FindASI.cmake/FindQHY.cmake
# call plain find_path/find_library, nothing pointed at this checkout)
# - on On, it builds just the vendor SDKs (libasi, libqhy...) instead.
# One pass alone therefore never produces both: with BUILD_LIBS=0 on a
# clean system, ASI_FOUND/QHY_FOUND come back false and it silently
# falls back to building libasi/libqhy AGAIN instead of indi-asi/
# indi-qhy - no error, just the driver quietly missing from the result
# (caught only by Gilles noticing no indi_asi_ccd in the finished rpm).
# Pass 1 installs for real (no DESTDIR) so pass 2's find_package calls
# can see it, same build/binary-safe container either way.
cmake -B /tmp/indi-3rdparty/build-libs -S /tmp/indi-3rdparty \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_LIBS=1 \
    ${DRIVER_FLAGS}
cmake --build /tmp/indi-3rdparty/build-libs -j"$(nproc)"
cmake --install /tmp/indi-3rdparty/build-libs
DESTDIR=/tmp/indi-3rdparty-stage cmake --install /tmp/indi-3rdparty/build-libs

cmake -B /tmp/indi-3rdparty/build-drivers -S /tmp/indi-3rdparty \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
    ${DRIVER_FLAGS}
cmake --build /tmp/indi-3rdparty/build-drivers -j"$(nproc)"
DESTDIR=/tmp/indi-3rdparty-stage cmake --install /tmp/indi-3rdparty/build-drivers

# fpm's argument parser (clamp) refuses any -flag once it has seen the
# first positional argument - "usr" (what -C's directory is relative
# to) must come last, after every -flag/--option.
# --iteration tied to CI_PIPELINE_ID: without it, rebuilding the same
# INDI_3RDPARTY_TAG with different driver content still produces the
# exact same NVR, and dnf treats that as already installed and no-ops
# instead of upgrading.
fpm -s dir -t rpm \
    -n indi-3rdparty-custom \
    -v "${INDI_3RDPARTY_TAG#v}" \
    --iteration "${CI_PIPELINE_ID}" \
    --depends libindi \
    -p "./${RPM_FILE_INDI_3RDPARTY}" \
    -C /tmp/indi-3rdparty-stage \
    usr

echo "Built: ${RPM_FILE_INDI_3RDPARTY}"
rpm -qpi "${RPM_FILE_INDI_3RDPARTY}"
