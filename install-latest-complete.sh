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
# mandatory/always-built job). Adjust the amd64 filename below if you
# need the 24.04 build instead (ostserver-complete_latest_2404_amd64.deb).
#
# On arm64, no pre-built .deb is published to the GitLab registry yet,
# so this falls back to the ppa:gehelem/ostserver PPA instead, which
# builds ostserver-complete from source on Launchpad's own builders for
# both amd64 and arm64 (confirmed 2026-10, resolute series).
set -e

# Passed through to the package's postinst (cmake/cpack/complete/postinst)
# so it configures the systemd service for YOU, not for "root" - $SUDO_USER
# alone is not reliable here: it resolves to whoever invoked the *nearest*
# sudo, so if this script itself ever runs under sudo (or anything else
# already elevated), the inner `sudo apt install` below would make the
# postinst see SUDO_USER=root instead. --preserve-env carries this one
# variable through explicitly regardless of how this script got invoked.
export OST_INSTALL_USER="$(id -un)"

ARCH="$(dpkg --print-architecture)"

echo "Adding the INDI library PPA (libindi1 is not in the standard Ubuntu archives)..."
sudo add-apt-repository -y ppa:mutlaqja/ppa
# universe carries nginx, the Qt6/opencv/avahi/gsl/cfitsio/wcs libs and
# astrometry.net itself - not guaranteed enabled on a minimal/cloud
# image (confirmed missing on a fresh VM, 2026-10).
sudo add-apt-repository -y universe

if [ "${ARCH}" = "arm64" ]; then
    echo "arm64 detected: no GitLab-built package for this architecture yet, using the Launchpad PPA instead..."
    sudo add-apt-repository -y ppa:gehelem/ostserver
    sudo apt update
    echo "Installing ostserver-complete..."
    sudo --preserve-env=OST_INSTALL_USER apt install -y ostserver-complete
else
    DEB_FILE="ostserver-complete_latest_2604_amd64.deb"
    URL="https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB_FILE}"

    echo "Downloading ${DEB_FILE}..."
    curl --fail --location --output "${DEB_FILE}" "${URL}"

    sudo apt update

    echo "Installing ${DEB_FILE}..."
    sudo --preserve-env=OST_INSTALL_USER apt install -y "./${DEB_FILE}"
fi
# ostserver.parms is created by the package's own postinst
# (cmake/cpack/complete/postinst, cp'd from the shared
# share/ostserver/ostserver.parms.skel) if missing - no need to
# duplicate that here now that the real bug keeping postinst from
# running at all (wrong maintainer-script name) is fixed.

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

echo "Installing astrometry.net..."
sudo apt install -y astrometry.net whiptail

# Baseline plate-solving indexes (Navigator/Polar modules), same set as
# documentation/content/install/fresh.en.md §3's default "wide field >5°"
# block - downloaded unconditionally, no prompt. ~152 MB total.
echo "Downloading default astrometry.net indexes (wide field >5°, ~152 MB)..."
for idx in 4208 4209 4210 4211 4212 4213 4214; do
    # -nc: skip a file already present (re-running this script shouldn't
    # re-download ~150 MB of unchanged index files every time).
    sudo wget -nc -q -P /usr/share/astrometry "http://data.astrometry.net/4200/index-${idx}.fits"
done

# Narrow-field indexes (4200-4207, long focal length / small sensor) are
# NOT downloaded by default - sizes range from ~158 MB (4207) up to
# ~13 GB (4200), real sizes confirmed against data.astrometry.net, 2026-10.
# whiptail (ships with Ubuntu Server, used by debconf) gives the same
# [*]/[ ] checklist as the Ubuntu/Debian installers - nothing fancy
# needed to get that UX.
CHOICES=$(whiptail --title "Additional astrometry.net indexes" \
    --checklist "Narrow-field plate-solving indexes (long focal length / small sensor).\nNothing is pre-selected - these can be several GB each.\nSpace to toggle, Tab to move, Enter to confirm." \
    20 78 8 \
    "4207" "22'-30'    ~158 MB  (12 files)" OFF \
    "4206" "16'-22'    ~313 MB  (12 files)" OFF \
    "4205" "11'-16'    ~629 MB  (12 files)" OFF \
    "4204" " 8'-11'    ~1.2 GB  (48 files)" OFF \
    "4203" "5.6'-8'    ~2.5 GB  (48 files)" OFF \
    "4202" "4'-5.6'    ~4.7 GB  (48 files)" OFF \
    "4201" "2.8'-4'    ~8.6 GB  (48 files)" OFF \
    "4200" " 2'-2.8'   ~13.2 GB (48 files)" OFF \
    3>&1 1>&2 2>&3) || true

if [ -n "${CHOICES}" ]; then
    eval "SELECTED_INDEXES=(${CHOICES})"
    for idx in "${SELECTED_INDEXES[@]}"; do
        # 4200-4204: 48 healpix tiles. 4205-4207: 12 tiles (see
        # data.astrometry.net/4200/README). seq -w pads to match
        # the width of the last number in both cases (00-11, 00-47).
        case "${idx}" in
            4200|4201|4202|4203|4204) last=47 ;;
            *)                        last=11 ;;
        esac
        echo "Downloading index-${idx} (tiles 00-${last})..."
        for i in $(seq -w 0 "${last}"); do
            sudo wget -nc -q -P /usr/share/astrometry "http://data.astrometry.net/4200/index-${idx}-${i}.fits"
        done
    done
else
    echo "No additional index selected."
fi

echo "Done - ostserver-complete is installed, the service should be starting."
# hostname -I lists every address on every interface (first one is
# usually the main LAN IP, which is what matters here - this is
# accessed from another machine on the local network, a .local/mDNS
# name is not guaranteed to resolve on every client).
THIS_MACHINE="$(hostname -I 2>/dev/null | awk '{print $1}')"
THIS_MACHINE="${THIS_MACHINE:-$(hostname)}"
echo "Web UI: http://${THIS_MACHINE}/"
echo "Documentation: http://${THIS_MACHINE}/documentation/"
