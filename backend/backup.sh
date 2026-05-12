#!/usr/bin/env bash
# Manual on-demand backup (the daily cron does the same).
set -euo pipefail
SRC="${SRC:-/opt/ktester/backend/data}"
DST="${DST:-/opt/ktester/backups}"
mkdir -p "$DST"
STAMP=$(date +%Y%m%d-%H%M%S)
tar -czf "$DST/ktester-manual-$STAMP.tar.gz" -C "$(dirname "$SRC")" "$(basename "$SRC")"
echo "saved: $DST/ktester-manual-$STAMP.tar.gz"
ls -lh "$DST" | tail -n 10
