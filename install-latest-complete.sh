#!/bin/bash
#
# Downloads and installs the latest "ostserver-complete" package
# (issue #229): backend + web UI + nginx, ready to use after this
# single script. Everything resolves from the standard distro
# archives EXCEPT libindi1: INDI has never shipped in Ubuntu's own
# repos (confirmed 2026-10, Ubuntu 26.04 "resolute" included - and
# older INDI PPA builds for pre-26.04 series are gone too), only via
# the ppa:mutlaqja/ppa PPA (same one the old install.sh/install_daily.sh
# used), so we add that one PPA here - the beginner still only runs
# this one script, no manual repo wrangling.
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

echo "Adding the INDI library PPA (libindi1 is not in the standard Ubuntu archives)..."
sudo add-apt-repository -y ppa:mutlaqja/ppa
sudo apt update

echo "Installing ${DEB_FILE}..."
sudo apt install -y "./${DEB_FILE}"

echo "Done - ostserver-complete is installed, the service should be starting."
echo "Web UI: http://<this-machine>/"
