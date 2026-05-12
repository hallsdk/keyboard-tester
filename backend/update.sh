#!/usr/bin/env bash
# Update ktester backend: pull latest code, take a snapshot backup,
# rebuild image, and restart the container in-place.
#
# Usage:  cd /opt/ktester/backend && ./update.sh
set -euo pipefail

cd "$(dirname "$0")/.."  # repo root
ROOT="$(pwd)"
cd "$ROOT/backend"

echo "[+] backing up data before update ..."
STAMP=$(date +%Y%m%d-%H%M%S)
mkdir -p ../../backups 2>/dev/null || true
BKDIR="/opt/ktester/backups"
mkdir -p "$BKDIR"
tar -czf "$BKDIR/pre-update-$STAMP.tar.gz" data || true
echo "    snapshot: $BKDIR/pre-update-$STAMP.tar.gz"

echo "[+] git pull"
git -C "$ROOT" fetch --all --quiet
git -C "$ROOT" reset --hard origin/main

echo "[+] rebuilding image"
docker compose build

echo "[+] restarting"
docker compose up -d

echo "[+] cleaning dangling images"
docker image prune -f >/dev/null || true

echo "[ok] update complete"
docker compose ps
