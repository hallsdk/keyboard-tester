# Windows one-click update script
# Usage: .\deploy-update.ps1
# First run: set up SSH key so no password is needed:
#   ssh-keygen -t ed25519 -f $env:USERPROFILE\.ssh\id_ktester -N ""
#   type $env:USERPROFILE\.ssh\id_ktester.pub | ssh root@47.112.13.246 "mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys"

param(
    [string]$Server = "root@47.112.13.246",
    [string]$KeyFile = "$env:USERPROFILE\.ssh\id_ktester"
)

$ErrorActionPreference = "Stop"
$BackendDir = $PSScriptRoot
$SshOpts = if (Test-Path $KeyFile) { @("-i", $KeyFile, "-o", "StrictHostKeyChecking=accept-new") } else { @() }

function Ssh-Run([string]$cmd) {
    # Write script to temp file with LF line endings to avoid \r\n issues on Linux
    $tmp = [System.IO.Path]::GetTempFileName() + ".sh"
    [System.IO.File]::WriteAllText($tmp, $cmd.Replace("`r`n", "`n"), [System.Text.UTF8Encoding]::new($false))
    scp @SshOpts -q $tmp "${Server}:/tmp/_deploy_run.sh"
    Remove-Item $tmp -Force
    ssh @SshOpts $Server "bash /tmp/_deploy_run.sh; rm -f /tmp/_deploy_run.sh"
}

Write-Host "[1/4] Compiling Linux binary..." -ForegroundColor Cyan
Push-Location $BackendDir
$env:GOOS = "linux"; $env:GOARCH = "amd64"; $env:CGO_ENABLED = "0"; $env:GOPROXY = "https://goproxy.cn,direct"
go build -trimpath -ldflags="-s -w" -o ktester-linux ./cmd/server
if ($LASTEXITCODE -ne 0) { Write-Host "Build failed" -ForegroundColor Red; exit 1 }
$env:GOOS = ""; $env:GOARCH = ""; $env:CGO_ENABLED = ""
Write-Host "  OK" -ForegroundColor Green

Write-Host "[2/4] Uploading binary + web..." -ForegroundColor Cyan
scp @SshOpts ktester-linux "${Server}:/tmp/ktester-new"
scp @SshOpts -r web "${Server}:/tmp/ktester-web"
if ($LASTEXITCODE -ne 0) { Write-Host "Upload failed" -ForegroundColor Red; exit 1 }
Remove-Item ktester-linux -Force
Write-Host "  OK" -ForegroundColor Green

Write-Host "[3/4] Running hot-update on server..." -ForegroundColor Cyan
# Use single-quoted heredoc so PowerShell does NOT interpolate $(...) bash expressions
$remote = @'
#!/bin/bash
set -e
INSTALL=/opt/ktester
BACKUP=/opt/ktester/backups
mkdir -p $BACKUP
STAMP=$(date +%Y%m%d-%H%M%S)
echo "  Backing up data -> $BACKUP/pre-update-$STAMP.tar.gz"
tar -czf $BACKUP/pre-update-$STAMP.tar.gz -C $INSTALL data 2>/dev/null || true
systemctl stop ktester
cp /tmp/ktester-new $INSTALL/ktester
chmod +x $INSTALL/ktester
cp -r /tmp/ktester-web/. $INSTALL/web/
rm -f /tmp/ktester-new
rm -rf /tmp/ktester-web
systemctl start ktester
sleep 1
systemctl status ktester --no-pager -l
echo "Done"
'@

Ssh-Run $remote
Write-Host "[4/4] Update complete." -ForegroundColor Green
Pop-Location

