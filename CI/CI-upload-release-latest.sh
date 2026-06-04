#!/bin/bash
#
# CI - Upload + Release - Latest
#
set -e
apk add curl
echo "delete previous"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/latest" || true
echo "upload packages"
for DEB in ostserver_latest_2404_amd64.deb ostserver_latest_2604_amd64.deb; do
    [ -f "${DEB}" ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file "${DEB}" "${PACKAGE_REGISTRY_LATEST}/${DEB}"
done
echo "create release"
release-cli create --name "latest" --tag-name latest \
  --assets-link "{\"name\":\"ostserver_latest_2404_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2404_amd64.deb\"}" \
  --assets-link "{\"name\":\"ostserver_latest_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2604_amd64.deb\"}"
