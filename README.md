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

