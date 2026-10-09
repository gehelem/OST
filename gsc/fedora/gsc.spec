%undefine _disable_source_fetch
# Skip debuginfo extraction entirely - rpmbuild's automatic
# debuginfo subpackage generation produced an empty debugsourcefiles
# list for this 1990s C tool and failed the whole build with it,
# confirmed from the actual build log, not something this package has
# any use for anyway. (Comment deliberately avoids writing the literal
# RPM section-tag spelling here - a past commit's comment naming a
# real macro verbatim got expanded for real and broke a different
# build the same way.)
%global debug_package %{nil}
# Separate, optional package (issue #230 follow-up) - feeds INDI's
# CCD/Guide simulator driver with a realistic star field from the
# real Guide Star Catalog I (STScI). Never needed for real hardware.
#
# Our own build, not the (nonexistent) official/COPR packages - see
# gsc/arch/PKGBUILD's own comment for the -std=gnu17 rationale (1990s
# K&R-style C, the AUR package it mirrors fails the same way on
# Fedora's equally modern GCC).
#
# The upstream tarball has no top-level wrapping directory (N*/, S*/,
# bin/, src/ straight at the root) - %setup -c creates %{name}-
# %{version} first and extracts into it, instead of expecting the
# tarball to already contain that directory.

Name:           gsc
Version:        1.2
Release:        1%{?dist}
Summary:        Guide Star Catalog I (STScI) query tool + data for INDI simulator drivers

License:        Custom
URL:            http://gsss.stsci.edu/Catalogs/GSC/GSC1/GSC1.htm
Source0:        http://cdsarc.u-strasbg.fr/viz-bin/nph-Cat/tar.gz?bincats/GSC_%{version}
BuildArch:      x86_64

%description
Guide Star Catalog I (STScI) query tool and data, packaged as a
standalone add-on for OST - feeds INDI's CCD/Guide simulator driver
with a realistic star field. Not needed for real hardware.

%prep
%setup -q -c

%build
cd src
# See gsc/arch/PKGBUILD's own comment for the full rationale - same
# fix on both. -I. because gsc.c et al use #include <gsc.h> (angle
# brackets); the -Wno-error=* flags because modern GCC defaults
# several legacy-C warning categories to hard errors regardless of
# -std=gnu17 (confirmed one at a time: implicit-function-declaration,
# implicit-int; builtin-declaration-mismatch/int-conversion/
# incompatible-pointer-types added defensively, not yet individually
# confirmed necessary).
make CC="gcc -I. -std=gnu17 \
    -Wno-error=implicit-function-declaration \
    -Wno-error=implicit-int \
    -Wno-error=builtin-declaration-mismatch \
    -Wno-error=int-conversion \
    -Wno-error=incompatible-pointer-types"

%install
for f in $(find [NS]* -type f); do
    install -D -m644 "${f}" "%{buildroot}/usr/share/GSC/${f}"
done
install -D -m755 src/gsc.exe %{buildroot}/usr/share/GSC/bin/gsc
install -D -m644 bin/regions.bin %{buildroot}/usr/share/GSC/bin/regions.bin
install -D -m644 bin/regions.ind %{buildroot}/usr/share/GSC/bin/regions.ind
mkdir -p %{buildroot}/usr/bin
ln -s /usr/share/GSC/bin/gsc %{buildroot}/usr/bin/gsc

# gsc itself reads the GSCDAT environment variable to find its own
# catalog data (/usr/share/GSC, confirmed from indilib/indi's own
# sky_renderer.cpp: it just calls popen() on "gsc -c ..." with no path
# argument at all). ostserver spawns indiserver, which spawns
# indi_simulator_ccd/_guide, which does that popen() - the variable
# has to be set somewhere in that chain. A systemd drop-in for
# ostserver.service rather than editing that shared file directly -
# it is used by every packaging backend, not just when gsc happens to
# be installed too.
%post
mkdir -p /etc/systemd/system/ostserver.service.d
cat > /etc/systemd/system/ostserver.service.d/gsc.conf << 'EOF'
[Service]
Environment=GSCDAT=/usr/share/GSC
EOF
systemctl daemon-reload 2>/dev/null || true
echo "==> gsc installed. If ostserver is already running, restart it"
echo "    to pick up GSCDAT:"
echo "        sudo systemctl restart ostserver.service"

%postun
if [ "$1" -eq 0 ]; then
    rm -f /etc/systemd/system/ostserver.service.d/gsc.conf
    systemctl daemon-reload 2>/dev/null || true
fi

%files
/usr/share/GSC/
/usr/bin/gsc

%changelog
* Thu Oct 09 2026 Gilles <gilles@joag.fr> - 1.2-1
- Initial package (issue #230 follow-up)
