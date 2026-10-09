#!/bin/bash
#
# Arch Linux equivalent of install-latest-complete.sh (issue #230).
#
# The AUR package itself (arch/PKGBUILD + arch/ostserver.install /
# arch/ostserver-complete.install) deliberately does NOT enable the
# systemd service or wire a live nginx config - that is Arch's own
# "hands-off" packaging convention, not something to fight. This
# script is the separate, opinionated "just make it work" path: same
# split as the Debian side (conservative .deb postinst + this script
# doing the rest) - it is allowed to be more heavy-handed about it.
#
# Unlike Debian, Arch's official repos already carry libindi (in
# [extra]) - no PPA-equivalent needed for that. astrometry.net is
# AUR-only, built locally with makepkg; gsc comes from our own
# prebuilt package instead (see below - the AUR "gsc" package itself
# doesn't compile under modern GCC, issue #230 follow-up).
#
set -e

# Note: the password may be asked for repeatedly during this script.
# A sudo credential-cache keepalive was tried here and removed again -
# it doesn't reliably cover makepkg's own internal sudo calls (pacman
# -U/-S) during the astrometry.net/gsc AUR builds further down, so it
# wasn't actually fixing anything, just adding a background process
# for no benefit.

DEB_EQUIV="ostserver-complete_latest_arch_x86_64.pkg.tar.zst"
URL="https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB_EQUIV}"

echo "Downloading ${DEB_EQUIV}..."
curl --fail --location --output "${DEB_EQUIV}" "${URL}"

echo "Installing ${DEB_EQUIV}..."
sudo pacman -Syu --needed --noconfirm
sudo pacman -U --noconfirm "./${DEB_EQUIV}"

echo "Enabling and starting ostserver..."
sudo systemctl daemon-reload
sudo systemctl enable --now ostserver.service

echo "Wiring the real nginx config (the package only ships an example)..."
sudo pacman -S --needed --noconfirm nginx
NGINX_CONF=/etc/nginx/nginx.conf
if ! grep -q 'include /etc/nginx/conf\.d/\*\.conf;' "${NGINX_CONF}" 2>/dev/null; then
    sudo sed -i '/^http {/a\    include /etc/nginx/conf.d/*.conf;' "${NGINX_CONF}"
fi
sudo mkdir -p /etc/nginx/conf.d
sudo cp /usr/share/doc/ostserver-complete/pilost.conf.example /etc/nginx/conf.d/pilost.conf
sudo systemctl enable --now nginx.service
sudo systemctl reload nginx.service

# ostserver.parms is created by the package's own .install hook
# (same shared cmake/cpack/ostserver.parms.skel as the Debian side) if
# missing - nothing more to do here.

# Documentation - same tarball, same destination, same reasoning as
# install-latest-complete.sh (built by a separate CI job, not baked
# into the package at build time).
DOC_FILE="documentation_latest.tar.gz"
DOC_URL="https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DOC_FILE}"
DOC_DEST="/var/www/pilost/documentation"

echo "Downloading ${DOC_FILE}..."
curl --fail --location --output "${DOC_FILE}" "${DOC_URL}"

echo "Installing documentation to ${DOC_DEST}..."
sudo mkdir -p "${DOC_DEST}"
sudo tar -xzf "${DOC_FILE}" -C "${DOC_DEST}"

# Build an AUR package locally with makepkg - must run as a regular
# user (makepkg refuses root), prompts sudo itself via `-s` for any
# missing pacman dependency and for the final install.
_install_aur() {
    local pkg="$1" dir
    dir=$(mktemp -d)
    git clone "https://aur.archlinux.org/${pkg}.git" "${dir}"
    (cd "${dir}" && makepkg -si --noconfirm)
    rm -rf "${dir}"
}

echo "Installing astrometry.net (AUR) and whiptail..."
sudo pacman -S --needed --noconfirm libnewt base-devel git
_install_aur astrometry.net

# gsc (optional - only feeds INDI's CCD/Guide simulator with a
# realistic star field, irrelevant for real hardware): AUR's own
# package is 1990s K&R-style C that fails under modern GCC's defaults
# - a problem in that upstream package, not OST (see
# notes/NOTES-ci-packaging-arch-fedora.md). Installed from our own
# prebuilt package instead, same upstream source, built with the
# flags that actually work (issue #230 follow-up, CI/gsc.yml).
GSC_PKG="gsc_arch_x86_64.pkg.tar.zst"
curl --fail --location --output "${GSC_PKG}" \
    "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/gsc/1.2/${GSC_PKG}"
sudo pacman -U --noconfirm "./${GSC_PKG}"
# gsc's own .install hook wires GSCDAT via a systemd drop-in, but
# ostserver is already running by this point in the script (started
# further up) - restart it so gsc actually works without a manual step.
sudo systemctl restart ostserver.service

# wget isn't part of a minimal Arch install either (same gap as
# hostname/inetutils above) - needed from here on for the index files.
sudo pacman -S --needed --noconfirm wget

# Baseline plate-solving indexes, same default set and reasoning as
# install-latest-complete.sh.
echo "Downloading default astrometry.net indexes (wide field >5°, ~152 MB)..."
sudo mkdir -p /usr/share/astrometry
for idx in 4208 4209 4210 4211 4212 4213 4214; do
    sudo wget -q -P /usr/share/astrometry "http://data.astrometry.net/4200/index-${idx}.fits"
done

# Narrow-field indexes (4200-4207) - same whiptail checklist, same
# real sizes, nothing pre-selected. See install-latest-complete.sh for
# the full derivation of these numbers.
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
        case "${idx}" in
            4200|4201|4202|4203|4204) last=47 ;;
            *)                        last=11 ;;
        esac
        echo "Downloading index-${idx} (tiles 00-${last})..."
        for i in $(seq -w 0 "${last}"); do
            sudo wget -q -P /usr/share/astrometry "http://data.astrometry.net/4200/index-${idx}-${i}.fits"
        done
    done
else
    echo "No additional index selected."
fi

echo "Done - ostserver-complete is installed and running."
# hostname/hostname -I are not part of a minimal Arch install - they
# come from inetutils (confirmed: "hostname: commande introuvable" on
# a fresh VM), unlike Ubuntu/Debian where it's already present.
sudo pacman -S --needed --noconfirm inetutils
THIS_MACHINE="$(hostname -I 2>/dev/null | awk '{print $1}')"
THIS_MACHINE="${THIS_MACHINE:-$(hostname)}"
echo "Web UI: http://${THIS_MACHINE}/"
echo "Documentation: http://${THIS_MACHINE}/documentation/"
