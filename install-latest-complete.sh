#!/bin/bash
#
# Downloads and installs the latest "ostserver-complete" package
# (issue #229): backend + web UI + nginx, ready to use after this
# single script. No PPA/custom repo needed - nginx and every other
# dependency resolve from the standard distro archives, that's the
# whole point of this package (see cmake/cpack/DebianPackage.cmake's
# Complete-component comment).
#
# Targets Ubuntu 26.04 (build-job-2604 in .gitlab-ci.yml, the only
# mandatory/always-built job). Adjust the filename below if you need
# the 24.04 build instead (ostserver-complete_latest_2404_amd64.deb).
#
set -e

DEB_FILE="ostserver-complete_latest_2604_amd64.deb"
URL="https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB_FILE}"

echo "Downloading ${DEB_FILE}..."
curl --fail --location --output "${DEB_FILE}" "${URL}"

echo "Installing ${DEB_FILE}..."
sudo apt install -y "./${DEB_FILE}"

echo "Done - ostserver-complete is installed, the service should be starting."
echo "Web UI: http://<this-machine>/"
