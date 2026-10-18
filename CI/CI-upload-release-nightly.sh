#!/bin/bash
#
# CI - Upload + Release - Nightly
#
# Same delete-then-recreate pattern as CI-upload-release-latest.sh, a
# different rolling release/package ("nightly" instead of "latest") so a
# push to a feature branch never touches the real latest release. Only
# the subset of artifacts that actually build outside of main/MR/tag
# (release-job-nightly's own dependencies in .gitlab-ci.yml) is handled
# here - no 22.04/24.04/fedora/arch.
set -e
apk add curl
echo "delete previous"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/nightly" || true
echo "upload packages"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_2604_amd64.deb "${PACKAGE_REGISTRY_NIGHTLY}/ostserver_latest_2604_amd64.deb"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-complete_latest_2604_amd64.deb "${PACKAGE_REGISTRY_NIGHTLY}/ostserver-complete_latest_2604_amd64.deb"
[ -f documentation/${DOC_FILE_LATEST} ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file documentation/${DOC_FILE_LATEST} "${PACKAGE_REGISTRY_NIGHTLY}/${DOC_FILE_LATEST}"
[ -f ${APK_FILE_LATEST} ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ${APK_FILE_LATEST} "${PACKAGE_REGISTRY_NIGHTLY}/${APK_FILE_LATEST}"
[ -f front/${WWW_ZIP_LATEST} ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file front/${WWW_ZIP_LATEST} "${PACKAGE_REGISTRY_NIGHTLY}/${WWW_ZIP_LATEST}"
echo "create release"
ASSETS="--assets-link {\"name\":\"ostserver_latest_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_NIGHTLY}/ostserver_latest_2604_amd64.deb\"}"
ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-complete_latest_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_NIGHTLY}/ostserver-complete_latest_2604_amd64.deb\"}"
[ -f documentation/${DOC_FILE_LATEST} ] && ASSETS="$ASSETS --assets-link {\"name\":\"${DOC_FILE_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_NIGHTLY}/${DOC_FILE_LATEST}\"}"
[ -f ${APK_FILE_LATEST} ] && ASSETS="$ASSETS --assets-link {\"name\":\"${APK_FILE_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_NIGHTLY}/${APK_FILE_LATEST}\"}"
[ -f front/${WWW_ZIP_LATEST} ] && ASSETS="$ASSETS --assets-link {\"name\":\"${WWW_ZIP_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_NIGHTLY}/${WWW_ZIP_LATEST}\"}"
release-cli create --name "nightly" --tag-name nightly --ref "$CI_COMMIT_SHA" $ASSETS
