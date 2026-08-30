#!/bin/bash
#
# CI - build job
#
# The build environment (toolchain, Qt6, INDI/XISF/GSC) is pre-baked in the
# image $CI_REGISTRY_IMAGE/ci-build:<series>, built from CI/Dockerfile.build by
# CI/ci-images.yml. Nothing is installed here anymore.
#
set -e

# Fail early if the image is stale / missing INDI.
INDI_VERSION=$(dpkg-query -W -f='${Version}' libindi-dev)
if ! dpkg --compare-versions "${INDI_VERSION}" ge "2.0~"; then
    echo "ERROR: libindi-dev ${INDI_VERSION} is older than 2.0 - CI image out of date?" >&2
    exit 1
fi

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
