#!/bin/bash
#
# CI - Upload + Release - Latest
# 
set -e
apk add curl
echo "JOB-TOKEN: ${CI_JOB_TOKEN}"
echo "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/latest"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/latest"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ${DEB_FILE_LATEST} ${PACKAGE_REGISTRY_LATEST}/${DEB_FILE_LATEST}
release-cli create --name "latest" --tag-name latest --assets-link "{\"name\":\"${DEB_FILE_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/${DEB_FILE_LATEST}\"}" 
