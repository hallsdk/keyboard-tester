# Keyboard Tester · Backend

Go + SQLite + Docker. Hosts a JSON/REST API and a web admin console for
managing keyboard layouts (VID/PID → layout file) and user accounts.

```
┌─────────────────────────────────────────────┐
│  /api/auth/*    register, login, me, pwd    │
│  /api/devices/* list/get/upload (admin+)    │
│  /api/users/*   list (admin+), CRUD (super) │
│  /static/*  +  /index.html  (SPA)           │
└─────────────────────────────────────────────┘
```

## 角色 (Roles)

| Role            | 权限                                                 |
|-----------------|------------------------------------------------------|
| `user`          | 看设备列表 + 下载配列（Qt 客户端默认就是它）         |
| `admin`         | 上述 + 新增/编辑/删除设备和配列                       |
| `super_admin`   | 上述 + 管理所有用户。**对其他 admin 不可见**          |

`super_admin` 通过 `.env` 中 `SUPER_ADMIN_USER` / `SUPER_ADMIN_PASS` 引导。
管理员用 `/api/users` 看到的列表自动隐藏所有超级管理员。

## 一键部署

```bash
# 在服务器上以 root 执行：
curl -fsSL https://raw.githubusercontent.com/hallsdk/keyboard-tester/main/backend/deploy.sh | bash
```

脚本会：
1. 安装 docker + compose
2. clone 仓库到 `/opt/ktester`
3. 生成 `.env`（随机 JWT 密钥 + 随机超管密码，写在屏幕和 .env 里）
4. `docker compose up -d --build`
5. 安装 `/etc/cron.daily/ktester-backup`，每天打包 `data/` 到 `/opt/ktester/backups/`，保留 14 天

之后再配置 Nginx：

```bash
cp /opt/ktester/backend/nginx.ktester.example.conf /etc/nginx/sites-available/ktester
ln -s /etc/nginx/sites-available/ktester /etc/nginx/sites-enabled/ktester
nginx -t && systemctl reload nginx
certbot --nginx -d ktester.hallsdk.com
```

## 更新

```bash
cd /opt/ktester/backend
./update.sh
```

`update.sh` 自动 git pull → 打 `pre-update-*` 快照 → 重建镜像 → 滚动重启。

## 备份 / 恢复

- **每日自动**：`/etc/cron.daily/ktester-backup` → `/opt/ktester/backups/ktester-YYYYMMDD-HHMMSS.tar.gz`
- **手动**：`./backup.sh`
- **恢复**：
  ```bash
  docker compose down
  tar -xzf /opt/ktester/backups/ktester-20260512-031500.tar.gz -C /opt/ktester/backend/
  docker compose up -d
  ```

强烈建议在另一台机器或 OSS 上再同步一份：

```bash
# /etc/cron.daily/ktester-offsite  (示例: rsync 到另一台主机)
rsync -aq --delete /opt/ktester/backups/ user@backup-host:/srv/ktester-backups/
```

## API 速查

所有 `/api/*`（除 `/auth/register` 与 `/auth/login` 外）都需要
`Authorization: Bearer <token>` 头。

### 认证
- `POST /api/auth/register`  `{username, password}` → `{token, role}`（角色固定 user）
- `POST /api/auth/login`     `{username, password}` → `{token, role}`
- `GET  /api/auth/me`        → `{id, username, role}`
- `POST /api/auth/change-password` `{old_password, new_password}`

### 设备
- `GET  /api/devices`
- `GET  /api/devices/{vid}-{pid}`           (vid/pid 形如 `0x36F9-0xAB05`)
- `GET  /api/devices/{vid}-{pid}/layout`    (返回配列原文件; Qt 客户端用)
- `POST /api/devices`      `{vid, pid, name, description, layout_file, layout_body}` (admin+)
- `PUT  /api/devices/{id}` 部分字段同上 (admin+)
- `PUT  /api/devices/{id}/layout?filename=xxx.c`  raw body = 文件内容 (admin+)
- `DEL  /api/devices/{id}` (admin+)

### 用户
- `GET  /api/users` (admin+；admin 视角隐藏 super_admin)
- `POST /api/users` `{username, password, role}` (super_admin)
- `PUT  /api/users/{id}` `{role?, password?}` (super_admin)
- `DEL  /api/users/{id}` (super_admin)

## 本地开发

```bash
cd backend
cp .env.example .env       # 改一改
go run ./cmd/server        # 监听 :3030
# 打开 http://localhost:3030
```
