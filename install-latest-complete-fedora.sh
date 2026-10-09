#!/bin/bash
#
# Fedora equivalent of install-latest-complete.sh/-arch.sh (issue #230).
#
# The RPM package itself (cmake/cpack/RpmPackage.cmake + cmake/cpack/
# rpm/post-*) deliberately does NOT enable the systemd service or wire
# a live nginx config - Fedora's own packaging guidelines mandate this
# hands-off behaviour (the %systemd_post macro's own documentation),
# same choice already made for Arch. This script is the separate, more
# opinionated "just make it work" path, same split as the other two.
#
# Unlike Arch, Fedora packages `astrometry` directly (no AUR-style
# local build needed) - but its own astrometry-data-* subpackages use
# a different index series/granularity than the raw files this script
# (and the Ubuntu/Arch ones) download straight from data.astrometry.net,
# so those are not used here: same files, same whiptail checklist,
# same numbers everywhere for one consistent experience regardless of
# distro. gsc has no official Fedora package (and the one third-party
# COPR repo that briefly had one has since lost that build) - installed
# from our own prebuilt package instead (issue #230 follow-up,
# CI/gsc.yml).
#
set -e

DEB_EQUIV="ostserver-complete_latest_fc_x86_64.rpm"
URL="https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DEB_EQUIV}"

echo "Downloading ${DEB_EQUIV}..."
sudo dnf install -y wget >/dev/null 2>&1 || true
wget -q "${URL}" -O "${DEB_EQUIV}" || curl --fail --location --output "${DEB_EQUIV}" "${URL}"

echo "Installing ${DEB_EQUIV}..."
# libindi explicitly alongside the package itself: the RPM's own
# auto-detected Requires doesn't reliably pull it in (same reason
# CI-test-rpm.sh has always listed it separately too, on top of the
# package it's testing - confirmed missing here the same way).
sudo dnf install -y "./${DEB_EQUIV}" libindi

echo "Enabling and starting ostserver..."
sudo systemctl daemon-reload
sudo systemctl enable --now ostserver.service

echo "Wiring the real nginx config (the package only ships an example)..."
sudo dnf install -y nginx

# Fedora's stock nginx.conf has its own default "server_name _;" block
# built directly INTO the main file (not a separate conf.d file like
# Debian's sites-enabled/default) - confirmed the hard way: pilost.conf
# loses the "conflicting server name" fight every time (conf.d files
# load after nginx.conf's own inline block, and nginx just keeps
# whichever one it saw first). nginx.conf is %config(noreplace) in
# Fedora's own packaging - editing it is the expected, supported way
# to customize it, not something we're reaching past our lane for.
# Brace-counting awk instead of a fixed line range: robust regardless
# of exact formatting, and a no-op if already removed (idempotent).
sudo awk '
/^[[:space:]]*server[[:space:]]*\{/ && !done {
    depth = 1; skip = 1; next
}
skip {
    if ($0 ~ /\{/) depth++
    if ($0 ~ /\}/) depth--
    if (depth == 0) { skip = 0; done = 1 }
    next
}
{ print }
' /etc/nginx/nginx.conf > /tmp/nginx.conf.new
sudo mv /tmp/nginx.conf.new /etc/nginx/nginx.conf
sudo chown root:root /etc/nginx/nginx.conf
sudo chmod 644 /etc/nginx/nginx.conf
# SELinux (enforcing by default on Fedora, unlike Ubuntu/Arch) ties
# the mv'd-in file to /tmp's own context, not /etc/nginx's expected
# one - confirmed the hard way: "open() nginx.conf failed (13:
# Permission denied)" despite correct plain Unix ownership/mode.
# restorecon re-applies the context the system's policy expects for
# this exact path.
sudo restorecon -v /etc/nginx/nginx.conf

sudo cp /usr/share/doc/ostserver-complete/pilost.conf.example /etc/nginx/conf.d/pilost.conf
# Belt and suspenders - plain cp into an existing directory usually
# does pick up the right context on its own (unlike mv), but not worth
# a fourth test cycle to find out the hard way if it doesn't.
sudo restorecon -v /etc/nginx/conf.d/pilost.conf
sudo systemctl enable --now nginx.service
sudo systemctl reload nginx.service

# firewalld is on by default on Fedora, unlike Ubuntu/Arch - confirmed
# the hard way: the web UI and WebSocket are both unreachable from
# another machine until these ports are opened.
echo "Opening the web UI (80) and WebSocket (9624) ports in firewalld..."
sudo firewall-cmd --add-port=80/tcp --add-port=9624/tcp --permanent
sudo firewall-cmd --reload

# ostserver.parms is created by the package's own %post scriptlet
# (same shared cmake/cpack/ostserver.parms.skel as the other two
# packaging backends) if missing - nothing more to do here.

# Documentation - same tarball, same destination, same reasoning as
# the Ubuntu/Arch scripts (built by a separate CI job, not baked into
# the package at build time).
DOC_FILE="documentation_latest.tar.gz"
DOC_URL="https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/latest/latest/${DOC_FILE}"
DOC_DEST="/var/www/pilost/documentation"

echo "Downloading ${DOC_FILE}..."
wget -q "${DOC_URL}" -O "${DOC_FILE}"

echo "Installing documentation to ${DOC_DEST}..."
sudo mkdir -p "${DOC_DEST}"
sudo tar -xzf "${DOC_FILE}" -C "${DOC_DEST}"

echo "Installing astrometry.net and whiptail..."
sudo dnf install -y astrometry newt

# Baseline plate-solving indexes, same default set and reasoning as
# the Ubuntu/Arch scripts - straight from data.astrometry.net, not
# Fedora's own astrometry-data-* subpackages (different series/
# granularity, would mean a different set of choices on this page
# alone).
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

# gsc (optional - only feeds INDI's CCD/Guide simulator with a
# realistic star field, irrelevant for real hardware): no official
# package, and the one third-party COPR repo that briefly had a
# fedora-44 build has since lost it (see
# notes/NOTES-ci-packaging-arch-fedora.md). Our own prebuilt package
# instead, same upstream source, built with the flags that actually
# work.
echo "Installing gsc..."
GSC_RPM="gsc_fc_x86_64.rpm"
wget -q "https://gitlab.ostserver.fr/api/v4/projects/1/packages/generic/gsc/1.2/${GSC_RPM}" -O "${GSC_RPM}"
sudo dnf install -y "./${GSC_RPM}"

echo "Done - ostserver-complete is installed and running."
sudo dnf install -y hostname >/dev/null 2>&1 || true
THIS_MACHINE="$(hostname -I 2>/dev/null | awk '{print $1}')"
THIS_MACHINE="${THIS_MACHINE:-$(hostname)}"
echo "Web UI: http://${THIS_MACHINE}/"
echo "Documentation: http://${THIS_MACHINE}/documentation/"
