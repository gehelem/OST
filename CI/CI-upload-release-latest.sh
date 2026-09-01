#!/bin/bash
#
# CI - Upload + Release - Latest
#
set -e
apk add curl
echo "delete previous"
curl --request DELETE --header "JOB-TOKEN: ${CI_JOB_TOKEN}" "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/releases/latest" || true
echo "upload packages"
[ -f ostserver_latest_2404_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_2404_amd64.deb "${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2404_amd64.deb"
[ -f ostserver_latest_2204_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_2204_amd64.deb "${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2204_amd64.deb"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_2604_amd64.deb "${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2604_amd64.deb"
[ -f ostserver_latest_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_fc_x86_64.rpm "${PACKAGE_REGISTRY_LATEST}/ostserver_latest_fc_x86_64.rpm"
[ -f ostserver-devel_latest_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-devel_latest_fc_x86_64.rpm "${PACKAGE_REGISTRY_LATEST}/ostserver-devel_latest_fc_x86_64.rpm"
echo "create release"
ASSETS="--assets-link {\"name\":\"ostserver_latest_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2604_amd64.deb\"}"
[ -f ostserver_latest_2404_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_latest_2404_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2404_amd64.deb\"}"
[ -f ostserver_latest_2204_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_latest_2204_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2204_amd64.deb\"}"
[ -f ostserver_latest_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_latest_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_fc_x86_64.rpm\"}"
[ -f ostserver-devel_latest_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-devel_latest_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver-devel_latest_fc_x86_64.rpm\"}"
release-cli create --name "latest" --tag-name latest $ASSETS
