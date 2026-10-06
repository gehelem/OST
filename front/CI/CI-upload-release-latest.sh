#!/bin/bash
#
# CI - Upload + Release - Latest (Android APK + web www/ zip)
#
# Mirrors OST/CI/CI-upload-release-latest.sh's own pattern exactly (same
# project, consistency on purpose): delete whatever "latest" release
# already exists, upload the fresh artifact to the generic package
# registry, recreate the "latest" release pointing at it.
set -e
apk add curl
echo "delete previous latest release"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/latest" || true
# Deleting the release alone does NOT delete the underlying git tag it
# points to - "latest" stayed pinned to whatever commit first created it
# (2026-10-03), never advancing on later pipeline runs even though the
# release object itself got recreated each time (caught live, Gilles,
# 2026-10-06: "il ne remplace pas la release precedente" - confirmed via
# the GitLab API: release.commit.id was still the very first pipeline's
# commit days later). release-cli create has no way to MOVE an existing
# tag, only to create one that doesn't exist yet (its own --ref is only
# honored in that case) - the tag itself has to go too.
echo "delete previous latest tag"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/repository/tags/latest" || true
echo "upload APK"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ${APK_FILE_LATEST} "${PACKAGE_REGISTRY_LATEST}/${APK_FILE_LATEST}"
echo "upload web zip"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ${WWW_ZIP_LATEST} "${PACKAGE_REGISTRY_LATEST}/${WWW_ZIP_LATEST}"
echo "create release"
release-cli create --name "latest" --tag-name latest --ref "$CI_COMMIT_SHA" \
  --assets-link "{\"name\":\"${APK_FILE_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/${APK_FILE_LATEST}\"}" \
  --assets-link "{\"name\":\"${WWW_ZIP_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/${WWW_ZIP_LATEST}\"}"
