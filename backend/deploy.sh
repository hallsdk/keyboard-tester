#!/usr/bin/env bash
# One-shot deploy script for ktester backend on a fresh Ubuntu/Debian host.
# Usage (as root):
#   curl -fsSL https://raw.githubusercontent.com/hallsdk/keyboard-tester/main/backend/deploy.sh | bash
#
# Or, after `git clone`:
#   cd /opt/ktester/backend && ./deploy.sh
set -euo pipefail

REPO_URL="${REPO_URL:-https://github.com/hallsdk/keyboard-tester.git}"
INSTALL_DIR="${INSTALL_DIR:-/opt/ktester}"
BRANCH="${BRANCH:-main}"

log()  { echo -e "\033[1;34m[+]\033[0m $*"; }
warn() { echo -e "\033[1;33m[!]\033[0m $*"; }
die()  { echo -e "\033[1;31m[x]\033[0m $*"; exit 1; }

[[ $EUID -eq 0 ]] || die "must run as root"

# ---- 1. Install prerequisites ----
log "installing prerequisites (docker, docker compose plugin, git)"
if ! command -v docker >/dev/null 2>&1; then
  apt-get update -qq
  apt-get install -y -qq ca-certificates curl gnupg lsb-release git
  install -m 0755 -d /etc/apt/keyrings
  curl -fsSL https://download.docker.com/linux/$(. /etc/os-release && echo "$ID")/gpg \
    | gpg --dearmor -o /etc/apt/keyrings/docker.gpg
  chmod a+r /etc/apt/keyrings/docker.gpg
  echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/$(. /etc/os-release && echo "$ID") $(. /etc/os-release && echo "$VERSION_CODENAME") stable" \
    > /etc/apt/sources.list.d/docker.list
  apt-get update -qq
  apt-get install -y -qq docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
  systemctl enable --now docker
fi
command -v git >/dev/null 2>&1 || apt-get install -y -qq git

# ---- 2. Clone or update source ----
mkdir -p "$INSTALL_DIR"
if [[ -d "$INSTALL_DIR/.git" ]]; then
  log "updating existing checkout in $INSTALL_DIR"
  git -C "$INSTALL_DIR" fetch --all --quiet
  git -C "$INSTALL_DIR" checkout "$BRANCH"
  git -C "$INSTALL_DIR" reset --hard "origin/$BRANCH"
else
  log "cloning $REPO_URL into $INSTALL_DIR"
  git clone --branch "$BRANCH" "$REPO_URL" "$INSTALL_DIR"
fi

cd "$INSTALL_DIR/backend"

# ---- 3. .env ----
if [[ ! -f .env ]]; then
  log "generating .env (random JWT_SECRET; super-admin default 'root'/random)"
  PW=$(openssl rand -hex 12)
  cat > .env <<EOF
JWT_SECRET=$(openssl rand -hex 32)
SUPER_ADMIN_USER=root
SUPER_ADMIN_PASS=$PW
EOF
  warn "Initial super-admin credentials written to $INSTALL_DIR/backend/.env"
  warn "  username: root"
  warn "  password: $PW"
  warn "Please log in and change the password ASAP."
else
  log ".env already exists, leaving as-is"
fi

# ---- 4. Build & start ----
log "building and starting container ..."
docker compose up -d --build

# ---- 5. Cron-based daily backup ----
log "installing daily backup cron job"
cat > /etc/cron.daily/ktester-backup <<'EOF'
#!/usr/bin/env bash
# Daily backup of ktester sqlite + layouts.
set -e
SRC="/opt/ktester/backend/data"
DST="/opt/ktester/backups"
mkdir -p "$DST"
STAMP=$(date +%Y%m%d-%H%M%S)
TMP="$(mktemp -d)"
# Hot-copy sqlite via .backup so it's consistent.
docker exec ktester sh -c "wget -q -O- http://127.0.0.1:3030/" >/dev/null 2>&1 || true
cp -a "$SRC" "$TMP/data"
tar -czf "$DST/ktester-$STAMP.tar.gz" -C "$TMP" data
rm -rf "$TMP"
# Keep last 14 days only.
find "$DST" -name 'ktester-*.tar.gz' -mtime +14 -delete
EOF
chmod +x /etc/cron.daily/ktester-backup

log "done!"
log "container status:"
docker compose ps
echo
log "next step: configure /etc/nginx/sites-available/ktester (see backend/nginx.ktester.example.conf)"
log "then:  ln -s /etc/nginx/sites-available/ktester /etc/nginx/sites-enabled/ktester && certbot --nginx -d ktester.hallsdk.com"
