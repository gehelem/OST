#!/bin/bash
#
# CI - Upload + Release - Latest documentation
#
set -e
apk add curl
echo "delete previous release"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/latest"
echo "upload new"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ${DOC_FILE_LATEST} ${PACKAGE_REGISTRY_LATEST}/${DOC_FILE_LATEST}
echo "create release"
release-cli create --name "latest" --tag-name latest --assets-link "{\"name\":\"${DOC_FILE_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/${DOC_FILE_LATEST}\"}"
