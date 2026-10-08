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

# Passed through to the package's postinst (cmake/cpack/complete/postinst)
# so it configures the systemd service for YOU, not for "root" - $SUDO_USER
# alone is not reliable here: it resolves to whoever invoked the *nearest*
# sudo, so if this script itself ever runs under sudo (or anything else
# already elevated), the inner `sudo apt install` below would make the
# postinst see SUDO_USER=root instead. --preserve-env carries this one
# variable through explicitly regardless of how this script got invoked.
export OST_INSTALL_USER="$(id -un)"

DEB_FILE="ostserver-complete_latest_2604_amd64.deb"
URL="https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB_FILE}"

echo "Downloading ${DEB_FILE}..."
curl --fail --location --output "${DEB_FILE}" "${URL}"

echo "Adding the INDI library PPA (libindi1 is not in the standard Ubuntu archives)..."
sudo add-apt-repository -y ppa:mutlaqja/ppa
sudo apt update

echo "Installing ${DEB_FILE}..."
sudo --preserve-env=OST_INSTALL_USER apt install -y "./${DEB_FILE}"

# Belt and suspenders: the package's own postinst (cmake/cpack/complete/postinst)
# already creates this file if missing, but we've twice found that script
# silently not running at all (wrong maintainer-script name, then a
# packaging quirk) - make sure it exists here too rather than trust it
# blindly. Script runs as the invoking user (not elevated at the top),
# so $HOME is already the right one, no SUDO_USER juggling needed.
PARMS_FILE="${HOME}/.ost/ostserver.parms"
if [ ! -f "${PARMS_FILE}" ]; then
    echo "Creating ${PARMS_FILE}..."
    mkdir -p "${HOME}/.ost"
    cat > "${PARMS_FILE}" << 'EOF'
# ostserver startup options — one argument per line
# This file is read at service startup and will not be overwritten on upgrade.
# Remove the leading # to activate an option.

# Web server media folder (must be writable)
--webroot=/var/lib/osterix/media

# Start embedded INDI server (Y/N)
--indiserver=Y

# Log level: 0=debug, 1=info, 2=warning, 3=error, 4=critical
--loglevel=1

# Log file path (default: ostserver_YYYY-MM-DD_hh-mm-ss.log in working directory)
#--logfile=/home/user/.ost/ostserver.log

# Security level: 0=disabled, 1=read only, 2=user grants
--grant=0

# Language (fr/en/de/it/es)
#--lng=fr

# Frontend banner text (spaces are allowed, no quoting needed)
#--banner=Observatoire Sans tête

# Load a specific configuration name (default: default)
#--configuration=default

# Modules library path (default: system path)
#--libpath=

# Database path (default: working directory)
#--dbpath=

# System resource watch interval in seconds (0 to disable)
#--systemwatchinterval=10

# Minimum free disk space before skipping image write (%, 0-99, default: 10)
#--minfree=10

# Enable SSL for WebSocket server (Y/N)
#--ssl=N
#--sslcert=/etc/ost/server.crt
#--sslkey=/etc/ost/server.key

# Set admin password (applied at startup, then remove this line)
#--setadminpassword=
EOF
fi

# Documentation (the Hugo site built by the separate build-doc CI job,
# see documentation/CI/CI-build.sh) rides on the same "latest" generic
# package registry, not inside the .deb itself - the doc job runs in
# its own CI stage/image (Hugo, not the Qt6/C++ toolchain) and the
# release that publishes it only happens after every build job,
# ostserver-complete's included, so it can't be baked into the
# package at CPack time without risking a stale/mismatched tarball.
# Served for free under the existing nginx root (/var/www/pilost),
# no site config change needed - it's just a static subdirectory.
DOC_FILE="documentation_latest.tar.gz"
DOC_URL="https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DOC_FILE}"
DOC_DEST="/var/www/pilost/documentation"

echo "Downloading ${DOC_FILE}..."
curl --fail --location --output "${DOC_FILE}" "${DOC_URL}"

echo "Installing documentation to ${DOC_DEST}..."
sudo mkdir -p "${DOC_DEST}"
sudo tar -xzf "${DOC_FILE}" -C "${DOC_DEST}"

echo "Done - ostserver-complete is installed, the service should be starting."
echo "Web UI: http://<this-machine>/"
echo "Documentation: http://<this-machine>/documentation/"
