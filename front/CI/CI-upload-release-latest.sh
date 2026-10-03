#!/bin/bash
#
# CI - Upload + Release - Latest (Android APK)
#
# Mirrors OST/CI/CI-upload-release-latest.sh's own pattern exactly (same
# project, consistency on purpose): delete whatever "latest" release
# already exists, upload the fresh artifact to the generic package
# registry, recreate the "latest" release pointing at it.
set -e
apk add curl
echo "delete previous latest release"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/latest" || true
echo "upload APK"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ${APK_FILE_LATEST} "${PACKAGE_REGISTRY_LATEST}/${APK_FILE_LATEST}"
echo "create release"
release-cli create --name "latest" --tag-name latest \
  --assets-link "{\"name\":\"${APK_FILE_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/${APK_FILE_LATEST}\"}"
