#!/bin/bash
#
# CI - Upload + Release - TAG
# 
apk add curl
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ${DEB_FILE_TAG} ${PACKAGE_REGISTRY_TAG}/${DEB_FILE_TAG}
release-cli create --name "Release $CI_COMMIT_TAG" --tag-name $CI_COMMIT_TAG --assets-link "{\"name\":\"${DEB_FILE_TAG}\",\"url\":\"${PACKAGE_REGISTRY_TAG}/${DEB_FILE_TAG}\"}" 
