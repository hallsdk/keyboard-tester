#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <cstdint>

#include "src/KeyLayout.h"
#include "DeviceManager.h"

class QComboBox;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class QAction;
class QTimer;
class QHBoxLayout;

class KeyboardView;
class DeviceManager;
class KeyHook;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onImportLayout();
    void onReloadLayout();
    void onConnectClicked();
    void onDisconnectClicked();
    void onDeviceConnected(uint32_t board_id);
    void onDeviceDisconnected();
    void onVidPidChanged(const QString& text);
    void onQueryFnLayer();
    void onToggleFnView(bool on);

    void onResetTest();
    void onGlobalKey(int scanCode, bool pressed);

    void onLightColor();
    void onAbout();
    void onCheckUpdate();

private:
    void buildUi();
    bool loadLayoutFile(const QString& path);
    void appendLog(const QString& msg);
    // Look up VID-PID in layouts/device_map.json (next to exe).
    // Returns the resolved absolute path of the mapped layout file,
    // or an empty string if no mapping exists / file missing.
    QString resolveLayoutForVidPid(uint16_t vid, uint16_t pid) const;
    bool tryAutoLoadForVidPid(uint16_t vid, uint16_t pid);

    KeyboardView*   m_view         = nullptr;
    QComboBox*      m_vidpid       = nullptr;
    QPushButton*    m_btnConn      = nullptr;
    QPushButton*    m_btnDisc      = nullptr;
    QPushButton*    m_btnReset     = nullptr;
    QPushButton*    m_btnImport    = nullptr;
    QPushButton*    m_btnReload    = nullptr;
    QLabel*         m_statusBoard  = nullptr;
    QLabel*         m_statusLayout = nullptr;
    QLabel*         m_statusCount  = nullptr;
    QLabel*         m_statusKey    = nullptr;   // most recent key, incl. multimedia
    QPlainTextEdit* m_log          = nullptr;

    QPushButton*    m_btnLightR = nullptr;
    QPushButton*    m_btnLightG = nullptr;
    QPushButton*    m_btnLightB = nullptr;
    QPushButton*    m_btnLightY = nullptr;
    QPushButton*    m_btnLightW = nullptr;
    QPushButton*    m_btnLightOff = nullptr;

    QPushButton*    m_btnQueryFn = nullptr;   // 从设备读取 Fn1 层
    QPushButton*    m_btnViewFn  = nullptr;   // 切换显示 Fn0/Fn1 (隐藏代理)
    QPushButton*    m_btnLayerFn0 = nullptr;  // 键盘左侧 Fn0 按钮 (可见)
    QPushButton*    m_btnLayerFn1 = nullptr;  // 键盘左侧 Fn1 按钮 (可见)
    QHash<uint16_t, uint16_t> m_fn1Map; // FN0 keycode -> FN1 keycode (查询后填充)
    bool            m_viewingFn  = false;

    // 每个 Fn 层独立保存"按键测试状态". 在切换层时与 KeyboardView 互换.
    QHash<int, int> m_stateFn0;   // index -> KeyboardView::State (按 int 存避免循环依赖)
    QHash<int, int> m_stateFn1;
    void            saveCurrentLayerState();   // 把 view 当前状态存到对应层
    void            applyLayerStateToView();   // 根据 m_viewingFn 把对应层状态写回 view

    // 菜单中的 QAction (与隐藏 QPushButton 一起做启用/禁用同步)
    QAction*        m_actQueryFn = nullptr;
    QAction*        m_actViewFn  = nullptr;
    QAction*        m_actLights[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    // Fn 自动检测: 出现多媒体/Desk usage 码即认为 Fn 处于按下状态
    QTimer*         m_fnAutoTimer = nullptr;
    bool            m_fnAutoActive = false;  // 由自动检测临时切换到 Fn1 视图

    // ----------------- 多媒体按键面板 (Fn1 层) -----------------
    // 顶部一条按钮带, 每个按钮对应一个从 FN1 映射中检测到的多媒体/Desk 按键.
    // 当用户按 Fn+key 触发该多媒体码时, 对应按钮置 Passed (绿色).
    QWidget*                  m_mmBar     = nullptr;
    QHBoxLayout*              m_mmBarLay  = nullptr;
    QHash<uint16_t, QPushButton*> m_mmBtns;  // mm-keycode -> 按钮
    QHash<uint16_t, bool>     m_mmPassed;    // mm-keycode -> 是否已通过

    void rebuildMultimediaBar();
    int  mmTotal()   const { return m_mmBtns.size(); }
    int  mmPassed()  const;

    // ----------------- 按键测试 / 灯光测试流程 -----------------
    enum class LightStep { None, Red, Green, Blue, Yellow, White, Black, Done };
    LightStep   m_lightStep   = LightStep::None;
    bool        m_lightAnyFail = false;
    QTimer*     m_lightTimer  = nullptr;   // 1 秒间隔
    bool        m_testReportShown = false; // 防止重复弹结果

    void startLightTest();
    void applyLightColor(uint32_t argb);   // 实际下发 (目前为占位)
    void advanceLightStep();               // 进入下一颜色 (或结束)
    void showLightPrompt();                // 更新 m_statusKey 提示
    void finishTestSequence();             // 弹出成功/失败 + 还原默认灯
    void resetTestSequence();              // 取消进行中的测试

    QLabel*     m_bigResult   = nullptr;   // 半透明覆盖在键盘视图上的结果横幅
    void showBigResult(bool ok);
    void hideBigResult();
    bool isInLightTest() const { return m_lightStep != LightStep::None && m_lightStep != LightStep::Done; }

    void checkAllKeysTested();   // 调用后若全部通过则启动灯光测试

    KeyLayout       m_layout;
    QString         m_lastLayoutPath;

    DeviceManager*  m_dev  = nullptr;
    KeyHook*        m_hook = nullptr;

    // --- 文件日志: output/log/{YYYYMMDD-HHmmss}.log ---
    QFile           m_logFile;
    QTextStream     m_logStream;
    bool            m_logFileOpen = false;
    void            openLogFile();   // 创建 output/log + 打开按时间命名的日志文件
    void            closeLogFile();

    // --- 测试前保存的灯效, 测试完毕后会写回 ---
    DeviceManager::RGBState m_savedRgb;

    // --- 灯光测试 提示弹窗 (frameless, top-most, non-modal 以便 KeyHook 继续收 Enter/Esc) ---
    class QDialog*  m_lightDlg       = nullptr;
    class QLabel*   m_lightDlgSwatch = nullptr;
    class QLabel*   m_lightDlgTitle  = nullptr;
    class QLabel*   m_lightDlgHint   = nullptr;
    void            showLightDialog(const QString& name, uint32_t argb);
    void            hideLightDialog();
};

#endif // MAINWINDOW_H
