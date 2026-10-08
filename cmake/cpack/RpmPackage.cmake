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
# RPM dependencies are auto-detected from ELF NEEDED entries by
# rpmbuild's own find-requires, same idea as Debian's dpkg-shlibdeps -
# only add what that cannot see (a loaded-at-runtime Qt plugin,
# nginx).

# ---------------------------------------------------------------------------
# ostserver (Runtime component)
# ---------------------------------------------------------------------------
set(CPACK_RPM_RUNTIME_PACKAGE_NAME "ostserver")
set(CPACK_RPM_RUNTIME_PACKAGE_SUMMARY "Observatoire Sans Tête - headless astrophotography automation tool")
set(CPACK_RPM_RUNTIME_PACKAGE_DESCRIPTION "Light and headless astrophotography automation tool.")
set(CPACK_RPM_RUNTIME_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_LIST_DIR}/rpm/post-runtime")

# ---------------------------------------------------------------------------
# ostserver-devel (Development component)
# ---------------------------------------------------------------------------
set(CPACK_RPM_DEVELOPMENT_PACKAGE_NAME "ostserver-devel")
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
set(CPACK_RPM_COMPLETE_PACKAGE_SUMMARY "Observatoire Sans Tête - all-in-one (backend + web UI + nginx)")
set(CPACK_RPM_COMPLETE_PACKAGE_DESCRIPTION "Backend + web UI + nginx, same binaries as \"ostserver\" installed a second time - see that package if you want to bring your own web server/layout instead.")
set(CPACK_RPM_COMPLETE_PACKAGE_REQUIRES "nginx")
set(CPACK_RPM_COMPLETE_PACKAGE_CONFLICTS "ostserver")
set(CPACK_RPM_COMPLETE_PACKAGE_OBSOLETES "ostserver")
set(CPACK_RPM_COMPLETE_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_LIST_DIR}/rpm/post-complete")
set(CPACK_RPM_COMPLETE_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_LIST_DIR}/rpm/preun-complete")
