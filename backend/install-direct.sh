#!/usr/bin/env bash
# 直接运行模式安装脚本 (无需 Docker)
# 在服务器上运行: bash install-direct.sh
set -euo pipefail

INSTALL_DIR="/opt/ktester"
DATA_DIR="$INSTALL_DIR/data"
BIN="$INSTALL_DIR/ktester"
WEB_DIR="$INSTALL_DIR/web"

log()  { echo -e "\033[1;34m[+]\033[0m $*"; }
warn() { echo -e "\033[1;33m[!]\033[0m $*"; }

[[ $EUID -eq 0 ]] || { echo "需要 root"; exit 1; }

log "创建目录"
mkdir -p "$DATA_DIR/layouts" "$INSTALL_DIR/web" /opt/ktester/backups

log "复制文件"
cp ktester-linux "$BIN"
chmod +x "$BIN"
cp -r web/. "$WEB_DIR/"

# ---- .env ----
if [[ ! -f "$INSTALL_DIR/.env" ]]; then
  PW=$(openssl rand -hex 12)
  cat > "$INSTALL_DIR/.env" <<EOF
LISTEN_ADDR=:3030
DATA_DIR=$DATA_DIR
JWT_SECRET=$(openssl rand -hex 32)
SUPER_ADMIN_USER=root
SUPER_ADMIN_PASS=$PW
EOF
  warn "初始超管密码: $PW  (已写入 $INSTALL_DIR/.env，请登录后立即修改)"
else
  log ".env 已存在，跳过生成"
fi

# ---- systemd service ----
cat > /etc/systemd/system/ktester.service <<EOF
[Unit]
Description=Keyboard Tester Backend
After=network.target

[Service]
Type=simple
WorkingDirectory=$INSTALL_DIR
EnvironmentFile=$INSTALL_DIR/.env
ExecStart=$BIN
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now ktester
systemctl status ktester --no-pager

log "安装完成！服务监听 :3030"
log "日志: journalctl -fu ktester"
