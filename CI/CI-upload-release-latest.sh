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
[ -f ostserver-complete_latest_2404_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-complete_latest_2404_amd64.deb "${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_2404_amd64.deb"
[ -f ostserver-complete_latest_2204_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-complete_latest_2204_amd64.deb "${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_2204_amd64.deb"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-complete_latest_2604_amd64.deb "${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_2604_amd64.deb"
[ -f ostserver_latest_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_fc_x86_64.rpm "${PACKAGE_REGISTRY_LATEST}/ostserver_latest_fc_x86_64.rpm"
[ -f ostserver-devel_latest_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-devel_latest_fc_x86_64.rpm "${PACKAGE_REGISTRY_LATEST}/ostserver-devel_latest_fc_x86_64.rpm"
[ -f ostserver-complete_latest_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-complete_latest_fc_x86_64.rpm "${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_fc_x86_64.rpm"
[ -f ostserver_latest_arch_x86_64.pkg.tar.zst ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_latest_arch_x86_64.pkg.tar.zst "${PACKAGE_REGISTRY_LATEST}/ostserver_latest_arch_x86_64.pkg.tar.zst"
[ -f ostserver-complete_latest_arch_x86_64.pkg.tar.zst ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-complete_latest_arch_x86_64.pkg.tar.zst "${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_arch_x86_64.pkg.tar.zst"
[ -f documentation/${DOC_FILE_LATEST} ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file documentation/${DOC_FILE_LATEST} "${PACKAGE_REGISTRY_LATEST}/${DOC_FILE_LATEST}"
# Only built on a push to main (CI/android.yml's own rules - never during
# an MR, so this is routinely absent on an MR's own release-job-latest
# run), same [ -f ... ] guard as every other optional asset above.
[ -f ${APK_FILE_LATEST} ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ${APK_FILE_LATEST} "${PACKAGE_REGISTRY_LATEST}/${APK_FILE_LATEST}"
[ -f front/${WWW_ZIP_LATEST} ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file front/${WWW_ZIP_LATEST} "${PACKAGE_REGISTRY_LATEST}/${WWW_ZIP_LATEST}"
echo "create release"
ASSETS="--assets-link {\"name\":\"ostserver_latest_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2604_amd64.deb\"}"
[ -f ostserver_latest_2404_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_latest_2404_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2404_amd64.deb\"}"
[ -f ostserver_latest_2204_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_latest_2204_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_2204_amd64.deb\"}"
ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-complete_latest_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_2604_amd64.deb\"}"
[ -f ostserver-complete_latest_2404_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-complete_latest_2404_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_2404_amd64.deb\"}"
[ -f ostserver-complete_latest_2204_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-complete_latest_2204_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_2204_amd64.deb\"}"
[ -f ostserver_latest_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_latest_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_fc_x86_64.rpm\"}"
[ -f ostserver-devel_latest_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-devel_latest_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver-devel_latest_fc_x86_64.rpm\"}"
[ -f ostserver-complete_latest_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-complete_latest_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_fc_x86_64.rpm\"}"
[ -f ostserver_latest_arch_x86_64.pkg.tar.zst ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_latest_arch_x86_64.pkg.tar.zst\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver_latest_arch_x86_64.pkg.tar.zst\"}"
[ -f ostserver-complete_latest_arch_x86_64.pkg.tar.zst ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-complete_latest_arch_x86_64.pkg.tar.zst\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/ostserver-complete_latest_arch_x86_64.pkg.tar.zst\"}"
[ -f documentation/${DOC_FILE_LATEST} ] && ASSETS="$ASSETS --assets-link {\"name\":\"${DOC_FILE_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/${DOC_FILE_LATEST}\"}"
[ -f ${APK_FILE_LATEST} ] && ASSETS="$ASSETS --assets-link {\"name\":\"${APK_FILE_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/${APK_FILE_LATEST}\"}"
[ -f front/${WWW_ZIP_LATEST} ] && ASSETS="$ASSETS --assets-link {\"name\":\"${WWW_ZIP_LATEST}\",\"url\":\"${PACKAGE_REGISTRY_LATEST}/${WWW_ZIP_LATEST}\"}"
release-cli create --name "latest" --tag-name latest $ASSETS

echo "sync debian/changelog version"
sh ./CI/CI-sync-debian-changelog.sh
