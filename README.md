# Keyboard Tester V2

一个基于 Qt 6 / C++17 的键盘测试工具 — **完全由外部 JSON 配列文件驱动**，
配合一个 Go + SQLite 的后端服务做配列分发与设备管理。

## 主要特性

- **KLE 风格的 JSON 布局**：每个键由 `x, y, w, h`（以 1u 为单位）描述，支持 1.5u / 2.25u Shift、6.25u 空格、ISO Enter 等异形。
- **VID-PID 自动匹配**：连接设备后按 VID-PID 在 `layouts/device_map.json` 里查到对应配列即刻加载。
- **服务器同步配列**：登录后从服务端拉取所有设备配列并写入 `layouts/`，自动更新 `device_map.json`。
- **多媒体键支持**：`WH_KEYBOARD_LL` 钩子捕获 `VK_VOLUME_*` / `VK_MEDIA_*` / `VK_BROWSER_*` 并映射为 OHID `KC_*`。钩子只在设备连接后启用，不影响输入密码等正常打字。
- **OHID HID 通道**：`DeviceManager` 走 hidapi + OpenAgreementHID 做扫描、同步、底色测试。

## 目录结构

```
KeyboardTesterV2/
├── main.cpp
├── MainWindow.{h,cpp}        主窗口、UI 编排、服务器菜单
├── DeviceManager.{h,cpp}     HID 设备发现 / 连接 / OHID 同步
├── KeyHook.{h,cpp}           Windows 全局键盘钩子（含多媒体键 remap）
├── src/
│   ├── ApiClient.{h,cpp}     后端 REST 客户端
│   ├── LoginDialog.{h,cpp}   登录 / 注册 / 服务器地址对话框
│   ├── KeyLayout.{h,cpp}     KLE 布局模型 + JSON 加载 + KC 别名表
│   ├── KeyboardView.{h,cpp}  以绝对坐标 QPushButton 渲染键位
│   └── ScanCodeMap.{h,cpp}   scancode ↔ KC_* 映射（含多媒体）
├── layouts/                  配列文件 + device_map.json（运行时可被同步覆盖）
│   ├── device_map.json       { "devices": { "0xVID-0xPID": "0xVID-0xPID/file.c" } }
│   └── 0xVVVV-0xPPPP/        每个设备一个子目录，存放 .c/.json 配列
├── OpenAgreementHID/         OHID 协议（Pack/Port/KeyBoard）
├── Layout/                   HID_Usage_Tables / OHID_Layout
├── hid/                      InterfaceHID 封装
├── Library/hidapi/           hidapi.dll
└── backend/                  Go 后端服务
```

## 服务器同步流程

1. **菜单 → 服务器 → 服务器地址…** 设置后端 URL（默认 `http://47.112.13.246:3030`）。
2. **菜单 → 服务器 → 登录…** 输入用户名/密码（首次使用可切到"注册"页）。
3. **菜单 → 服务器 → 同步设备列表** 拉取所有设备并写入 `layouts/0xVID-0xPID/<filename>`，同时更新 `layouts/device_map.json`，下拉框立即刷新。
4. 切换 VID-PID 时若已登录，会在后台拉取最新配列；新版本到达后自动重载。

> 离线状态下程序仅使用本地 `layouts/` 已有的配列。

## 权限模型（工厂 / 用户 / 设备）

后端支持多工厂隔离：

- **超级管理员（super_admin）**：能看/管全部工厂、全部用户、全部设备。
  - 在"工厂管理"里增删工厂。
  - 编辑用户时为其勾选所属工厂。
- **管理员（admin）**：可从属多个工厂。
  - 只能看到自己工厂里的设备和用户。
  - 可对本工厂的普通用户设置"可见设备"白名单。
  - 创建/修改设备时只能选自己的工厂。
- **普通用户（user）**：恰好属于一个工厂。
  - 默认能看到本工厂全部设备；若管理员设置了白名单则只看白名单内的。
  - 无管理权限。

Qt 客户端"同步设备列表"返回的就是当前账号能看到的设备集合，无需额外配置。

## JSON 布局格式

```jsonc
{
  "name": "ANSI 75% (sample)",
  "unit": 54,        // 1u = 54 px（默认 54）
  "margin": 8,
  "keys": [
    { "x": 0, "y": 0, "label": "Esc", "code": "ESC" },
    { "x": 12.75, "y": 3.25, "label": "Enter", "w": 2.25, "code": "ENTER" },
    { "x": 5, "y": 1, "label": "5", "code": "5", "row": 0, "col": 4 },
    { "x": 10, "y": 0, "label": "—", "decal": true },
    {
      "x": 12.75, "y": 2.25, "w": 1.5, "h": 2,
      "x2": 12.5,  "y2": 3.25, "w2": 1.75, "h2": 1,
      "label": "Enter", "code": "ENTER"
    },
    { "x": 0, "y": 0, "label": "🔊+", "code": "KC_VINC" }
  ]
}
```

### 字段速查

| 字段       | 含义                                          | 默认    |
|------------|-----------------------------------------------|---------|
| `x`,`y`    | 键左上角坐标（key units）                     | 必填    |
| `w`,`h`    | 键宽 / 高（key units）                        | `1`,`1` |
| `x2`…`h2`  | 第二个矩形（ISO Enter / Big-Ass Enter）       | 无      |
| `label`    | 显示文字（不写则自动从 code 生成）            | 自动    |
| `code`     | KC 名称 / 别名 / 十六进制                     | `KC_NO` |
| `row`,`col`| 强制 OHID 矩阵行列                            | -1      |
| `decal`    | 装饰键，不参与测试                            | false   |

### 支持的 `code` 写法

- KC 名称：`"KC_ESC"`、`"KC_VINC"`
- 简写：`"ESC"`、`"A"`、`"F12"`、`"LSFT"`、`"VINC"`、`"MUTE"`、`"PLAY"`
- 数字：`"0x0029"`、`"0xC0E9"`、`41`

完整别名见 `src/KeyLayout.cpp` 中的 `kAliases[]`。

## 多媒体键映射

| 物理键      | scancode（扩展）| KC_*        |
|-------------|-----------------|-------------|
| Mute        | `0x120`         | `KC_MUTE`   |
| Vol -       | `0x12E`         | `KC_VDEC`   |
| Vol +       | `0x130`         | `KC_VINC`   |
| Play/Pause  | `0x122`         | `KC_PLAY`   |
| Stop        | `0x124`         | `KC_CSTOP`  |
| Prev Track  | `0x110`         | `KC_PTRACK` |
| Next Track  | `0x119`         | `KC_NTRACK` |

## 构建

需要 Qt 6 (Widgets + Network) + MinGW 64-bit。

```sh
qmake KeyboardTesterV2.pro
mingw32-make
```

构建脚本会把 `Library/hidapi/win/x64/hidapi.dll` 与 `layouts/` 复制到输出目录。

## 服务器部署 / 更新

后端代码在 `backend/`，部署在 `/opt/ktester/`，systemd 服务名 `ktester`，端口 `3030`。

### 一键更新（Windows）

```powershell
cd E:\hallsdk\code\KeyboardTesterV2\backend
.\deploy-update.ps1
```

脚本执行步骤：
1. 交叉编译 Linux 二进制（`GOOS=linux GOARCH=amd64 CGO_ENABLED=0`，`GOPROXY=https://goproxy.cn,direct`）。
2. `scp` 上传二进制 + `web/` 到服务器 `/tmp/`。
3. SSH 执行：备份 `data/` 到 `/opt/ktester/backups/pre-update-<时间戳>.tar.gz` → `systemctl stop ktester` → 替换二进制与 web → `systemctl start ktester` → 打印 `systemctl status`。

> 用户数据（SQLite + 已上传的配列）位于 `/opt/ktester/data/`，脚本只备份不删除。

### 首次设置免密 SSH（只需一次）

```powershell
# 1) 生成密钥（提示 passphrase 时直接按两次回车 = 空密码）
ssh-keygen -t ed25519 -f "$env:USERPROFILE\.ssh\id_ktester"

# 2) 上传公钥（最后一次输服务器密码）
$pub = Get-Content "$env:USERPROFILE\.ssh\id_ktester.pub"
ssh root@47.112.13.246 "echo '$pub' >> ~/.ssh/authorized_keys"

# 3) 验证免密
ssh -i "$env:USERPROFILE\.ssh\id_ktester" root@47.112.13.246 "echo ok"
```

之后 `deploy-update.ps1` 全程无需密码。脚本默认读 `$env:USERPROFILE\.ssh\id_ktester`，可用 `-KeyFile` 参数覆盖。

### 服务器侧常用命令

```bash
systemctl status ktester          # 服务状态
systemctl restart ktester         # 重启
journalctl -u ktester -f          # 实时日志
ls /opt/ktester/backups           # 备份列表
cat /opt/ktester/.env             # 查看 JWT 密钥 / super-admin 初始密码
```
# Keyboard Tester V2

一个基于 Qt 6 / C++17 的键盘测试工具 — **完全由外部 JSON 配列文件驱动**，  
不再像旧版那样把配列硬编码进代码、靠 6×21 矩阵显隐键位。

## 主要特性

- **KLE 风格的 JSON 布局**：每个键的位置由 `x, y, w, h`（以 1u 为单位）描述，  
  支持 1.5u / 2.25u Shift、6.25u 空格、ISO Enter 等任意几何形状。
- **外部导入布局**：菜单/工具栏 → *导入布局…* 选择 `.json` 即可即时切换。
- **完全按配列显示**：键盘 UI 仅渲染 JSON 里声明的键，不存在隐藏占位。
- **多媒体按键支持**：通过 Windows `WH_KEYBOARD_LL` 全局钩子捕获  
  `VK_VOLUME_*`、`VK_MEDIA_*`、`VK_BROWSER_*` 等，并映射到 OHID 的  
  `KC_MUTE / KC_VINC / KC_VDEC / KC_PLAY / KC_NTRACK / KC_PTRACK / KC_CSTOP / …`。
- **保留 OHID HID 通道**：`DeviceManager` 继续走 hidapi + OpenAgreementHID  
  做扫描、同步、底色测试等。

## 目录结构

```
KeyboardTesterV2/
├── main.cpp
├── MainWindow.{h,cpp}        主窗口、UI 编排
├── DeviceManager.{h,cpp}     HID 设备发现 / 连接 / OHID 同步
├── KeyHook.{h,cpp}           Windows 全局键盘钩子（含多媒体键 remap）
├── src/
│   ├── KeyLayout.{h,cpp}     KLE 布局模型 + JSON 加载 + KC 别名表
│   ├── KeyboardView.{h,cpp}  以绝对坐标 QPushButton 渲染键位
│   └── ScanCodeMap.{h,cpp}   scancode ↔ KC_* 映射（含多媒体）
├── layouts/
│   ├── sample_75.json        75% 配列示例
│   └── sample_104.json       全 104 键 + 媒体键示例
├── OpenAgreementHID/         OHID 协议（Pack/Port/KeyBoard）
├── Layout/                   HID_Usage_Tables / OHID_Layout
├── hid/                      InterfaceHID 封装
└── Library/hidapi/           hidapi.dll
```

## JSON 布局格式

```jsonc
{
  "name": "ANSI 75% (sample)",
  "unit": 54,        // 1u = 54 px（不写时默认 54）
  "margin": 8,       // 外边距 px
  "keys": [
    { "x": 0, "y": 0, "label": "Esc", "code": "ESC" },
    { "x": 12.75, "y": 3.25, "label": "Enter", "w": 2.25, "code": "ENTER" },

    // 可选 row/col（若您想强制 OHID 矩阵位）
    { "x": 5, "y": 1, "label": "5", "code": "5", "row": 0, "col": 4 },

    // 装饰键（不参与测试，不响应点击）
    { "x": 10, "y": 0, "label": "—", "decal": true },

    // 双矩形（ISO Enter 等异形键）
    {
      "x": 12.75, "y": 2.25, "w": 1.5, "h": 2,
      "x2": 12.5, "y2": 3.25, "w2": 1.75, "h2": 1,
      "label": "Enter", "code": "ENTER"
    },

    // 直接写 OHID 码（任一种均可）
    { "x": 0, "y": 0, "label": "🔊+", "code": "0xC0E9" },
    { "x": 1, "y": 0, "label": "🔊+", "code": "KC_VINC" },
    { "x": 2, "y": 0, "label": "🔊+", "code": "VINC" }
  ]
}
```

### 字段速查

| 字段       | 含义                                                  | 默认       |
|------------|-------------------------------------------------------|------------|
| `x`,`y`    | 键左上角坐标（key units）                             | 必填       |
| `w`,`h`    | 键宽 / 高（key units）                                | `1`,`1`    |
| `x2`…`h2`  | 第二个矩形（用于 ISO Enter / Big-Ass Enter）          | 不存在     |
| `label`    | 显示文字（不写则自动从 code 生成）                    | 自动       |
| `code`     | KC 名称 / 别名 / 十六进制                             | `KC_NO`    |
| `row`,`col`| 强制 OHID 矩阵行列（可省）                            | -1         |
| `decal`    | 装饰键，不参与测试                                    | `false`    |

### 支持的 `code` 写法

- KC 名称：`"KC_ESC"`、`"KC_VINC"`
- 简写：`"ESC"`、`"A"`、`"F12"`、`"LSFT"`、`"VINC"`、`"MUTE"`、`"PLAY"`
- 数字：`"0x0029"`、`"0xC0E9"`、`41`

完整别名见 `src/KeyLayout.cpp` 中的 `kAliases[]`。

## 多媒体键

由 `KeyHook` 在 `WH_KEYBOARD_LL` 回调里把没有 PS/2 扫描码的 VK
（`VK_VOLUME_*` / `VK_MEDIA_*` / `VK_BROWSER_*`）映射到通用扩展扫描码，  
随后 `ScanCodeMap` 把这些扫描码翻译为 OHID 多媒体 `KC_*`：

| 物理键      | scancode（扩展）| KC_*       |
|-------------|-----------------|------------|
| Mute        | `0x120`         | `KC_MUTE`  |
| Vol -       | `0x12E`         | `KC_VDEC`  |
| Vol +       | `0x130`         | `KC_VINC`  |
| Play/Pause  | `0x122`         | `KC_PLAY`  |
| Stop        | `0x124`         | `KC_CSTOP` |
| Prev Track  | `0x110`         | `KC_PTRACK`|
| Next Track  | `0x119`         | `KC_NTRACK`|

在你的 JSON 里直接写 `"code": "VINC"` 即可让程序识别并高亮这个键。

## 构建

需要 Qt 6 (Widgets) + MinGW 64-bit。

```sh
qmake KeyboardTesterV2.pro
mingw32-make
```

构建脚本会把 `Library/hidapi/win/x64/hidapi.dll` 与 `layouts/` 目录复制到
输出目录，运行时程序会优先尝试加载 `layouts/sample_75.json`。

## 与旧版的差异

旧版（`KeyboardFactoryTester`）把每个键写死在 6×21 的 `OHID_KeyType_T` 表里
（`TK51Q[]`、`K104[]` …），UI 通过 `QGridLayout` 显示或隐藏对应单元格。  
本版彻底舍弃了 *硬编码矩阵 + 显隐* 的思路，所有几何位置、键码、显示标签都
由外部 JSON 决定，运行时无需重新编译即可支持任意新机型。

