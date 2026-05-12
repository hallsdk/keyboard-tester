#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QWidget>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QDialog>
#include <QScreen>
#include <QIcon>
#include <QFile>
#include <QInputDialog>
#include <QLineEdit>

#include "src/KeyboardView.h"
#include "src/ScanCodeMap.h"
#include "DeviceManager.h"
#include "KeyHook.h"
#include "src/ApiClient.h"
#include "src/LoginDialog.h"

extern "C" {
#include "Layout/HID_Usage_Tables.h"
#include "Layout/OHID_Layout.h"
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Keyboard Tester V2 — JSON-driven KLE Layout");
    resize(1180, 620);
    setMinimumSize(680, 380);

    buildUi();

    // API client (token + base URL persisted in QSettings).
    m_api = new ApiClient(this);
    updateServerMenuLabels();

    // Device manager
    m_dev = new DeviceManager(this);
    connect(m_dev, &DeviceManager::connected,
            this, &MainWindow::onDeviceConnected);
    connect(m_dev, &DeviceManager::disconnected,
            this, &MainWindow::onDeviceDisconnected);

    // Global keyboard hook — installed only while a device is connected.
    m_hook = new KeyHook(this);
    connect(m_hook, &KeyHook::keyEvent,
            this, &MainWindow::onGlobalKey,
            Qt::QueuedConnection);

    // Fn 自动检测定时器: 收到多媒体/Desk 码就(临时)切换到 Fn1 视图,
    // 一段时间(~600ms) 没有再来就视为 Fn 已松开, 切回 Fn0.
    m_fnAutoTimer = new QTimer(this);
    m_fnAutoTimer->setSingleShot(true);
    m_fnAutoTimer->setInterval(600);
    connect(m_fnAutoTimer, &QTimer::timeout, this, [this]{
        if (!m_fnAutoActive) return;
        m_fnAutoActive = false;
        // 仅在用户没有手动勾选 "显示 Fn 层" 时回到 Fn0.
        if (m_btnViewFn && !m_btnViewFn->isChecked()) {
            onToggleFnView(false);
        }
        appendLog("[FnAuto] Fn 视为已松开 → 切回 Fn0");
    });

    // Attempt auto-load of the bundled sample layout next to the exe.
    const QString defaultPath = QCoreApplication::applicationDirPath()
                                + "/layouts/sample_75.json";
    if (QFileInfo::exists(defaultPath)) {
        loadLayoutFile(defaultPath);
    } else {
        appendLog("提示: 未找到默认布局 layouts/sample_75.json — 请通过 “导入布局” 选择 JSON.");
    }

    // If the current VID-PID has a mapping, prefer that layout (overrides sample).
    onVidPidChanged(m_vidpid->currentText());

    // 打开会话日志文件 (output/log/test_YYYYMMDD-HHmmss.log)
    openLogFile();
    if (m_logFileOpen)
        appendLog(QString("日志文件: %1").arg(m_logFile.fileName()));
    else
        appendLog("警告: 无法打开 output/log 日志文件 (将仅显示在屏幕上)");
}

MainWindow::~MainWindow()
{
    closeLogFile();
}

// ---------------------------------------------------------------------------
//  UI
// ---------------------------------------------------------------------------
void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* rootV = new QVBoxLayout(central);
    rootV->setContentsMargins(8, 6, 8, 8);
    rootV->setSpacing(6);

    // ====== 顶部菜单栏: 布局 / 测试 / 关于 ======
    auto* mbar = menuBar();

    // ---- 布局 ----
    QMenu* menuLayout = mbar->addMenu("布局(&L)");
    QAction* actImport = menuLayout->addAction("导入布局…");
    QAction* actReload = menuLayout->addAction("重载当前布局");
    menuLayout->addSeparator();
    QAction* actViewFn = menuLayout->addAction("显示 Fn 层");
    actViewFn->setCheckable(true);
    actViewFn->setEnabled(false);
    menuLayout->addSeparator();
    QAction* actEditMap = menuLayout->addAction("打开 layouts 目录…");

    // ---- 测试 ----
    QMenu* menuTest = mbar->addMenu("测试(&T)");
    QAction* actReset    = menuTest->addAction("重置测试状态");
    menuTest->addSeparator();
    QAction* actQueryFn  = menuTest->addAction("读取设备 Fn 层");
    actQueryFn->setEnabled(false);
    menuTest->addSeparator();
    QMenu*   menuLight   = menuTest->addMenu("灯光测试");
    QAction* actLR  = menuLight->addAction("红");
    QAction* actLG  = menuLight->addAction("绿");
    QAction* actLB  = menuLight->addAction("蓝");
    QAction* actLY  = menuLight->addAction("黄");
    QAction* actLW  = menuLight->addAction("白");
    QAction* actLOff= menuLight->addAction("熄灭");
    for (QAction* a : { actLR, actLG, actLB, actLY, actLW, actLOff })
        a->setEnabled(false);

    // ---- 关于 ----
    QMenu* menuAbout = mbar->addMenu("关于(&A)");
    QAction* actAbout    = menuAbout->addAction("关于本软件…");
    QAction* actCheckUpd = menuAbout->addAction("检查版本升级…");
    menuAbout->addSeparator();
    QAction* actTheme = menuAbout->addAction("切换浅色主题");
    actTheme->setCheckable(true);
    actTheme->setChecked(false);

    // ---- 服务器 ----
    QMenu* menuSrv = mbar->addMenu("服务器(&S)");
    m_actSrvLogin  = menuSrv->addAction("登录…");
    m_actSrvLogout = menuSrv->addAction("注销");
    menuSrv->addSeparator();
    m_actSrvSync   = menuSrv->addAction("同步设备列表");
    menuSrv->addSeparator();
    m_actSrvAddr   = menuSrv->addAction("服务器地址…");
    connect(m_actSrvLogin,  &QAction::triggered, this, &MainWindow::onServerLogin);
    connect(m_actSrvLogout, &QAction::triggered, this, &MainWindow::onServerLogout);
    connect(m_actSrvSync,   &QAction::triggered, this, &MainWindow::onServerSyncList);
    connect(m_actSrvAddr,   &QAction::triggered, this, &MainWindow::onServerAddress);

    // 把上面这些 QAction 关联到代理 QPushButton 指针, 这样原有的 connect / 启用逻辑无需大改.
    // (m_btnImport / m_btnReload / ... 是隐藏的 QPushButton, 只用于复用现有 slot 链路.)
    m_btnImport   = new QPushButton(this); m_btnImport->setVisible(false);
    m_btnReload   = new QPushButton(this); m_btnReload->setVisible(false);
    m_btnReset    = new QPushButton(this); m_btnReset->setVisible(false);
    m_btnQueryFn  = new QPushButton(this); m_btnQueryFn->setVisible(false); m_btnQueryFn->setEnabled(false);
    m_btnViewFn   = new QPushButton(this); m_btnViewFn->setVisible(false);  m_btnViewFn->setCheckable(true); m_btnViewFn->setEnabled(false);
    m_btnLightR   = new QPushButton(this); m_btnLightR->setVisible(false);  m_btnLightR->setEnabled(false);
    m_btnLightG   = new QPushButton(this); m_btnLightG->setVisible(false);  m_btnLightG->setEnabled(false);
    m_btnLightB   = new QPushButton(this); m_btnLightB->setVisible(false);  m_btnLightB->setEnabled(false);
    m_btnLightY   = new QPushButton(this); m_btnLightY->setVisible(false);  m_btnLightY->setEnabled(false);
    m_btnLightW   = new QPushButton(this); m_btnLightW->setVisible(false);  m_btnLightW->setEnabled(false);
    m_btnLightOff = new QPushButton(this); m_btnLightOff->setVisible(false); m_btnLightOff->setEnabled(false);

    // Forward menu actions → hidden buttons → existing slots.
    connect(actImport,   &QAction::triggered, this, &MainWindow::onImportLayout);
    connect(actReload,   &QAction::triggered, this, &MainWindow::onReloadLayout);
    connect(actReset,    &QAction::triggered, this, &MainWindow::onResetTest);
    connect(actQueryFn,  &QAction::triggered, this, &MainWindow::onQueryFnLayer);
    connect(actViewFn,   &QAction::toggled,   this, &MainWindow::onToggleFnView);
    connect(actViewFn,   &QAction::toggled,   m_btnViewFn,  &QPushButton::setChecked);
    connect(m_btnViewFn, &QPushButton::toggled, actViewFn,  &QAction::setChecked);

    // Light actions reuse onLightColor() — set sender's "color hint" via property.
    auto wireLight = [this](QAction* a, QPushButton* b, uint32_t color) {
        b->setProperty("lightColor", color);
        connect(a, &QAction::triggered, b, &QPushButton::click);
        connect(b, &QPushButton::clicked, this, &MainWindow::onLightColor);
    };
    wireLight(actLR,   m_btnLightR,   0xFFFF0000);
    wireLight(actLG,   m_btnLightG,   0xFF00FF00);
    wireLight(actLB,   m_btnLightB,   0xFF0000FF);
    wireLight(actLY,   m_btnLightY,   0xFFFFFF00);
    wireLight(actLW,   m_btnLightW,   0xFFFFFFFF);
    wireLight(actLOff, m_btnLightOff, 0x00000000);

    // QAction 启用状态在 onDeviceConnected/onDeviceDisconnected 中与对应隐藏按钮一起切换.
    m_actQueryFn  = actQueryFn;
    m_actViewFn   = actViewFn;
    m_actLights[0] = actLR; m_actLights[1] = actLG; m_actLights[2] = actLB;
    m_actLights[3] = actLY; m_actLights[4] = actLW; m_actLights[5] = actLOff;

    // About / update / theme.
    connect(actAbout,    &QAction::triggered, this, &MainWindow::onAbout);
    connect(actCheckUpd, &QAction::triggered, this, &MainWindow::onCheckUpdate);
    connect(actTheme, &QAction::toggled, this, [this](bool light) {
        applyTheme(!light);
        m_actTheme->setText(light ? "切换深色主题" : "切换浅色主题");
    });
    m_actTheme = actTheme;

    // Open the layouts dir (where device_map.json lives) in OS file browser.
    connect(actEditMap, &QAction::triggered, this, [this]{
        const QString dir = QCoreApplication::applicationDirPath() + "/layouts";
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
        appendLog(QString("已在文件管理器中打开: %1").arg(dir));
    });

    // ====== 设备控制行 ======
    auto* devH = new QHBoxLayout();
    devH->setSpacing(6);
    devH->addWidget(new QLabel("VID-PID:", central));
    m_vidpid = new QComboBox(central);
    m_vidpid->setEditable(true);
    // 自动扫描 layouts/ 子目录，以目录名作为 VID-PID 条目填充下拉列表.
    // 只需在 layouts/ 下创建对应子目录，程序重启后自动识别.
    {
        const QString devicesDir = QCoreApplication::applicationDirPath() + "/layouts";
        QDir dir(devicesDir);
        const QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString &d : subDirs) {
            // 只接受形如 0xXXXX-0xYYYY 的目录名
            if (d.contains('-') && d.startsWith("0x", Qt::CaseInsensitive))
                m_vidpid->addItem(d);
        }
        // 如果目录不存在或为空，回退到硬编码列表
        if (m_vidpid->count() == 0) {
            m_vidpid->addItem("0x36F9-0xAB05");
            m_vidpid->addItem("0x36F9-0xAB06");
            m_vidpid->addItem("0x2E3C-0x5745");
        }
    }
    m_vidpid->setMinimumWidth(160);
    devH->addWidget(m_vidpid);
    connect(m_vidpid, &QComboBox::currentTextChanged,
            this, &MainWindow::onVidPidChanged);

    m_btnConn = new QPushButton("连接", central);
    m_btnDisc = new QPushButton("断开", central);
    m_btnDisc->setEnabled(false);
    devH->addWidget(m_btnConn);
    devH->addWidget(m_btnDisc);
    devH->addStretch(1);
    rootV->addLayout(devH);

    // ====== 状态行 ======
    auto* statusH = new QHBoxLayout();
    statusH->setSpacing(20);

    m_statusLayout = new QLabel("布局: <未加载>", central);
    m_statusCount  = new QLabel("按键: 0", central);
    m_statusBoard  = new QLabel("设备: 未连接", central);
    m_statusKey    = new QLabel("当前按键: —", central);
    m_statusKey->setMinimumWidth(320);
    m_statusKey->setStyleSheet("QLabel { color: rgb(180,200,220); font-weight: 600; padding: 2px 8px;"
                               " border: 1px solid rgb(70,90,110); border-radius: 4px;"
                               " background-color: rgb(30,38,48); }");
    statusH->addWidget(m_statusLayout);
    statusH->addWidget(m_statusCount);
    statusH->addWidget(m_statusBoard);
    statusH->addWidget(m_statusKey);
    statusH->addStretch(1);
    rootV->addLayout(statusH);

    // ====== 多媒体按键栏 (默认隐藏, 查询过 Fn1 后才显示) ======
    m_mmBar = new QWidget(central);
    m_mmBar->setStyleSheet(
        "QWidget { background-color: rgb(28,32,42); border: 1px solid rgb(70,40,110);"
        " border-radius: 5px; }"
        "QLabel { color: rgb(200,180,230); font-weight: 600; }");
    m_mmBarLay = new QHBoxLayout(m_mmBar);
    m_mmBarLay->setContentsMargins(8, 4, 8, 4);
    m_mmBarLay->setSpacing(6);
    auto* mmTitle = new QLabel("多媒体键 (Fn 层):", m_mmBar);
    m_mmBarLay->addWidget(mmTitle);
    m_mmBarLay->addStretch(1);
    m_mmBar->setVisible(false);
    rootV->addWidget(m_mmBar);

    // ====== 键盘视图 + 日志 ======
    auto* split = new QSplitter(Qt::Vertical, central);
    split->setChildrenCollapsible(false);

    // 键盘上方/左侧容器: 左边一列 Fn0 / Fn1 切换按钮, 右边是 KeyboardView
    auto* kbHost = new QWidget(split);
    auto* kbH    = new QHBoxLayout(kbHost);
    kbH->setContentsMargins(0, 0, 0, 0);
    kbH->setSpacing(6);

    auto* layerCol = new QWidget(kbHost);
    auto* layerV   = new QVBoxLayout(layerCol);
    layerV->setContentsMargins(2, 2, 2, 2);
    layerV->setSpacing(6);

    m_btnLayerFn0 = new QPushButton("Fn0\n层", layerCol);
    m_btnLayerFn1 = new QPushButton("Fn1\n层", layerCol);
    for (QPushButton* b : { m_btnLayerFn0, m_btnLayerFn1 }) {
        b->setCheckable(true);
        b->setAutoExclusive(true);
        b->setMinimumWidth(58);
        b->setMinimumHeight(56);
        b->setStyleSheet(
            "QPushButton { background-color: rgb(60,68,82); color: rgb(220,225,235);"
            " border: 1px solid rgb(80,90,110); border-radius: 6px;"
            " font-weight: 700; padding: 4px; }"
            "QPushButton:hover { background-color: rgb(80,90,110); }"
            "QPushButton:checked { background-color: rgb(40,110,180);"
            " color: white; border: 1px solid rgb(80,160,230); }"
            "QPushButton:disabled { color: rgb(120,125,135);"
            " background-color: rgb(45,50,60); border-color: rgb(60,65,75); }");
    }
    m_btnLayerFn0->setChecked(true);
    m_btnLayerFn1->setEnabled(false);   // 查到 Fn1 映射后再启用
    layerV->addWidget(m_btnLayerFn0);
    layerV->addWidget(m_btnLayerFn1);
    layerV->addStretch(1);

    // Fn0 → 关闭 Fn 视图; Fn1 → 打开 Fn 视图. 复用 m_btnViewFn 的现有切换流程.
    connect(m_btnLayerFn0, &QPushButton::clicked, this, [this](bool){
        if (m_btnLayerFn1 && !m_btnLayerFn1->isEnabled()) {
            // Fn1 不可用时按 Fn0 是 no-op (保持选中)
            m_btnLayerFn0->setChecked(true);
            return;
        }
        if (m_btnViewFn && m_btnViewFn->isChecked()) m_btnViewFn->setChecked(false);
        else onToggleFnView(false);
    });
    connect(m_btnLayerFn1, &QPushButton::clicked, this, [this](bool){
        if (m_btnViewFn) {
            if (!m_btnViewFn->isChecked()) m_btnViewFn->setChecked(true);
            else onToggleFnView(true);
        } else {
            onToggleFnView(true);
        }
    });
    // 当其它入口 (菜单/FnAuto) 切换视图时, 让左侧按钮保持同步
    auto syncLayerBtns = [this](bool on){
        if (m_btnLayerFn0 && m_btnLayerFn0->isChecked() == on)
            m_btnLayerFn0->setChecked(!on);
        if (m_btnLayerFn1 && m_btnLayerFn1->isChecked() != on)
            m_btnLayerFn1->setChecked(on);
    };
    // m_btnViewFn 在 onToggleFnView 之前 toggled, 用 toggled 信号即可同步.
    // (此时 m_btnViewFn 尚未创建? 它在前面 init 阶段已经 new 过了, 安全.)
    if (m_btnViewFn) {
        connect(m_btnViewFn, &QPushButton::toggled, this, syncLayerBtns);
    }

    m_view = new KeyboardView(kbHost);

    kbH->addWidget(layerCol, 0);
    kbH->addWidget(m_view,   1);

    m_log = new QPlainTextEdit(split);
    m_log->setReadOnly(true);
    m_log->setMaximumBlockCount(2000);
    m_log->setStyleSheet("background-color: rgb(25,28,33); color: rgb(210,215,220); "
                         "font-family: Consolas, 'Courier New', monospace; font-size: 11px;");

    split->addWidget(kbHost);
    split->addWidget(m_log);
    split->setStretchFactor(0, 5);
    split->setStretchFactor(1, 1);
    rootV->addWidget(split, 1);

    // 半透明结果横幅 (覆盖在键盘视图上)
    m_bigResult = new QLabel(m_view);
    m_bigResult->setAlignment(Qt::AlignCenter);
    m_bigResult->setVisible(false);
    m_bigResult->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_bigResult->setStyleSheet(
        "QLabel { background-color: rgba(0,0,0,200); color: white;"
        " font-size: 52px; font-weight: 900; border-radius: 8px;"
        " padding: 30px 50px; }");

    // ====== 信号 ======
    connect(m_btnConn,   &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_btnDisc,   &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    // Visual click-to-pass: clicking a key in the view will mark it Passed.
    connect(m_view, &KeyboardView::keyClicked, this, [this](int idx) {
        if (m_view->keyState(idx) == KeyboardView::Passed)
            m_view->setKeyState(idx, KeyboardView::Idle);
        else
            m_view->setKeyState(idx, KeyboardView::Passed);
    });

    // Apply initial dark theme.
    applyTheme(true);
}

// ---------------------------------------------------------------------------
//  Theme
// ---------------------------------------------------------------------------
void MainWindow::applyTheme(bool dark)
{
    m_isDark = dark;
    if (m_view) m_view->setDarkTheme(dark);

    if (dark) {
        setStyleSheet(
            "QMenuBar { background-color: rgb(38,44,54); color: rgb(220,225,235); padding: 2px; }"
            "QMenuBar::item:selected { background-color: rgb(70,85,110); }"
            "QMenu { background-color: rgb(38,44,54); color: rgb(220,225,235); border: 1px solid rgb(70,80,95); }"
            "QMenu::item:selected { background-color: rgb(70,90,120); }"
            "QMenu::item:disabled { color: rgb(110,115,125); }");
        centralWidget()->setStyleSheet(
            "QWidget { background-color: rgb(32,36,44); color: rgb(220,225,235); }"
            "QPushButton { background-color: rgb(60,68,82); padding: 4px 10px;"
            "  border: 1px solid rgb(85,95,110); border-radius: 4px; }"
            "QPushButton:hover { background-color: rgb(80,90,108); }"
            "QPushButton:disabled { color: rgb(120,120,120); background-color: rgb(45,50,60); }"
            "QComboBox { background-color: rgb(60,68,82); padding: 2px 6px; }"
            "QLabel { color: rgb(220,225,235); }");
        if (m_log) m_log->setStyleSheet(
            "background-color: rgb(25,28,33); color: rgb(210,215,220);"
            " font-family: Consolas, 'Courier New', monospace; font-size: 11px;");
        if (m_statusKey) m_statusKey->setStyleSheet(
            "QLabel { color: rgb(180,200,220); font-weight: 600; padding: 2px 8px;"
            " border: 1px solid rgb(70,90,110); border-radius: 4px;"
            " background-color: rgb(30,38,48); }");
        if (m_mmBar) m_mmBar->setStyleSheet(
            "QWidget { background-color: rgb(28,32,42); border: 1px solid rgb(70,40,110);"
            " border-radius: 5px; }"
            "QLabel { color: rgb(200,180,230); font-weight: 600; }");
        const QString layerBtnSS =
            "QPushButton { background-color: rgb(60,68,82); color: rgb(220,225,235);"
            " border: 1px solid rgb(80,90,110); border-radius: 6px;"
            " font-weight: 700; padding: 4px; }"
            "QPushButton:hover { background-color: rgb(80,90,110); }"
            "QPushButton:checked { background-color: rgb(40,110,180);"
            " color: white; border: 1px solid rgb(80,160,230); }"
            "QPushButton:disabled { color: rgb(120,125,135);"
            " background-color: rgb(45,50,60); border-color: rgb(60,65,75); }";
        if (m_btnLayerFn0) m_btnLayerFn0->setStyleSheet(layerBtnSS);
        if (m_btnLayerFn1) m_btnLayerFn1->setStyleSheet(layerBtnSS);
    } else {
        setStyleSheet(
            "QMenuBar { background-color: rgb(238,240,245); color: rgb(30,35,50); padding: 2px;"
            " border-bottom: 1px solid rgb(200,203,215); }"
            "QMenuBar::item:selected { background-color: rgb(200,208,228); }"
            "QMenu { background-color: rgb(248,249,252); color: rgb(30,35,50);"
            " border: 1px solid rgb(195,198,215); }"
            "QMenu::item:selected { background-color: rgb(195,208,235); }"
            "QMenu::item:disabled { color: rgb(170,172,182); }");
        centralWidget()->setStyleSheet(
            "QWidget { background-color: rgb(242,243,247); color: rgb(30,35,50); }"
            "QPushButton { background-color: rgb(220,223,232); padding: 4px 10px;"
            "  border: 1px solid rgb(175,180,200); border-radius: 4px; color: rgb(20,28,50); }"
            "QPushButton:hover { background-color: rgb(200,205,222); }"
            "QPushButton:disabled { color: rgb(165,168,180); background-color: rgb(210,212,220); }"
            "QComboBox { background-color: rgb(220,223,232); padding: 2px 6px; color: rgb(20,28,50);"
            " border: 1px solid rgb(175,180,200); }"
            "QLabel { color: rgb(30,35,50); }");
        if (m_log) m_log->setStyleSheet(
            "background-color: rgb(248,249,252); color: rgb(30,38,55);"
            " font-family: Consolas, 'Courier New', monospace; font-size: 11px;"
            " border: 1px solid rgb(200,203,215);");
        if (m_statusKey) m_statusKey->setStyleSheet(
            "QLabel { color: rgb(20,60,120); font-weight: 600; padding: 2px 8px;"
            " border: 1px solid rgb(130,155,200); border-radius: 4px;"
            " background-color: rgb(220,230,248); }");
        if (m_mmBar) m_mmBar->setStyleSheet(
            "QWidget { background-color: rgb(230,228,248); border: 1px solid rgb(160,140,210);"
            " border-radius: 5px; }"
            "QLabel { color: rgb(80,50,150); font-weight: 600; }");
        const QString layerBtnSS =
            "QPushButton { background-color: rgb(215,218,230); color: rgb(20,28,55);"
            " border: 1px solid rgb(170,175,198); border-radius: 6px;"
            " font-weight: 700; padding: 4px; }"
            "QPushButton:hover { background-color: rgb(195,200,220); }"
            "QPushButton:checked { background-color: rgb(60,130,210);"
            " color: white; border: 1px solid rgb(40,100,190); }"
            "QPushButton:disabled { color: rgb(165,168,180);"
            " background-color: rgb(210,212,222); border-color: rgb(185,188,205); }";
        if (m_btnLayerFn0) m_btnLayerFn0->setStyleSheet(layerBtnSS);
        if (m_btnLayerFn1) m_btnLayerFn1->setStyleSheet(layerBtnSS);
    }
}

// ---------------------------------------------------------------------------
//  Logging
// ---------------------------------------------------------------------------
void MainWindow::openLogFile()
{
    const QString dir = QCoreApplication::applicationDirPath() + "/output/log";
    QDir().mkpath(dir);
    const QString fn = QString("%1/test_%2.log")
                       .arg(dir)
                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss"));
    m_logFile.setFileName(fn);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        m_logStream.setDevice(&m_logFile);
        m_logStream.setEncoding(QStringConverter::Utf8);
        m_logFileOpen = true;
        m_logStream << "=== Keyboard Tester V2 log started "
                    << QDateTime::currentDateTime().toString(Qt::ISODate)
                    << " ===\n";
        m_logStream.flush();
    } else {
        m_logFileOpen = false;
    }
}

void MainWindow::closeLogFile()
{
    if (m_logFileOpen) {
        m_logStream << "=== log closed "
                    << QDateTime::currentDateTime().toString(Qt::ISODate)
                    << " ===\n";
        m_logStream.flush();
        m_logFile.close();
        m_logFileOpen = false;
    }
}

void MainWindow::appendLog(const QString& msg)
{
    const QString ts = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    const QString line = QString("[%1] %2").arg(ts, msg);
    if (m_log) m_log->appendPlainText(line);
    if (m_logFileOpen) {
        m_logStream << line << '\n';
        m_logStream.flush();
    }
}

// ---------------------------------------------------------------------------
//  Layout import / reload
// ---------------------------------------------------------------------------
bool MainWindow::loadLayoutFile(const QString& path)
{
    KeyLayout nl;
    QString err;
    if (!nl.loadFromFile(path, &err)) {
        QMessageBox::warning(this, "加载布局失败", err);
        appendLog("加载布局失败: " + err);
        return false;
    }
    m_layout = nl;
    m_lastLayoutPath = path;
    m_view->setLayout(m_layout);

    // ----- Fn 键标记为 Disabled (无法通过 OS 检测, 不计入测试目标) -----
    int fnDisabled = 0;
    for (int i = 0; i < m_view->keyCount(); ++i) {
        const KeyDef* k = m_view->keyAt(i);
        if (!k) continue;
        const QString cn = k->codeName.toUpper();
        const QString lb = k->label.trimmed();
        const bool isFn =
            cn.startsWith("FK_FN") ||
            lb.compare("Fn", Qt::CaseInsensitive) == 0 ||
            lb.compare("FN", Qt::CaseInsensitive) == 0;
        if (isFn) { m_view->setKeyDisabled(i); ++fnDisabled; }
    }
    if (fnDisabled > 0)
        appendLog(QString("已将 %1 个 Fn 键标记为不可测 (OS 无法检测)").arg(fnDisabled));

    // 切换布局后重置测试与多媒体记录
    m_mmPassed.clear();
    resetTestSequence();
    rebuildMultimediaBar();

    m_statusLayout->setText(QString("布局: %1").arg(m_layout.name()));
    m_statusCount->setText(QString("按键: %1 (可测 %2)")
                           .arg(m_layout.keys().size())
                           .arg(m_view->totalTestable()));
    appendLog(QString("已加载布局 \"%1\" — %2 个键, %3 个可测试")
              .arg(m_layout.name())
              .arg(m_layout.keys().size())
              .arg(m_view->totalTestable()));

    // 切换/重载布局后 Fn1 映射不再有效, 清空; 若设备已连接, 自动重读一次.
    m_fn1Map.clear();
    if (m_btnLayerFn1) m_btnLayerFn1->setEnabled(false);
    if (m_btnLayerFn0) m_btnLayerFn0->setChecked(true);
    if (m_dev && m_dev->isConnected()) {
        QTimer::singleShot(0, this, [this]{
            if (!m_dev || !m_dev->isConnected()) return;
            if (m_view->keyCount() == 0) return;
            appendLog("[Auto] 布局变更, 自动查询 Fn 层 ...");
            onQueryFnLayer();
        });
    }
    return true;
}

void MainWindow::onImportLayout()
{
    const QString startDir = m_lastLayoutPath.isEmpty()
        ? QCoreApplication::applicationDirPath() + "/layouts"
        : QFileInfo(m_lastLayoutPath).absolutePath();

    const QString file = QFileDialog::getOpenFileName(
        this, "选择键盘布局文件", startDir,
        "All Supported (*.json *.c *.h *.txt);;Layout JSON (*.json);;OHID Matrix (*.c *.h *.txt);;All Files (*.*)");
    if (file.isEmpty()) return;
    loadLayoutFile(file);
}

void MainWindow::onReloadLayout()
{
    if (m_lastLayoutPath.isEmpty()) {
        appendLog("尚未加载任何布局, 无法重载。");
        return;
    }
    loadLayoutFile(m_lastLayoutPath);
}

void MainWindow::onResetTest()
{
    resetTestSequence();
    m_view->resetState();
    // 重新标记 Fn 为 Disabled (resetState 把所有键置回 Idle)
    for (int i = 0; i < m_view->keyCount(); ++i) {
        const KeyDef* k = m_view->keyAt(i);
        if (!k) continue;
        const QString cn = k->codeName.toUpper();
        const QString lb = k->label.trimmed();
        if (cn.startsWith("FK_FN") ||
            lb.compare("Fn", Qt::CaseInsensitive) == 0 ||
            lb.compare("FN", Qt::CaseInsensitive) == 0)
            m_view->setKeyDisabled(i);
    }
    // 清空多媒体按键的 Passed 状态
    m_mmPassed.clear();
    for (auto* b : m_mmBtns) if (b) b->setProperty("passed", false);
    rebuildMultimediaBar();
    appendLog("测试状态已重置");
}

// ---------------------------------------------------------------------------
//  Global hook handler — scan code → KC_* → key index → highlight
// ---------------------------------------------------------------------------
void MainWindow::onGlobalKey(int scanCode, bool pressed)
{
    if (scanCode > 0x300) return;            // ignore garbage

    // ----- 灯光测试中: Space = 正确, Esc = 错误, 其它键忽略 -----
    if (isInLightTest() && pressed) {
        // Space = 0x39, Esc = 0x01
        if (scanCode == 0x39) {
            appendLog("[LightTest] 用户确认 正确");
            advanceLightStep();
            return;
        }
        if (scanCode == 0x01) {
            appendLog("[LightTest] 用户报告 错误");
            m_lightAnyFail = true;
            advanceLightStep();
            return;
        }
        // 其它按键在灯光测试期间不进入正常处理流程
        return;
    }

    const uint16_t kc = ScanCodeMap::scanToKc(scanCode);
    if (kc == KC_NO) {
        if (pressed) {
            appendLog(QString("未识别 scan=0x%1").arg(scanCode, 0, 16));
            m_statusKey->setText(QString("当前按键: 未识别 scan=0x%1").arg(scanCode, 0, 16));
            m_statusKey->setStyleSheet(
                "QLabel { color: rgb(220,180,80); font-weight: 600; padding: 2px 8px;"
                " border: 1px solid rgb(160,130,40); border-radius: 4px;"
                " background-color: rgb(60,48,20); }");
        }
        return;
    }

    // Build a pretty description (multimedia keys get a verbose name).
    const QString shortName = KeyLayout::codeToShortName(kc);
    const QString fullName  = KeyLayout::codeToHumanName(kc);

    // Probe multimedia membership without a KeyDef:
    auto isMM = [](uint16_t c) {
        const uint16_t page = c & 0xFF00;  // UT_MASK
        return page == 0x0C00 /*UT_CONSUMER*/ || page == 0x0100 /*UT_DESK*/;
    };
    const bool mm = isMM(kc);

    // ---- Fn 自动检测 ----
    // OS 无法直接读到 Fn 修饰键, 但 Fn+key 会以多媒体/Desk usage 码到达 (UT_CONSUMER/UT_DESK).
    // 一旦看到这类码, 就近似推断 Fn 处于按下状态, 自动切换到 Fn1 层视图;
    // 一段时间内 (~600ms) 没有再来就认为 Fn 已松开, 切回 Fn0.
    // 仅当已经读取过 Fn 层映射 (m_fn1Map 非空) 且用户未手动勾选 "显示 Fn 层" 时启用.
    if (pressed && mm && !m_fn1Map.isEmpty() && m_btnViewFn && !m_btnViewFn->isChecked()) {
        if (!m_fnAutoActive) {
            m_fnAutoActive = true;
            onToggleFnView(true);
            appendLog("[FnAuto] 检测到多媒体码 → 推断 Fn 按下, 切换到 Fn1 视图");
        }
        if (m_fnAutoTimer) m_fnAutoTimer->start();   // restart 600ms 看门狗
    } else if (mm && m_fnAutoActive) {
        // 释放事件也续期, 直到真的没有多媒体码再到达.
        if (m_fnAutoTimer) m_fnAutoTimer->start();
    }

    // 解析"该按键事件应映射到 view 中的哪个物理索引".
    // idx  — 当前视图中应高亮的物理键索引 (Fn0 直接找, Fn1 反查).
    // fn1Idx — 在 Fn1 映射中对应的物理键索引 (始终计算, 用于同步 m_stateFn1).
    int idx = -1;
    int fn1Idx = -1;
    auto reverseFn1 = [&]() -> int {
        for (auto it = m_fn1Map.begin(); it != m_fn1Map.end(); ++it) {
            if (it.value() == kc && it.value() != 0 && it.value() != 0xFFFF) {
                const int i = m_view->findIndexByCode(it.key());
                if (i >= 0) return i;
            }
        }
        return -1;
    };
    if (!m_fn1Map.isEmpty()) fn1Idx = reverseFn1();
    if (m_viewingFn) {
        idx = fn1Idx;
    } else {
        idx = m_view->findIndexByCode(kc);
    }

    if (pressed) {
        m_statusKey->setText(QString("当前按键: %1   %2   (KC=0x%3)")
                             .arg(shortName, fullName)
                             .arg(kc, 4, 16, QChar('0')));
        if (mm) {
            // Eye-catching color for multimedia
            m_statusKey->setStyleSheet(
                "QLabel { color: white; font-weight: 700; padding: 2px 8px;"
                " border: 1px solid rgb(120,40,160); border-radius: 4px;"
                " background-color: rgb(110,40,170); }");
        } else {
            m_statusKey->setStyleSheet(
                "QLabel { color: rgb(240,250,255); font-weight: 600; padding: 2px 8px;"
                " border: 1px solid rgb(70,140,200); border-radius: 4px;"
                " background-color: rgb(35,75,110); }");
        }

        // 若 kc 出现在多媒体栏里 (Fn1 多媒体码 或 Fn0 缺失的物理键映射),
        // 不论当前是 Fn0/Fn1 视图都直接把对应按钮标记为已测.
        if (m_mmBtns.contains(kc) && !m_mmPassed.value(kc, false)) {
            m_mmPassed[kc] = true;
            QPushButton* b = m_mmBtns[kc];
            if (b) {
                b->setStyleSheet(
                    "QPushButton { background-color: rgb(46,160,67); color: white;"
                    " border: 1px solid rgb(30,120,50); border-radius: 5px;"
                    " padding: 2px 6px; font-weight: 600; }"
                    "QPushButton:disabled { color: white; }");
            }
            appendLog(QString("多媒体键已测: %1  [%2]")
                      .arg(shortName).arg(fullName));
        }

        // 不论当前在哪一层视图, 只要 fn1Idx 有效就同步 m_stateFn1 的 Pressed 状态;
        // 这样切到 Fn1 视图后该键已经是 Pressed 颜色.
        if (fn1Idx >= 0 && !m_viewingFn) {
            m_stateFn1[fn1Idx] = static_cast<int>(KeyboardView::Pressed);
        }
    }

    if (idx < 0) {
        // 即便当前视图里找不到 idx (e.g. 在 Fn0 视图按了 Fn+PgUp 生成 Home),
        // 仍要同步 m_stateFn1 的 Passed 状态 (release 事件, fn1Idx 有效时).
        if (!pressed && fn1Idx >= 0) {
            m_stateFn1[fn1Idx] = static_cast<int>(KeyboardView::Passed);
        }
        if (pressed && !m_mmBtns.contains(kc)) {
            appendLog(QString("scan=0x%1 → %2  %3%4")
                      .arg(scanCode, 0, 16)
                      .arg(shortName, mm ? QString(" [%1]").arg(fullName) : QString())
                      .arg(QString("  (该键不在当前布局)")));
        }
        if (pressed) checkAllKeysTested();
        return;
    }

    if (pressed) {
        m_view->setKeyState(idx, KeyboardView::Pressed);
        appendLog(QString("按下 scan=0x%1  %2%3")
                  .arg(scanCode, 0, 16)
                  .arg(shortName)
                  .arg(mm ? QString("  [%1]").arg(fullName) : QString()));
    } else {
        m_view->setKeyState(idx, KeyboardView::Passed);
        // 同步 Fn1 缓存: 若当前是 Fn1 视图且同一 idx, 下次切层时已是 Passed;
        // 若当前是 Fn0 视图 (idx 来自 Fn0 直接查), 额外保存到 m_stateFn1.
        if (!m_viewingFn && fn1Idx >= 0) {
            m_stateFn1[fn1Idx] = static_cast<int>(KeyboardView::Passed);
        }
        const int passed = m_view->passedCount();
        const int total  = m_view->totalTestable();
        m_statusCount->setText(QString("按键: %1 (已测 %2 / %3)")
                               .arg(m_layout.keys().size()).arg(passed).arg(total));
        checkAllKeysTested();
    }
}

// ---------------------------------------------------------------------------
//  Device connection
// ---------------------------------------------------------------------------
void MainWindow::onConnectClicked()
{
    const QString text = m_vidpid->currentText();
    const QStringList parts = text.split('-');
    if (parts.size() != 2) {
        appendLog("VID-PID 格式错误: " + text);
        return;
    }
    bool okV=false, okP=false;
    const uint16_t vid = parts[0].toUShort(&okV, 16);
    const uint16_t pid = parts[1].toUShort(&okP, 16);
    if (!okV || !okP) {
        appendLog("VID/PID 解析失败: " + text);
        return;
    }

    appendLog(QString("尝试连接 VID=0x%1 PID=0x%2 …")
              .arg(vid, 4, 16, QChar('0'))
              .arg(pid, 4, 16, QChar('0')));

    // Try to load the matching layout before we open the device, so a fresh
    // view is up when SYNC reports back.
    tryAutoLoadForVidPid(vid, pid);

    m_btnConn->setEnabled(false);
    const bool ok = m_dev->tryConnect(vid, pid, 3000);
    m_btnConn->setEnabled(!ok);
    m_btnDisc->setEnabled(ok);
    if (!ok) appendLog("连接失败");
}

void MainWindow::onDisconnectClicked()
{
    m_dev->disconnect();
}

void MainWindow::onDeviceConnected(uint32_t board_id)
{
    // Start capturing keys only now that a device is connected.
    m_hook->install();
    const auto& info = m_dev->info();
    m_statusBoard->setText(QString("设备: 已连接 board_id=0x%1  fw=%2")
                           .arg(board_id, 8, 16, QChar('0'))
                           .arg(QString::fromLatin1((const char*)info.Version).trimmed()));
    appendLog(QString("设备已连接 board_id=0x%1")
              .arg(board_id, 8, 16, QChar('0')));

    // Enable light-test buttons
    for (QPushButton* b : { m_btnLightR, m_btnLightG, m_btnLightB,
                            m_btnLightY, m_btnLightW, m_btnLightOff }) {
        b->setEnabled(true);
    }
    m_btnQueryFn->setEnabled(true);
    if (m_actQueryFn) m_actQueryFn->setEnabled(true);
    for (QAction* a : m_actLights) if (a) a->setEnabled(true);

    // 连接后立即读取一次当前灯效, 作为"上电灯效"快照;
    // 测试结束时会写回, 让键盘回到刚上电时的样子.
    m_savedRgb = DeviceManager::RGBState{};
    if (m_dev->readRGBState(&m_savedRgb, 800)) {
        appendLog(QString("[Connect] 已记录上电灯效 (mode=%1, speed=%2, gray=%3, back=0x%4)")
                      .arg(m_savedRgb.mode).arg(m_savedRgb.speed).arg(m_savedRgb.gray)
                      .arg(m_savedRgb.back, 8, 16, QChar('0')));
    } else {
        appendLog("[Connect] 警告: 未能读取上电灯效, 测试结束后将退回默认白光");
    }

    // 连接后自动触发一次 Fn 层读取 (有布局且有可查询按键时).
    // 延迟到事件循环, 避免在 connected 信号回调内做较长的阻塞 IO.
    QTimer::singleShot(0, this, [this]{
        if (!m_dev || !m_dev->isConnected()) return;
        if (m_view->keyCount() == 0) return;        // 还未加载布局
        if (!m_fn1Map.isEmpty()) return;            // 已经查过了
        appendLog("[Auto] 自动查询 Fn 层 ...");
        onQueryFnLayer();
    });
}

void MainWindow::onDeviceDisconnected()
{
    // Stop capturing keys while no device is connected.
    m_hook->uninstall();
    m_btnConn->setEnabled(true);
    m_btnDisc->setEnabled(false);
    m_statusBoard->setText("设备: 未连接");
    appendLog("设备已断开");
    m_savedRgb = DeviceManager::RGBState{};  // 断开时清除上电灯效快照

    // 断开后不再保留上一台设备的 Fn1 映射与按键状态 (可能换了另一台).
    m_fn1Map.clear();
    m_stateFn0.clear();
    m_stateFn1.clear();
    m_mmPassed.clear();
    if (m_view) m_view->resetState();
    // 重新按布局标记 Fn 物理键为 Disabled (resetState 会清掉这个状态)
    for (int i = 0; m_view && i < m_view->keyCount(); ++i) {
        const KeyDef* k = m_view->keyAt(i);
        if (!k || k->code == 0 || k->decal) continue;
        const uint16_t page = k->code & 0xFF00;
        const bool isFn = (page == 0x1000 /*UT_OPEN*/);
        if (isFn) m_view->setKeyDisabled(i);
    }
    if (m_btnLayerFn1) m_btnLayerFn1->setEnabled(false);
    if (m_btnLayerFn0) m_btnLayerFn0->setChecked(true);
    m_viewingFn = false;
    for (QPushButton* b : { m_btnLightR, m_btnLightG, m_btnLightB,
                            m_btnLightY, m_btnLightW, m_btnLightOff }) {
        b->setEnabled(false);
    }
    m_btnQueryFn->setEnabled(false);
    m_btnViewFn->setEnabled(false);
    if (m_btnViewFn->isChecked()) m_btnViewFn->setChecked(false);
    if (m_actQueryFn) m_actQueryFn->setEnabled(false);
    if (m_actViewFn)  { m_actViewFn->setEnabled(false); m_actViewFn->setChecked(false); }
    for (QAction* a : m_actLights) if (a) a->setEnabled(false);

    // 左侧 Fn0/Fn1 按钮: 设备断开后 Fn1 不可点, 退回 Fn0.
    if (m_btnLayerFn1) m_btnLayerFn1->setEnabled(false);
    if (m_btnLayerFn0) m_btnLayerFn0->setChecked(true);
}

// ---------------------------------------------------------------------------
//  Light test stubs.
//  The full RGB pipeline (write_IRGB / OHIDRGB / mix_word) lives in the
//  original KeyboardFactoryTester/mainwindow.cpp. Wire those up via
//  DeviceManager once you bring them over.
// ---------------------------------------------------------------------------
void MainWindow::onLightColor()
{
    auto* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    // Color may be set as a property by the menu wiring; fall back to identity-based.
    bool ok = false;
    uint32_t color = (uint32_t)btn->property("lightColor").toUInt(&ok);
    if (!ok) {
        color = 0xFF000000;
        if      (btn == m_btnLightR)   color = 0xFFFF0000;
        else if (btn == m_btnLightG)   color = 0xFF00FF00;
        else if (btn == m_btnLightB)   color = 0xFF0000FF;
        else if (btn == m_btnLightY)   color = 0xFFFFFF00;
        else if (btn == m_btnLightW)   color = 0xFFFFFFFF;
        else if (btn == m_btnLightOff) color = 0x00000000;
    }

    appendLog(QString("[LightTest] 请求点亮颜色 0x%1")
              .arg(color, 8, 16, QChar('0')));
    if (!m_dev || !m_dev->isConnected()) {
        appendLog("[LightTest] 设备未连接, 无法下发");
        return;
    }
    if (!m_dev->setSolidColor(color, 500)) {
        appendLog("[LightTest] 下发失败 (设备无响应)");
    }
}

// ---------------------------------------------------------------------------
//  VID-PID → layout mapping (layouts/device_map.json)
// ---------------------------------------------------------------------------
QString MainWindow::resolveLayoutForVidPid(uint16_t vid, uint16_t pid) const
{
    // 1) Server cache (preferred when present) — written by fetchLayoutFromServer().
    const QString cached = cachedLayoutPath(vid, pid);
    if (!cached.isEmpty()) return cached;

    // 2) Local fallback: layouts/device_map.json shipped with the exe.
    const QString dir = QCoreApplication::applicationDirPath() + "/layouts";
    const QString mapPath = dir + "/device_map.json";
    QFile f(mapPath);
    if (!f.open(QIODevice::ReadOnly)) return QString();
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
    f.close();
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) return QString();

    const QJsonObject root = doc.object();
    const QJsonObject devs = root.value("devices").toObject();

    // Build the key the same way the user enters it in the combo: "0xVVVV-0xPPPP".
    const QString key = QString("0x%1-0x%2")
        .arg(vid, 4, 16, QChar('0'))
        .arg(pid, 4, 16, QChar('0'))
        .toUpper();
    // The combo strings use mixed case; also try the original-case version
    // and a lowercase variant for forgiving lookup.
    QString rel;
    for (const QString& k : { key, key.toLower(),
                              QString("0x%1-0x%2").arg(vid,4,16,QChar('0')).arg(pid,4,16,QChar('0')) }) {
        if (devs.contains(k)) { rel = devs.value(k).toString(); break; }
    }
    // Also do a case-insensitive scan as last resort.
    if (rel.isEmpty()) {
        for (auto it = devs.begin(); it != devs.end(); ++it) {
            if (it.key().compare(key, Qt::CaseInsensitive) == 0) {
                rel = it.value().toString();
                break;
            }
        }
    }
    if (rel.isEmpty()) return QString();

    // Resolve relative paths against the layouts/ folder.
    QFileInfo fi(rel);
    const QString abs = fi.isAbsolute() ? rel : QDir(dir).absoluteFilePath(rel);
    return QFileInfo::exists(abs) ? abs : QString();
}

bool MainWindow::tryAutoLoadForVidPid(uint16_t vid, uint16_t pid)
{
    // Kick off server refresh in the background (uses cached file in the meantime).
    if (m_api && m_api->isLoggedIn()) {
        QTimer::singleShot(0, this, [this, vid, pid]{ fetchLayoutFromServer(vid, pid); });
    }
    const QString path = resolveLayoutForVidPid(vid, pid);
    if (path.isEmpty()) {
        appendLog(QString("device_map.json 未配置 0x%1-0x%2 的布局")
                  .arg(vid,4,16,QChar('0')).arg(pid,4,16,QChar('0')));
        return false;
    }
    appendLog(QString("匹配到 VID-PID 布局: %1").arg(QFileInfo(path).fileName()));
    return loadLayoutFile(path);
}

void MainWindow::onVidPidChanged(const QString& text)
{
    const QStringList parts = text.split('-');
    if (parts.size() != 2) return;
    bool okV=false, okP=false;
    const uint16_t vid = parts[0].toUShort(&okV, 16);
    const uint16_t pid = parts[1].toUShort(&okP, 16);
    if (!okV || !okP) return;

    // Background refresh from server (will reload if a newer file arrives).
    if (m_api && m_api->isLoggedIn()) {
        QTimer::singleShot(0, this, [this, vid, pid]{ fetchLayoutFromServer(vid, pid); });
    }

    // Silent attempt — only switches if a mapping exists.
    const QString path = resolveLayoutForVidPid(vid, pid);
    if (!path.isEmpty() && path != m_lastLayoutPath) {
        appendLog(QString("VID-PID 切换 → 自动加载 %1").arg(QFileInfo(path).fileName()));
        loadLayoutFile(path);
    }
}

// ---------------------------------------------------------------------------
//  Fn 层读取 (OHID_CMD_DRIVER_PARAM, Item=OHID_PAGE_FN1)
// ---------------------------------------------------------------------------
void MainWindow::onQueryFnLayer()
{
    if (!m_dev || !m_dev->isConnected()) {
        appendLog("尚未连接设备, 无法读取 Fn 层");
        return;
    }

    // 收集 FN0 配列中所有非占位键的 keycode 作为查询 MKEY 列表.
    QVector<uint16_t> mkeys;
    mkeys.reserve(m_layout.keys().size());
    for (const KeyDef& k : m_layout.keys()) {
        if (k.code != 0 && !k.decal) mkeys.push_back(k.code);
    }
    if (mkeys.isEmpty()) {
        appendLog("当前布局没有可查询的按键");
        return;
    }

    appendLog(QString("→ 查询 FN1 层 ... 请求 %1 个按键 (每批 12)").arg(mkeys.size()));
    m_btnQueryFn->setEnabled(false);
    QApplication::processEvents();

    QHash<uint16_t, uint16_t> map;
    const bool ok = m_dev->readParamPage(/*page=FN1*/ 0x01, mkeys, &map, 300);
    m_btnQueryFn->setEnabled(true);

    if (!ok) {
        appendLog("读取 Fn 层失败 (超时或设备不支持)");
        return;
    }
    m_fn1Map = map;
    appendLog(QString("← 收到 FN1 映射 %1 项").arg(m_fn1Map.size()));

    // 日志里打印多媒体键, 让工厂能立刻看到 Fn 层多媒体配置.
    int mmCount = 0;
    for (auto it = m_fn1Map.begin(); it != m_fn1Map.end(); ++it) {
        const uint16_t fn0 = it.key();
        const uint16_t fn1 = it.value();
        if (fn1 == 0 || fn1 == 0xFFFF) continue;
        const uint16_t page = fn1 & 0xFF00;
        const bool mm = (page == 0x0C00 /*UT_CONSUMER*/ || page == 0x0100 /*UT_DESK*/);
        if (mm) {
            ++mmCount;
            appendLog(QString("  Fn+%1  →  %2   [%3]")
                      .arg(KeyLayout::codeToShortName(fn0))
                      .arg(KeyLayout::codeToShortName(fn1))
                      .arg(KeyLayout::codeToHumanName(fn1)));
        }
    }
    appendLog(QString("Fn 层共发现 %1 个多媒体键").arg(mmCount));

    m_btnViewFn->setEnabled(true);
    if (m_btnLayerFn1) m_btnLayerFn1->setEnabled(true);
    if (m_actViewFn) m_actViewFn->setEnabled(true);
    // 若当前已经在 Fn 视图, 刷新一次.
    if (m_btnViewFn->isChecked()) onToggleFnView(true);

    // 构建/刷新多媒体按键栏
    rebuildMultimediaBar();
}

void MainWindow::onToggleFnView(bool on)
{
    // 切层前先把当前 view 的状态保存到当前层
    saveCurrentLayerState();

    m_viewingFn = on;
    m_btnViewFn->setText(on ? "显示 Fn0 层" : "显示 Fn 层");

    const auto& keys = m_layout.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const KeyDef& k = keys[i];
        if (k.code == 0 || k.decal) continue;
        if (!on) {
            // 回到主层 — 用配列里的 label / 默认名
            const QString lbl = k.label.isEmpty()
                ? KeyLayout::codeToShortName(k.code) : k.label;
            m_view->setKeyLabel(i, lbl);
        } else {
            const uint16_t fn1 = m_fn1Map.value(k.code, 0);
            if (fn1 == 0 || fn1 == 0xFFFF) {
                m_view->setKeyLabel(i, "·");
            } else {
                m_view->setKeyLabel(i, KeyLayout::codeToShortName(fn1));
            }
        }
    }

    // 应用目标层的状态 (Idle/Pressed/Passed/Failed/Disabled)
    applyLayerStateToView();

    appendLog(on ? "切换到 Fn1 层视图" : "切换回 Fn0 层视图");
}


// ---------------------------------------------------------------------------
//  About / Version upgrade
// ---------------------------------------------------------------------------
static constexpr const char* kAppVersion = "2.1.0";

void MainWindow::onAbout()
{
    const QString text = QString(
        "<h3>SDK Keyboard Tester V2</h3>"
        "<p><b>版本</b>: %1</p>"
        "<p>基于 Qt %2 构建.</p>"
        "<p>键盘工厂测试 / 布局可视化 / OHID 协议交互工具.</p>"
        "<p>支持 KLE-JSON 与 OHID Matrix C 片段两种布局, "
        "通过 <code>layouts/device_map.json</code> 按 VID-PID 自动加载.</p>"
        "<hr>"
        "<p style='color:gray'>&copy; 2026 hallsdk</p>"
    ).arg(kAppVersion, QT_VERSION_STR);
    QMessageBox::about(this, "关于", text);
}

void MainWindow::onCheckUpdate()
{
    appendLog("[Update] 正在检查版本升级 ...");
    // TODO: 接入实际的版本服务. 这里先给出占位实现.
    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle("检查版本升级");
    box.setText(QString("当前版本: <b>%1</b>").arg(kAppVersion));
    box.setInformativeText(
        "尚未配置升级服务器.\n"
        "可在工程里实现 onCheckUpdate() 访问版本接口 (如 GitHub Releases) "
        "并下载新版本.");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
    appendLog("[Update] 检查完成 (占位实现)");
}


// ===========================================================================
//  多媒体栏 / 测试流程 / 灯光测试 / 结果横幅
// ===========================================================================

int MainWindow::mmPassed() const
{
    int n = 0;
    for (auto it = m_mmPassed.begin(); it != m_mmPassed.end(); ++it)
        if (it.value()) ++n;
    return n;
}

void MainWindow::saveCurrentLayerState()
{
    QHash<int, int>& dst = m_viewingFn ? m_stateFn1 : m_stateFn0;
    dst.clear();
    const int n = m_view ? m_view->keyCount() : 0;
    for (int i = 0; i < n; ++i) {
        const int s = static_cast<int>(m_view->keyState(i));
        dst.insert(i, s);
    }
}

void MainWindow::applyLayerStateToView()
{
    const QHash<int, int>& src = m_viewingFn ? m_stateFn1 : m_stateFn0;
    const int n = m_view ? m_view->keyCount() : 0;
    for (int i = 0; i < n; ++i) {
        // 没保存过 → 默认 Idle (Disabled 由布局加载时单独设置, 这里若曾标记会被保留)
        const KeyboardView::State def = m_view->keyState(i) == KeyboardView::Disabled
                                            ? KeyboardView::Disabled
                                            : KeyboardView::Idle;
        const KeyboardView::State s = src.contains(i)
                                          ? static_cast<KeyboardView::State>(src.value(i))
                                          : def;
        m_view->setKeyState(i, s);
    }
}

// 多媒体键 白名单 + 友好名 + 图标资源.
// 与 KeyboardFactoryTester/mainwindow.cpp 的 isMultimediaKeyCode/getMultimediaKeyName 对齐;
// 额外允许 Home/End (键盘 Fn0 配列没有时通过 Fn 调出 — 用户要求显示在多媒体栏).
struct MMEntry {
    uint16_t    code;
    const char* shortName;
    const char* friendly;
    const char* iconRes;   // 资源路径; 为空则用文字
};
static const MMEntry kMMTable[] = {
    { KC_MUTE,     "Mute",     "静音",            ":/icon/volume_mute.png" },
    { KC_VDEC,     "Vol-",     "音量减",          ":/icon/volume_down.png" },
    { KC_VINC,     "Vol+",     "音量加",          ":/icon/volume_up.png"   },
    { KC_PLAY,     "Play",     "播放/暂停",       ":/icon/media_play_pause.png" },
    { KC_CSTOP,    "Stop",     "停止",            ":/icon/media_stop.png"  },
    { KC_PTRACK,   "Prev",     "上一曲",          ":/icon/media_prev_track.png" },
    { KC_NTRACK,   "Next",     "下一曲",          ":/icon/media_next_track.png" },
    { KC_AC_BACK,  "Back",     "浏览器后退",      ":/icon/application_left.png" },
    { KC_FORWARD,  "Fwd",      "浏览器前进",      ":/icon/application_right.png" },
    { KC_REFRESH,  "Refresh",  "浏览器刷新",      ":/icon/browser__refresh.png" },
    { KC_AC_STOP,  "BrwStop",  "浏览器停止/关闭", ":/icon/close.png"       },
    { KC_SEARCH,   "Search",   "浏览器搜索",      ":/icon/browser_search.png" },
    { KC_MARKS,    "Fav",      "浏览器收藏",      ":/icon/Favorites.png"   },
    { KC_WWW,      "Home",     "浏览器主页",      ":/icon/home.png"        },
    { KC_PC,       "MyPC",     "我的电脑",        ":/icon/my_computer.png" },
    { KC_EMAIL,    "Mail",     "邮件",            ":/icon/Mail.png"        },
    { KC_CALC,     "Calc",     "计算器",          ":/icon/calculator.png"  },
    { KC_MEDIA,    "Media",    "多媒体播放器",    ":/icon/my_music.png"    },
    // 非多媒体, 但用户要求 Home/End (Fn0 缺失时) 也显示在多媒体栏
    { KC_HOME,     "Home",     "Home (光标)",     nullptr },
    { KC_END,      "End",      "End (光标)",      nullptr },
};

static const MMEntry* findMMEntry(uint16_t kc)
{
    for (const auto& e : kMMTable) if (e.code == kc) return &e;
    return nullptr;
}

void MainWindow::rebuildMultimediaBar()
{
    // 清空旧按钮 (保留 stretch)
    for (auto* b : m_mmBtns) if (b) b->deleteLater();
    m_mmBtns.clear();

    // 删除布局中除标题/stretch 外的元素
    while (m_mmBarLay->count() > 2) {
        QLayoutItem* it = m_mmBarLay->takeAt(1);
        if (it) {
            if (it->widget()) it->widget()->deleteLater();
            delete it;
        }
    }

    // 从 m_fn1Map 收集 "需要出现在多媒体栏" 的键 (按白名单 + Fn0 是否已有).
    // 注意按 kMMTable 顺序排列, 让 UI 保持一致.
    QSet<uint16_t> wanted;
    for (auto it = m_fn1Map.begin(); it != m_fn1Map.end(); ++it) {
        const uint16_t fn1 = it.value();
        if (fn1 == 0 || fn1 == 0xFFFF) continue;
        const MMEntry* e = findMMEntry(fn1);
        if (!e) continue;
        const uint16_t page = fn1 & 0xFF00;
        const bool isMmCode = (page == 0x0C00 || page == 0x0100);
        // KB 类 (如 Home/End): 仅在 Fn0 配列里没有该物理键时才放进多媒体栏
        if (!isMmCode && m_view->findIndexByCode(fn1) >= 0) continue;
        wanted.insert(fn1);
    }

    if (wanted.isEmpty()) {
        m_mmBar->setVisible(false);
        return;
    }

    int insertAt = 1;
    int shown = 0;
    for (const auto& e : kMMTable) {
        if (!wanted.contains(e.code)) continue;
        auto* b = new QPushButton(m_mmBar);
        b->setToolTip(QString::fromUtf8(e.friendly));
        b->setEnabled(false);
        b->setFixedHeight(36);
        if (e.iconRes && QFile::exists(e.iconRes)) {
            b->setIcon(QIcon(QString::fromUtf8(e.iconRes)));
            b->setIconSize(QSize(22, 22));
            b->setText("");            // 仅图标
            b->setFixedWidth(40);
        } else {
            b->setText(QString::fromUtf8(e.shortName));
            b->setMinimumWidth(56);
        }
        const bool passed = m_mmPassed.value(e.code, false);
        if (passed) {
            b->setStyleSheet(
                "QPushButton { background-color: rgb(46,160,67); color: white;"
                " border: 1px solid rgb(30,120,50); border-radius: 5px;"
                " padding: 2px 6px; font-weight: 600; }"
                "QPushButton:disabled { color: white; }");
        } else {
            b->setStyleSheet(
                "QPushButton { background-color: rgb(55,40,80); color: rgb(220,200,240);"
                " border: 1px solid rgb(120,80,170); border-radius: 5px;"
                " padding: 2px 6px; }"
                "QPushButton:disabled { color: rgb(220,200,240); }");
        }
        m_mmBarLay->insertWidget(insertAt++, b);
        m_mmBtns.insert(e.code, b);
        ++shown;
    }
    m_mmBar->setVisible(shown > 0);
    if (shown == 0) return;
    appendLog(QString("多媒体栏: 显示 %1 个键").arg(shown));
}

void MainWindow::checkAllKeysTested()
{
    if (m_testReportShown) return;
    if (isInLightTest()) return;

    // 只要求 Fn0 (主层) 全部物理键 + 多媒体栏全部测完, 不要求 Fn1 层每键被按.
    // - 若当前正在 Fn0 视图, view 的状态就是 Fn0; 否则用 m_stateFn0 缓存.
    const int total = m_view->totalTestable();
    int passed = 0;
    if (!m_viewingFn) {
        passed = m_view->passedCount();
    } else {
        for (auto it = m_stateFn0.begin(); it != m_stateFn0.end(); ++it) {
            if (it.value() == static_cast<int>(KeyboardView::Passed))
                ++passed;
        }
    }

    const int mmAll = mmTotal();
    const int mmOk  = mmPassed();

    if (total <= 0) return;
    if (passed < total) return;
    if (mmAll > 0 && mmOk < mmAll) return;

    appendLog("=== Fn0 主层 + 多媒体键全部通过, 进入灯光测试 ===");
    startLightTest();
}

// ---------------- 灯光测试 FSM ----------------

void MainWindow::startLightTest()
{
    m_lightAnyFail = false;
    m_testReportShown = false;
    m_lightStep = LightStep::Red;

    if (m_savedRgb.valid) {
        appendLog("[LightTest] 将在结束后还原至上电时的灯效");
    } else if (m_dev && m_dev->isConnected()) {
        // 兜底: 若连接时未抓到 (例如手动连接前已经在做别的), 现在再试一次
        if (m_dev->readRGBState(&m_savedRgb, 800)) {
            appendLog("[LightTest] 已临时保存当前灯效, 结束后将还原");
        } else {
            appendLog("[LightTest] 警告: 无可用灯效快照, 结束后将退回默认白光");
        }
    }

    if (!m_lightTimer) {
        m_lightTimer = new QTimer(this);
        m_lightTimer->setSingleShot(true);
        connect(m_lightTimer, &QTimer::timeout,
                this, [this]{
                    // 1 秒后进入下一颜色 (advanceLightStep 已先记录用户结果)
                    if (m_lightStep == LightStep::Done) return;
                    // 当前由 timer 触发的 step 已经在 advanceLightStep 中切换;
                    // 这里展示新颜色的灯并提示.
                    showLightPrompt();
                });
    }

    appendLog("--- 进入灯光测试 ---");
    // 直接显示第一种颜色 (红), 并提示 Enter/Esc
    uint32_t c = 0xFFFF0000;
    applyLightColor(c);
    showLightPrompt();
}

void MainWindow::applyLightColor(uint32_t argb)
{
    // 颜色命名 (与 lightStepName 中常用色保持一致)
    auto nameOf = [](uint32_t c)->const char* {
        switch (c) {
        case 0xFFFF0000: return "红色";
        case 0xFF00FF00: return "绿色";
        case 0xFF0000FF: return "蓝色";
        case 0xFFFFFF00: return "黄色";
        case 0xFFFFFFFF: return "白色";
        case 0x00000000: return "黑色 (熄灭)";
        default:         return "自定义";
        }
    };
    appendLog(QString("[LightTest] 下发颜色: %1  (0x%2)")
                  .arg(QString::fromUtf8(nameOf(argb)))
                  .arg(argb, 8, 16, QChar('0')));
    if (!m_dev || !m_dev->isConnected()) {
        appendLog("[LightTest] 警告: 设备未连接, 颜色未实际下发");
        return;
    }
    if (!m_dev->setSolidColor(argb, 500)) {
        appendLog("[LightTest] 错误: setSolidColor 失败 (设备无响应或不支持)");
    }
}

static const char* lightStepName(int s)
{
    switch (s) {
    case 1: return "红色";
    case 2: return "绿色";
    case 3: return "蓝色";
    case 4: return "黄色";
    case 5: return "白色";
    case 6: return "黑色 (熄灭)";
    default: return "?";
    }
}

void MainWindow::showLightPrompt()
{
    const int s = static_cast<int>(m_lightStep);
    if (s < 1 || s > 6) return;
    const QString name = QString::fromUtf8(lightStepName(s));
    const QString promptText =
        QString("灯光测试: 请确认 [%1]   ✔ Space=正确    ✘ Esc=错误").arg(name);
    m_statusKey->setText(promptText);
    m_statusKey->setStyleSheet(
        "QLabel { color: white; font-weight: 700; padding: 2px 8px;"
        " border: 1px solid rgb(200,160,40); border-radius: 4px;"
        " background-color: rgb(140,90,20); }");

    // 当前 step 对应的 ARGB
    uint32_t c = 0xFF000000;
    switch (m_lightStep) {
    case LightStep::Red:    c = 0xFFFF0000; break;
    case LightStep::Green:  c = 0xFF00FF00; break;
    case LightStep::Blue:   c = 0xFF0000FF; break;
    case LightStep::Yellow: c = 0xFFFFFF00; break;
    case LightStep::White:  c = 0xFFFFFFFF; break;
    case LightStep::Black:  c = 0x00000000; break;
    default: break;
    }
    showLightDialog(name, c);
}

void MainWindow::showLightDialog(const QString& name, uint32_t argb)
{
    if (!m_lightDlg) {
        m_lightDlg = new QDialog(this, Qt::Dialog | Qt::FramelessWindowHint
                                       | Qt::WindowStaysOnTopHint);
        m_lightDlg->setModal(false);     // 非模态, KeyHook 仍可收 Enter/Esc
        m_lightDlg->setAttribute(Qt::WA_ShowWithoutActivating, true);
        m_lightDlg->setFocusPolicy(Qt::NoFocus);
        m_lightDlg->setStyleSheet("QDialog { background-color: rgb(20,22,28);"
                                  " border: 3px solid rgb(220,220,230); border-radius: 12px; }");

        auto* lay = new QVBoxLayout(m_lightDlg);
        lay->setContentsMargins(28, 24, 28, 24);
        lay->setSpacing(16);

        m_lightDlgTitle = new QLabel(m_lightDlg);
        m_lightDlgTitle->setAlignment(Qt::AlignCenter);
        m_lightDlgTitle->setStyleSheet(
            "QLabel { color: white; font-size: 28pt; font-weight: 800; }");
        lay->addWidget(m_lightDlgTitle);

        m_lightDlgSwatch = new QLabel(m_lightDlg);
        m_lightDlgSwatch->setFixedSize(420, 180);
        m_lightDlgSwatch->setAlignment(Qt::AlignCenter);
        lay->addWidget(m_lightDlgSwatch, 0, Qt::AlignHCenter);

        m_lightDlgHint = new QLabel(m_lightDlg);
        m_lightDlgHint->setAlignment(Qt::AlignCenter);
        m_lightDlgHint->setTextFormat(Qt::RichText);
        m_lightDlgHint->setStyleSheet(
            "QLabel { color: rgb(230,230,240); font-size: 16pt; font-weight: 600; }");
        m_lightDlgHint->setText(
            "<span style='color:#7CFC8C'>✔ Space = 正确</span>"
            "&nbsp;&nbsp;&nbsp;&nbsp;"
            "<span style='color:#FF8080'>✘ Esc = 错误</span>");
        lay->addWidget(m_lightDlgHint);
    }

    // 文字: "当前颜色: 红色"
    m_lightDlgTitle->setText(QString("当前颜色 : %1").arg(name));

    // 色块: 实际填色; 黑色时显示带边框的深灰说明"熄灭"
    const int r = (argb >> 16) & 0xFF;
    const int g = (argb >> 8)  & 0xFF;
    const int b = (argb)       & 0xFF;
    if (argb == 0x00000000) {
        m_lightDlgSwatch->setText("（已熄灭）");
        m_lightDlgSwatch->setStyleSheet(
            "QLabel { background-color: rgb(8,8,10); color: rgb(180,180,190);"
            " font-size: 22pt; font-weight: 700;"
            " border: 2px dashed rgb(120,120,130); border-radius: 10px; }");
    } else {
        // 浅色 (黄/白) 用黑字, 否则白字
        const int lum = (r * 299 + g * 587 + b * 114) / 1000;
        const QString fg = (lum > 160) ? "black" : "white";
        m_lightDlgSwatch->setText(name);
        m_lightDlgSwatch->setStyleSheet(
            QString("QLabel { background-color: rgb(%1,%2,%3); color: %4;"
                    " font-size: 26pt; font-weight: 800;"
                    " border: 2px solid rgb(80,80,90); border-radius: 10px; }")
                .arg(r).arg(g).arg(b).arg(fg));
    }

    m_lightDlg->adjustSize();
    // 居中到主窗口 (略偏上)
    QRect g0 = this->geometry();
    QSize ds = m_lightDlg->size();
    m_lightDlg->move(g0.center().x() - ds.width()/2,
                     g0.top() + g0.height()/4);
    if (!m_lightDlg->isVisible()) m_lightDlg->show();
    m_lightDlg->raise();
}

void MainWindow::hideLightDialog()
{
    if (m_lightDlg && m_lightDlg->isVisible()) m_lightDlg->hide();
}

void MainWindow::advanceLightStep()
{
    // 进入下一颜色; 中间插入 1 秒间隔
    auto next = [](LightStep s) {
        switch (s) {
        case LightStep::Red:    return LightStep::Green;
        case LightStep::Green:  return LightStep::Blue;
        case LightStep::Blue:   return LightStep::Yellow;
        case LightStep::Yellow: return LightStep::White;
        case LightStep::White:  return LightStep::Black;
        case LightStep::Black:  return LightStep::Done;
        default:                return LightStep::Done;
        }
    };

    m_lightStep = next(m_lightStep);

    if (m_lightStep == LightStep::Done) {
        finishTestSequence();
        return;
    }

    // 暂时熄灭灯, 1 秒后显示下一颜色
    applyLightColor(0x00000000);
    hideLightDialog();
    m_statusKey->setText("灯光测试: 1 秒后显示下一颜色 ...");
    m_statusKey->setStyleSheet(
        "QLabel { color: rgb(200,200,210); font-weight: 600; padding: 2px 8px;"
        " border: 1px solid rgb(80,80,90); border-radius: 4px;"
        " background-color: rgb(40,40,50); }");

    QTimer::singleShot(1000, this, [this]{
        if (m_lightStep == LightStep::Done || m_lightStep == LightStep::None) return;
        uint32_t c = 0xFF000000;
        switch (m_lightStep) {
        case LightStep::Red:    c = 0xFFFF0000; break;
        case LightStep::Green:  c = 0xFF00FF00; break;
        case LightStep::Blue:   c = 0xFF0000FF; break;
        case LightStep::Yellow: c = 0xFFFFFF00; break;
        case LightStep::White:  c = 0xFFFFFFFF; break;
        case LightStep::Black:  c = 0x00000000; break;
        default: break;
        }
        applyLightColor(c);
        showLightPrompt();
    });
}

void MainWindow::finishTestSequence()
{
    m_lightStep = LightStep::Done;
    hideLightDialog();
    // 还原测试前的灯效 (若可用), 否则退回默认白光
    if (m_dev && m_dev->isConnected()) {
        if (m_savedRgb.valid && m_dev->writeRGBState(m_savedRgb, 800)) {
            appendLog("--- 灯光测试结束, 已还原测试前的灯效 ---");
        } else {
            m_dev->restoreDefaultLight(500);
            appendLog("--- 灯光测试结束, 已还原为默认灯光 (未能恢复原始灯效) ---");
        }
    } else {
        appendLog("--- 灯光测试结束 (设备未连接, 无法还原) ---");
    }

    const bool ok = !m_lightAnyFail;
    m_testReportShown = true;

    showBigResult(ok);

    if (ok)
        appendLog("=== 测试成功 ===");
    else
        appendLog("=== 测试失败 ===");
}

void MainWindow::resetTestSequence()
{
    if (m_lightTimer) m_lightTimer->stop();
    m_lightStep   = LightStep::None;
    m_lightAnyFail = false;
    m_testReportShown = false;
    hideLightDialog();
    hideBigResult();
}

void MainWindow::showBigResult(bool ok)
{
    if (!m_bigResult || !m_view) return;
    m_bigResult->setText(ok ? "✓  测试成功" : "✗  测试失败");
    if (ok) {
        m_bigResult->setStyleSheet(
            "QLabel { background-color: rgba(20,120,50,230); color: white;"
            " font-size: 56px; font-weight: 900; border-radius: 10px;"
            " padding: 30px 60px; border: 3px solid rgb(60,200,90); }");
    } else {
        m_bigResult->setStyleSheet(
            "QLabel { background-color: rgba(180,30,30,235); color: white;"
            " font-size: 60px; font-weight: 900; border-radius: 10px;"
            " padding: 30px 60px; border: 3px solid rgb(255,80,80); }");
    }
    m_bigResult->adjustSize();
    const QSize sz = m_bigResult->sizeHint();
    const QRect r = m_view->rect();
    m_bigResult->setGeometry(r.center().x() - sz.width()/2,
                             r.center().y() - sz.height()/2,
                             sz.width(), sz.height());
    m_bigResult->raise();
    m_bigResult->show();

    // 点击键盘视图任意处或 6 秒后自动隐藏
    QTimer::singleShot(6000, this, [this]{ hideBigResult(); });
}

void MainWindow::hideBigResult()
{
    if (m_bigResult) m_bigResult->hide();
}

// ===========================================================================
//  Server / API
// ===========================================================================
QString MainWindow::cachedLayoutPath(uint16_t vid, uint16_t pid) const
{
    const QString key = QString("0x%1-0x%2")
        .arg(vid, 4, 16, QChar('0'))
        .arg(pid, 4, 16, QChar('0')).toUpper();
    const QString dir = QCoreApplication::applicationDirPath()
                        + "/cache/layouts/" + key;
    QDir d(dir);
    if (!d.exists()) return QString();
    const auto names = d.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    if (names.isEmpty()) return QString();
    return d.absoluteFilePath(names.first());
}

bool MainWindow::fetchLayoutFromServer(uint16_t vid, uint16_t pid)
{
    if (!m_api || !m_api->isLoggedIn()) return false;

    QByteArray body;
    QString filename, err;
    if (!m_api->fetchLayout(vid, pid, &body, &filename, &err)) {
        appendLog(QString("[Server] 拉取 0x%1-0x%2 失败: %3")
                  .arg(vid, 4, 16, QChar('0'))
                  .arg(pid, 4, 16, QChar('0'))
                  .arg(err));
        return false;
    }
    // Sanitize filename — strip any directory part the server might have included.
    QString safe = QFileInfo(filename).fileName();
    if (safe.isEmpty()) safe = "layout.c";

    const QString key = QString("0x%1-0x%2")
        .arg(vid, 4, 16, QChar('0'))
        .arg(pid, 4, 16, QChar('0')).toUpper();
    const QString dirPath = QCoreApplication::applicationDirPath()
                            + "/cache/layouts/" + key;
    QDir().mkpath(dirPath);

    // Clear stale files so listing returns just this one.
    QDir d(dirPath);
    for (const QString& fn : d.entryList(QDir::Files | QDir::NoDotAndDotDot)) {
        if (fn != safe) d.remove(fn);
    }

    const QString abs = dirPath + "/" + safe;
    QFile f(abs);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        appendLog("[Server] 缓存写入失败: " + abs);
        return false;
    }
    f.write(body);
    f.close();
    appendLog(QString("[Server] 已同步配列 %1 (%2 bytes)").arg(safe).arg(body.size()));

    // If this VID-PID is currently selected, reload immediately.
    if (m_vidpid) {
        const QStringList parts = m_vidpid->currentText().split('-');
        if (parts.size() == 2) {
            const uint16_t cv = parts[0].toUShort(nullptr, 16);
            const uint16_t cp = parts[1].toUShort(nullptr, 16);
            if (cv == vid && cp == pid) {
                if (m_lastLayoutPath != abs) loadLayoutFile(abs);
            }
        }
    }
    return true;
}

void MainWindow::updateServerMenuLabels()
{
    if (!m_actSrvLogin) return;
    if (m_api && m_api->isLoggedIn()) {
        m_actSrvLogin->setText(QString("已登录: %1 (%2)")
                               .arg(m_api->username(), m_api->role()));
        m_actSrvLogin->setEnabled(false);
        m_actSrvLogout->setEnabled(true);
        m_actSrvSync->setEnabled(true);
    } else {
        m_actSrvLogin->setText("登录…");
        m_actSrvLogin->setEnabled(true);
        m_actSrvLogout->setEnabled(false);
        m_actSrvSync->setEnabled(false);
    }
}

void MainWindow::onServerLogin()
{
    if (!m_api) return;
    LoginDialog dlg(m_api, this);
    if (dlg.exec() == QDialog::Accepted) {
        appendLog(QString("[Server] 已登录: %1 (%2)")
                  .arg(m_api->username(), m_api->role()));
        updateServerMenuLabels();
        // Try to refresh layout for current VID/PID right away.
        if (m_vidpid) onVidPidChanged(m_vidpid->currentText());
    }
}

void MainWindow::onServerLogout()
{
    if (!m_api || !m_api->isLoggedIn()) return;
    if (QMessageBox::question(this, "注销",
            QString("确认注销账号 %1？").arg(m_api->username()))
        != QMessageBox::Yes) return;
    m_api->clearAuth();
    appendLog("[Server] 已注销");
    updateServerMenuLabels();
}

void MainWindow::onServerAddress()
{
    if (!m_api) return;
    bool ok = false;
    const QString cur = m_api->baseUrl();
    const QString v = QInputDialog::getText(this, "服务器地址",
        "示例: https://ktester.hallsdk.com",
        QLineEdit::Normal, cur, &ok);
    if (!ok || v.trimmed().isEmpty()) return;
    m_api->setBaseUrl(v.trimmed());
    appendLog(QString("[Server] 服务器地址已设置为 %1").arg(m_api->baseUrl()));
}

void MainWindow::onServerSyncList()
{
    if (!m_api || !m_api->isLoggedIn()) {
        appendLog("[Server] 请先登录");
        return;
    }
    QList<ApiDevice> list;
    QString err;
    if (!m_api->listDevices(&list, &err)) {
        appendLog("[Server] 同步设备列表失败: " + err);
        QMessageBox::warning(this, "同步设备列表", err);
        return;
    }
    appendLog(QString("[Server] 服务器共 %1 个设备, 开始同步配列…").arg(list.size()));
    int ok = 0;
    for (const auto& d : list) {
        bool okv=false, okp=false;
        const uint16_t v = d.vid.mid(2).toUShort(&okv, 16);
        const uint16_t p = d.pid.mid(2).toUShort(&okp, 16);
        if (!okv || !okp) continue;
        if (fetchLayoutFromServer(v, p)) ++ok;
    }
    appendLog(QString("[Server] 同步完成: %1 / %2 成功").arg(ok).arg(list.size()));
    QMessageBox::information(this, "同步设备列表",
        QString("已同步 %1 / %2 个设备的配列到本地缓存").arg(ok).arg(list.size()));
}
