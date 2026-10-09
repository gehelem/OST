#!/bin/bash
#
# CI - Arch Linux package build (issue #230)
#
# Builds straight from this checkout (not a downloaded tarball) - same
# reasoning as CI-build-rpm.sh building directly with cmake/cpack
# instead of going through a separately maintained spec file. The
# committed arch/PKGBUILD (the one published on AUR) stays a normal,
# tag-pinned package; this script only overrides pkgver + blanks out
# `source` for this one build, it never touches the committed file.
#
set -e

pacman -Syu --noconfirm --needed \
    base-devel git cmake \
    qt6-base qt6-tools qt6-scxml qt6-websockets \
    mesa libnova gsl wcslib cfitsio avahi eigen \
    libraw libsecret opencv libindi extra-cmake-modules

# makepkg refuses to run as root (by design) - throwaway build user.
useradd -m builder

# Version string computed the same way DebianPackage.cmake derives
# OST_UPSTREAM_VERSION, so the Arch "latest" artifact's version lines up
# with the Debian one for the same commit. pacman pkgver only allows
# [a-zA-Z0-9._] - no "+" or "-", hence ".r<n>.<sha>" instead of
# "+git<n>.g<sha>".
if GIT_DESCRIBE=$(git describe --tags --long --match "[0-9]*" 2>/dev/null) \
   && [[ "${GIT_DESCRIBE}" =~ ^([0-9]+\.[0-9]+\.[0-9]+)-([0-9]+)-g([0-9a-f]+)$ ]]; then
    TAG="${BASH_REMATCH[1]}"
    COMMITS="${BASH_REMATCH[2]}"
    SHA="${BASH_REMATCH[3]}"
    if [ "${COMMITS}" = "0" ]; then
        PKGVER="${TAG}"
    else
        PKGVER="${TAG}.r${COMMITS}.${SHA}"
    fi
else
    PKGVER="0.0.0.r$(git rev-list --count HEAD).g$(git rev-parse --short HEAD)"
fi
echo "Computed pkgver: ${PKGVER}"

# Lay out a local-source build: PKGBUILD's build() does
# `cmake -S "${pkgbase}-${pkgver}"` relative to $srcdir, which makepkg
# always sets to "<PKGBUILD dir>/src/" - even with source=() empty,
# confirmed from the actual build log ("source directory .../src/...
# does not exist"), not the PKGBUILD's own directory as first assumed.
BUILD_DIR=/tmp/build
mkdir -p "${BUILD_DIR}/src/ostserver-${PKGVER}"
cp -a . "${BUILD_DIR}/src/ostserver-${PKGVER}/"
sed \
    -e "s/^pkgver=.*/pkgver=${PKGVER}/" \
    -e "s/^source=.*/source=()/" \
    arch/PKGBUILD > "${BUILD_DIR}/PKGBUILD"
cp arch/ostserver.install arch/ostserver-complete.install "${BUILD_DIR}/"

chown -R builder:builder "${BUILD_DIR}"
cd "${BUILD_DIR}"
su builder -c "makepkg --noconfirm --skipinteg"

PKG_OSTSERVER=$(ls ostserver-*-x86_64.pkg.tar.zst | grep -v complete | head -1)
PKG_COMPLETE=$(ls ostserver-complete-*-x86_64.pkg.tar.zst | head -1)
echo "Packages built: ${PKG_OSTSERVER}  ${PKG_COMPLETE}"

cp "${PKG_OSTSERVER}" "${CI_PROJECT_DIR}/${PKG_FILE_LATEST}"
cp "${PKG_COMPLETE}"  "${CI_PROJECT_DIR}/${PKG_FILE_COMPLETE_LATEST}"
if [ -n "${CI_COMMIT_TAG}" ]; then
    cp "${PKG_OSTSERVER}" "${CI_PROJECT_DIR}/${PKG_FILE_TAG}"
    cp "${PKG_COMPLETE}"  "${CI_PROJECT_DIR}/${PKG_FILE_COMPLETE_TAG}"
fi

echo "Build complete: ${PKG_OSTSERVER}  ${PKG_COMPLETE}"
