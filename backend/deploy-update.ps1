# Windows 一键更新脚本
# 用法: .\deploy-update.ps1
# 作用: 编译 Linux 二进制 -> 上传到服务器 -> 服务器自动拉取 web + 重启

param(
    [string]$Server = "root@47.112.13.246"
)

$ErrorActionPreference = "Stop"
$BackendDir = "$PSScriptRoot"

Write-Host "[1/4] 交叉编译 Linux 二进制..." -ForegroundColor Cyan
Push-Location $BackendDir
$env:GOOS       = "linux"
$env:GOARCH     = "amd64"
$env:CGO_ENABLED = "0"
$env:GOPROXY    = "https://goproxy.cn,direct"
go build -trimpath -ldflags="-s -w" -o ktester-linux ./cmd/server
if ($LASTEXITCODE -ne 0) { Write-Host "编译失败" -ForegroundColor Red; exit 1 }
$env:GOOS = ""; $env:GOARCH = ""; $env:CGO_ENABLED = ""
Write-Host "  编译成功" -ForegroundColor Green

Write-Host "[2/4] 上传二进制..." -ForegroundColor Cyan
scp ktester-linux "${Server}:/tmp/ktester-new"
if ($LASTEXITCODE -ne 0) { Write-Host "上传失败" -ForegroundColor Red; exit 1 }

Write-Host "[3/4] 上传 web 目录..." -ForegroundColor Cyan
scp -r web "${Server}:/tmp/ktester-web"
if ($LASTEXITCODE -ne 0) { Write-Host "上传 web 失败" -ForegroundColor Red; exit 1 }

Write-Host "[4/4] 服务器执行热更新..." -ForegroundColor Cyan
$remote = @'
set -e
INSTALL=/opt/ktester
BACKUP=/opt/ktester/backups

# 备份数据
mkdir -p $BACKUP
STAMP=$(date +%Y%m%d-%H%M%S)
tar -czf $BACKUP/pre-update-$STAMP.tar.gz -C $INSTALL data 2>/dev/null || true
echo "  数据已备份: $BACKUP/pre-update-$STAMP.tar.gz"

# 热换二进制
systemctl stop ktester
cp /tmp/ktester-new $INSTALL/ktester
chmod +x $INSTALL/ktester

# 更新 web
cp -r /tmp/ktester-web/. $INSTALL/web/

# 清理临时文件
rm -f /tmp/ktester-new
rm -rf /tmp/ktester-web

systemctl start ktester
sleep 1
systemctl status ktester --no-pager -l
echo "更新完成"
'@
ssh $Server $remote
Pop-Location
