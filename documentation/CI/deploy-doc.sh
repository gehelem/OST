#!/bin/bash
#
# Deploy latest OST documentation to /var/www/documentation
# To be run as a cron job, e.g. every hour:
#   0 * * * * /path/to/deploy-doc.sh >> /var/log/deploy-doc.log 2>&1
#
set -e

GITLAB_URL="https://gitlab.ostserver.fr"
PROJECT_ID="1"  # OST (backend repo, doc now built from documentation/ there)
PACKAGE_URL="${GITLAB_URL}/api/v4/projects/${PROJECT_ID}/packages/generic/latest/latest/documentation_latest.tar.gz"
DEPLOY_DIR="/var/www/documentation"
TMP_FILE=$(mktemp /tmp/documentation_latest.XXXXXX.tar.gz)

echo "[$(date)] Downloading documentation..."
curl -fsSL -o "${TMP_FILE}" "${PACKAGE_URL}"

echo "[$(date)] Deploying to ${DEPLOY_DIR}..."
mkdir -p "${DEPLOY_DIR}"
tar -xzf "${TMP_FILE}" -C "${DEPLOY_DIR}"

rm -f "${TMP_FILE}"
echo "[$(date)] Done."
