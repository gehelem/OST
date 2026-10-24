# cmake/cpack/RpmPackage.cmake
# CPack RPM packaging (Fedora, issue #230) - mirrors DebianPackage.cmake's
# own component split (Runtime/Development/Complete), reusing the exact
# same CMake install() rules. include(CPack) itself is deferred to
# CMakeLists.txt (see DebianPackage.cmake's own trailing comment) - this
# file only sets CPACK_RPM_* variables, read only when `cpack -G RPM` is
# the generator actually invoked (CI/CI-build-rpm.sh).
#
# RPM's %post/%preun scriptlets take a numeric $1 (1=fresh install,
# 2=upgrade for %post; reversed for %preun, 0=final removal, 1=being
# upgraded) - NOT dpkg's "configure"/"abort-upgrade" strings, so the
# Debian postinst/prerm scripts (cmake/cpack/postinst, cmake/cpack/
# complete/postinst) cannot be reused verbatim here. Separate scripts
# under cmake/cpack/rpm/ instead, same underlying logic, no case "$1"
# wrapper needed (our logic is already idempotent - safe to just run
# unconditionally on both fresh install and upgrade).
#
# Does NOT enable/start the service or write a live nginx config, same
# "hands-off" choice already made for Arch (Fedora's own packaging
# guidelines actually mandate this: see the %systemd_post/%systemd_preun
# macros' own doc, services are not auto-enabled by convention) - a
# separate, more opinionated script does that part, same split as the
# Debian/Arch sides.

set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_RPM_PACKAGE_URL "https://gitlab.ostserver.fr/ost/ostserver/")
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_VERSION "${OST_UPSTREAM_VERSION}")
set(CPACK_RPM_PACKAGE_RELEASE "${OST_DEBIAN_REVISION}")
# RPM dependencies are supposed to be auto-detected from ELF NEEDED
# entries, same idea as Debian's dpkg-shlibdeps - but confirmed the
# hard way that libindi specifically does NOT get picked up this way
# for this package (CPack's auto-generated spec doesn't run the same
# full find-requires machinery a hand-written SPEC + plain rpmbuild
# would), so it needs listing explicitly here, same as nginx below for
# the Complete component. avahi is the running service our avahi-client
# linkage talks to over D-Bus to publish mDNS discovery - never picked
# up automatically either, nothing links against the daemon itself.
set(_requires_common "libindi, avahi")

# ---------------------------------------------------------------------------
# ostserver (Runtime component)
# ---------------------------------------------------------------------------
set(CPACK_RPM_RUNTIME_PACKAGE_NAME "ostserver")
# CPACK_RPM_<COMPONENT>_PACKAGE_NAME only sets the RPM's internal Name:
# field, NOT the generated .rpm file's name - that defaults to
# CPACK_PACKAGE_NAME (the global project name, "OST") + "-Linux-
# <Component>.rpm" unless FILE_NAME is set explicitly too (confirmed
# the hard way: CI-build-rpm.sh's ls globs found nothing without this).
set(CPACK_RPM_RUNTIME_FILE_NAME "ostserver.rpm")
set(CPACK_RPM_RUNTIME_PACKAGE_SUMMARY "Observatoire Sans Tête - headless astrophotography automation tool")
set(CPACK_RPM_RUNTIME_PACKAGE_DESCRIPTION "Light and headless astrophotography automation tool.")
set(CPACK_RPM_RUNTIME_PACKAGE_REQUIRES "${_requires_common}")
set(CPACK_RPM_RUNTIME_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_LIST_DIR}/rpm/post-runtime")

# ---------------------------------------------------------------------------
# ostserver-devel (Development component)
# ---------------------------------------------------------------------------
set(CPACK_RPM_DEVELOPMENT_PACKAGE_NAME "ostserver-devel")
set(CPACK_RPM_DEVELOPMENT_FILE_NAME "ostserver-devel.rpm")
set(CPACK_RPM_DEVELOPMENT_PACKAGE_SUMMARY "Observatoire Sans Tête - development headers")
set(CPACK_RPM_DEVELOPMENT_PACKAGE_DESCRIPTION "Headers for building OST modules.")
set(CPACK_RPM_DEVELOPMENT_PACKAGE_REQUIRES "ostserver = ${OST_UPSTREAM_VERSION}")

# ---------------------------------------------------------------------------
# ostserver-complete (Complete component - issue #230)
# ---------------------------------------------------------------------------
# Same reasoning as the Debian/Arch sides: ships the exact same
# binaries a second time (not a dependent of "ostserver") + front/'s
# static files + nginx wiring. Conflicts/Obsoletes "ostserver", never
# Requires - a single `dnf install ./ostserver-complete.rpm` must work
# standalone.
set(CPACK_RPM_COMPLETE_PACKAGE_NAME "ostserver-complete")
set(CPACK_RPM_COMPLETE_FILE_NAME "ostserver-complete.rpm")
set(CPACK_RPM_COMPLETE_PACKAGE_SUMMARY "Observatoire Sans Tête - all-in-one (backend + web UI + nginx)")
set(CPACK_RPM_COMPLETE_PACKAGE_DESCRIPTION "Backend + web UI + nginx, same binaries as \"ostserver\" installed a second time - see that package if you want to bring your own web server/layout instead.")
set(CPACK_RPM_COMPLETE_PACKAGE_REQUIRES "${_requires_common}, nginx")
set(CPACK_RPM_COMPLETE_PACKAGE_CONFLICTS "ostserver")
set(CPACK_RPM_COMPLETE_PACKAGE_OBSOLETES "ostserver")
set(CPACK_RPM_COMPLETE_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_LIST_DIR}/rpm/post-complete")
set(CPACK_RPM_COMPLETE_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_LIST_DIR}/rpm/preun-complete")
