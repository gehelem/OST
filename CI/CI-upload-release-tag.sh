#!/bin/bash
#
# CI - Upload + Release - TAG
#
set -e
apk add curl jq
echo "upload packages"
[ -f ostserver_${CI_COMMIT_TAG}_2404_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_2404_amd64.deb "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2404_amd64.deb"
[ -f ostserver_${CI_COMMIT_TAG}_2204_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_2204_amd64.deb "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2204_amd64.deb"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_2604_amd64.deb "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2604_amd64.deb"
[ -f ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm"
[ -f ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm "${PACKAGE_REGISTRY_TAG}/ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm"
[ -f documentation/documentation_${CI_COMMIT_TAG}.tar.gz ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file documentation/documentation_${CI_COMMIT_TAG}.tar.gz "${PACKAGE_REGISTRY_TAG}/documentation_${CI_COMMIT_TAG}.tar.gz"
echo "create release"
ASSETS="--assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2604_amd64.deb\"}"
[ -f ostserver_${CI_COMMIT_TAG}_2404_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_2404_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2404_amd64.deb\"}"
[ -f ostserver_${CI_COMMIT_TAG}_2204_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_2204_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2204_amd64.deb\"}"
[ -f ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_fc_x86_64.rpm\"}"
[ -f ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver-devel_${CI_COMMIT_TAG}_fc_x86_64.rpm\"}"
[ -f documentation/documentation_${CI_COMMIT_TAG}.tar.gz ] && ASSETS="$ASSETS --assets-link {\"name\":\"documentation_${CI_COMMIT_TAG}.tar.gz\",\"url\":\"${PACKAGE_REGISTRY_TAG}/documentation_${CI_COMMIT_TAG}.tar.gz\"}"

echo "generate changelog"
# GitLab auto-detects the range against the previous semver tag when only
# "version" is given. Only commits carrying a "Changelog:" trailer show up.
# Never let this break the release: on any failure, fall through with no
# description, same as before this was added.
CHANGELOG_NOTES=""
if curl -fsS --header "JOB-TOKEN: ${CI_JOB_TOKEN}" \
    --get --data-urlencode "version=${CI_COMMIT_TAG}" \
    "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/repository/changelog" \
    -o /tmp/changelog_response.json
then
    CHANGELOG_NOTES=$(jq -r '.notes // empty' /tmp/changelog_response.json 2>/dev/null || true)
fi

echo "create release"
if [ -n "$CHANGELOG_NOTES" ]; then
    release-cli create --name "Release $CI_COMMIT_TAG" --tag-name $CI_COMMIT_TAG --description "$CHANGELOG_NOTES" $ASSETS
else
    release-cli create --name "Release $CI_COMMIT_TAG" --tag-name $CI_COMMIT_TAG $ASSETS
fi

echo "commit changelog to CHANGELOG.md"
# Same data as above, but this variant (POST) commits a Markdown section
# to CHANGELOG.md on the default branch server-side - no git push from
# the runner involved. Lands as a commit *after* the tag, on main.
# Non-fatal: a failure here must never be mistaken for a failed release,
# the release itself is already created above.
curl -fsS --header "JOB-TOKEN: ${CI_JOB_TOKEN}" \
    --request POST \
    --data-urlencode "version=${CI_COMMIT_TAG}" \
    "${CI_API_V4_URL}/projects/${CI_PROJECT_ID}/repository/changelog" \
    || echo "changelog commit failed (non-fatal, release already created)"
