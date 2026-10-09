#!/bin/bash
#
# CI - one-shot build: gsc package for Fedora (issue #230 follow-up)
# Triggered manually (BUILD_GSC_PACKAGES=true), not part of the normal
# pipeline - see CI/gsc.yml's own header comment.
#
set -e

dnf -y install rpm-build make gcc wget

mkdir -p ~/rpmbuild/SPECS
cp "${CI_PROJECT_DIR}/gsc/fedora/gsc.spec" ~/rpmbuild/SPECS/
# %undefine _disable_source_fetch in the spec lets rpmbuild fetch
# Source0 (a plain URL) itself - no need to pre-download here.
rpmbuild -ba ~/rpmbuild/SPECS/gsc.spec

RPM=$(find ~/rpmbuild/RPMS -name "gsc-*.rpm" | head -1)
cp "${RPM}" "${CI_PROJECT_DIR}/gsc_fc_x86_64.rpm"
echo "Built: ${RPM}"

cd "${CI_PROJECT_DIR}"
echo "Uploading to the generic package registry..."
curl --fail --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" \
    --upload-file gsc_fc_x86_64.rpm \
    "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/packages/generic/gsc/1.2/gsc_fc_x86_64.rpm"
echo "Done."
