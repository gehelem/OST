#!/bin/bash
#
# CI - Upload + Release - TAG
#
set -e
apk add curl
echo "upload packages"
[ -f ostserver_${CI_COMMIT_TAG}_2404_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_2404_amd64.deb "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2404_amd64.deb"
[ -f ostserver_${CI_COMMIT_TAG}_2204_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_2204_amd64.deb "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2204_amd64.deb"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_2604_amd64.deb "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2604_amd64.deb"
[ -f ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm"
[ -f ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm "${PACKAGE_REGISTRY_TAG}/ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm"
echo "create release"
ASSETS="--assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2604_amd64.deb\"}"
[ -f ostserver_${CI_COMMIT_TAG}_2404_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_2404_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2404_amd64.deb\"}"
[ -f ostserver_${CI_COMMIT_TAG}_2204_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_2204_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2204_amd64.deb\"}"
[ -f ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm\"}"
[ -f ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm\"}"
release-cli create --name "Release $CI_COMMIT_TAG" --tag-name $CI_COMMIT_TAG $ASSETS
