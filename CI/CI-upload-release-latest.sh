#!/bin/bash
#
# CI - Upload + Release - Latest
#
set -e
apk add curl
echo "delete previous"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/latest" || true
echo "upload packages"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_amd64.deb "${PACKAGE_REGISTRY_LATEST}/ostserver_latest_amd64.deb"
[ -f ostserver_latest_2604_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_2604_amd64.deb "${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2604_amd64.deb"
echo "create release"
ASSETS="--assets-link {\"name\":\"ostserver_latest_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_amd64.deb\"}"
[ -f ostserver_latest_2604_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_latest_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2604_amd64.deb\"}"
release-cli create --name "latest" --tag-name latest $ASSETS
