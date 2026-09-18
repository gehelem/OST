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
# The repository/changelog POST endpoint (server-side commit, no git
# push needed) looked simpler but CI_JOB_TOKEN can't use it: GitLab
# deliberately restricts job tokens to GET on this endpoint, a real
# "git push" instead. Requires "Allow Git push requests to the
# repository" enabled under Settings > CI/CD > Job token permissions
# (done 2026-09-18). The job token then pushes with the triggering
# user's own permissions, and GitLab does not re-trigger a pipeline for
# a push made with a job token - no [skip ci] needed, no loop risk.
# Whole sequence isolated in a subshell: if any step fails, the release
# (already created above) is never affected.
if [ -n "$CHANGELOG_NOTES" ]; then
    (
        set -e
        git config user.email "gilles@joag.fr"
        git config user.name "gilles"
        git fetch origin main
        git checkout main
        if [ -f CHANGELOG.md ]; then
            { echo "$CHANGELOG_NOTES"; echo; cat CHANGELOG.md; } > CHANGELOG.md.new
            mv CHANGELOG.md.new CHANGELOG.md
        else
            echo "$CHANGELOG_NOTES" > CHANGELOG.md
        fi
        git add CHANGELOG.md
        git commit -m "Add changelog for version ${CI_COMMIT_TAG}"
        git push origin main
    ) || echo "changelog commit/push failed (non-fatal, release already created)"
else
    echo "no changelog notes, skipping CHANGELOG.md commit"
fi
