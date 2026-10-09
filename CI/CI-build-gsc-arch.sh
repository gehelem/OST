#!/bin/bash
#
# CI - one-shot build: gsc package for Arch Linux (issue #230 follow-up)
# Triggered manually (BUILD_GSC_PACKAGES=true), not part of the normal
# pipeline - see CI/gsc.yml's own header comment.
#
set -e

pacman -Syu --noconfirm --needed base-devel git wget

# makepkg refuses to run as root (by design) - throwaway build user,
# same pattern as CI-build-arch.sh.
useradd -m builder
mkdir -p /tmp/build
cp "${CI_PROJECT_DIR}/gsc/arch/PKGBUILD" /tmp/build/
chown -R builder:builder /tmp/build
cd /tmp/build
su builder -c "makepkg --noconfirm"

PKG=$(ls gsc-*-x86_64.pkg.tar.zst | head -1)
cp "${PKG}" "${CI_PROJECT_DIR}/gsc_arch_x86_64.pkg.tar.zst"
echo "Built: ${PKG}"

cd "${CI_PROJECT_DIR}"
echo "Uploading to the generic package registry..."
curl --fail --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" \
    --upload-file gsc_arch_x86_64.pkg.tar.zst \
    "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/packages/generic/gsc/1.2/gsc_arch_x86_64.pkg.tar.zst"
echo "Done."
