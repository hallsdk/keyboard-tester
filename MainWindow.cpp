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
#include <QDebug>

#include "KeyboardGrid.h"
#include "LayoutConfig.h"
#include "DeviceManager.h"
#include "KeyHook.h"

extern "C" {
#include "Layout/HID_Usage_Tables.h"
#include "Layout/OHID_Layout.h"
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Keyboard Tester V2");
    resize(1100, 560);
    setMinimumSize(640, 360);          // 允许自由拉伸, 但给一个最小尺寸

    buildUi();

    // 初始按默认布局渲染一次 (无设备), 这样开机就有个键盘可看.
    LayoutConfig::load();
    applyLayoutForBoard(0);

    // Device manager
    m_dev = new DeviceManager(this);
    connect(m_dev, &DeviceManager::connected,
            this, &MainWindow::onDeviceConnected);
    connect(m_dev, &DeviceManager::disconnected,
            this, &MainWindow::onDeviceDisconnected);

    // Global keyboard hook (on-demand; can also install at startup)
    m_hook = new KeyHook(this);
    connect(m_hook, &KeyHook::keyEvent,
            this, &MainWindow::onGlobalKey,
            Qt::QueuedConnection);
    m_hook->install();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* rootV = new QVBoxLayout(central);
    rootV->setContentsMargins(6, 6, 6, 6);
    rootV->setSpacing(6);

    // ---- Top toolbar ----
    auto* toolH = new QHBoxLayout();
    toolH->setSpacing(6);

    toolH->addWidget(new QLabel("VID-PID:", central));
    m_vidpid = new QComboBox(central);
    m_vidpid->addItem("0x36F9-0xAB05");
    m_vidpid->addItem("0x36F9-0xAB06");
    
    m_vidpid->addItem("0x2E3C-0x5745");
    toolH->addWidget(m_vidpid);

    m_btnConn = new QPushButton("连接", central);
    m_btnDisc = new QPushButton("断开", central);
    m_btnReload = new QPushButton("重载 config", central);
    m_btnDisc->setEnabled(false);
    toolH->addWidget(m_btnConn);
    toolH->addWidget(m_btnDisc);
    toolH->addWidget(m_btnReload);

    toolH->addStretch(1);

    m_statusBoard = new QLabel("未连接", central);
    m_statusBoard->setStyleSheet("color: rgb(200,200,200);");
    toolH->addWidget(m_statusBoard);

    rootV->addLayout(toolH);

    // ---- Splitter: Keyboard grid (top) + Log (bottom) ----
    auto* split = new QSplitter(Qt::Vertical, central);
    split->setChildrenCollapsible(false);

    auto* gridHost = new QWidget(split);
    gridHost->setStyleSheet("background-color: rgb(20,20,20);");
    auto* gridHostV = new QVBoxLayout(gridHost);
    gridHostV->setContentsMargins(0, 0, 0, 0);
    m_grid = new KeyboardGrid(gridHost);
    gridHostV->addWidget(m_grid);

    m_log = new QPlainTextEdit(split);
    m_log->setReadOnly(true);
    m_log->setMaximumBlockCount(2000);
    m_log->setStyleSheet("background-color: rgb(25,25,25); color: rgb(200,200,200); font-family: Consolas;");

    split->addWidget(gridHost);
    split->addWidget(m_log);
    split->setStretchFactor(0, 4);
    split->setStretchFactor(1, 1);

    rootV->addWidget(split, 1);

    // ---- Signals ----
    connect(m_btnConn,   &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_btnDisc,   &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(m_btnReload, &QPushButton::clicked, this, &MainWindow::onReloadConfigClicked);

    // Dark theme for central
    central->setStyleSheet("QWidget { background-color: rgb(35,35,35); color: rgb(220,220,220); }"
                           "QPushButton { background-color: rgb(60,60,60); padding: 4px 10px; border: 1px solid rgb(80,80,80); border-radius: 3px; }"
                           "QPushButton:hover { background-color: rgb(80,80,80); }"
                           "QComboBox { background-color: rgb(60,60,60); padding: 2px 6px; }");
}

void MainWindow::applyLayoutForBoard(uint32_t board_id)
{
    m_currentBoardId = board_id;
    m_grid->applyLayout(board_id);
    rebuildScanToPosMap(board_id);
    appendLog(QString("已应用布局: board_id=0x%1 (有效按键数=%2)")
              .arg(board_id, 8, 16, QChar('0'))
              .arg(m_scanToPos.size()));
}

// 复制自旧工程逻辑: 构建 scanCode → (row,col), 仅注册当前布局矩阵中存在的 driverCode.
void MainWindow::rebuildScanToPosMap(uint32_t board_id)
{
    m_scanToPos.clear();

    const tkb_Half_matrix_t* mat = LayoutConfig::resolveMatrix(board_id);
    if (!mat) return;

    QMap<uint16_t, QPair<int,int>> driverToPos;
    for (int r = 0; r < TKB_ROWS; ++r)
        for (int c = 0; c < TKB_COLS; ++c)
            if (mat->Matrix[r][c] != (uint16_t)xxSK)
                driverToPos[mat->Matrix[r][c]] = {r, c};

    auto addPos = [&](int scanCode, uint16_t driverCode) {
        auto it = driverToPos.find(driverCode);
        if (it != driverToPos.end())
            m_scanToPos.insert(scanCode, it.value());
    };

    const int EXT = 0x100;
    // Standard
    addPos(0x02, KC_1);  addPos(0x03, KC_2);  addPos(0x04, KC_3);  addPos(0x05, KC_4);
    addPos(0x06, KC_5);  addPos(0x07, KC_6);  addPos(0x08, KC_7);  addPos(0x09, KC_8);
    addPos(0x0A, KC_9);  addPos(0x0B, KC_0);  addPos(0x0C, KC_MINS); addPos(0x0D, KC_EQUAL);
    addPos(0x10, KC_Q);  addPos(0x11, KC_W);  addPos(0x12, KC_E);  addPos(0x13, KC_R);
    addPos(0x14, KC_T);  addPos(0x15, KC_Y);  addPos(0x16, KC_U);  addPos(0x17, KC_I);
    addPos(0x18, KC_O);  addPos(0x19, KC_P);
    addPos(0x1A, KC_LEFT_BRACKET); addPos(0x1B, KC_RIGHT_BRACKET);
    addPos(0x1E, KC_A);  addPos(0x1F, KC_S);  addPos(0x20, KC_D);  addPos(0x21, KC_F);
    addPos(0x22, KC_G);  addPos(0x23, KC_H);  addPos(0x24, KC_J);  addPos(0x25, KC_K);
    addPos(0x26, KC_L);  addPos(0x27, KC_SEMICOLON); addPos(0x28, KC_QUOTE);
    addPos(0x29, KC_GRAVE); addPos(0x2B, KC_BACKSLASH);
    addPos(0x2C, KC_Z);  addPos(0x2D, KC_X);  addPos(0x2E, KC_C);  addPos(0x2F, KC_V);
    addPos(0x30, KC_B);  addPos(0x31, KC_N);  addPos(0x32, KC_M);
    addPos(0x33, KC_COMMA); addPos(0x34, KC_DOT); addPos(0x35, KC_SLASH);
    addPos(0x39, KC_SPACE);
    addPos(0x3B, KC_F1); addPos(0x3C, KC_F2); addPos(0x3D, KC_F3); addPos(0x3E, KC_F4);
    addPos(0x3F, KC_F5); addPos(0x40, KC_F6); addPos(0x41, KC_F7); addPos(0x42, KC_F8);
    addPos(0x43, KC_F9); addPos(0x44, KC_F10); addPos(0x57, KC_F11); addPos(0x58, KC_F12);
    // Numpad
    addPos(0x47, KC_KP_7); addPos(0x48, KC_KP_8); addPos(0x49, KC_KP_9);
    addPos(0x4B, KC_KP_4); addPos(0x4C, KC_KP_5); addPos(0x4D, KC_KP_6);
    addPos(0x4F, KC_KP_1); addPos(0x50, KC_KP_2); addPos(0x51, KC_KP_3);
    addPos(0x52, KC_KP_0); addPos(0x53, KC_KP_DOT); addPos(0x37, KC_KP_ASTERISK);
    // Control
    addPos(0x01, KC_ESC);       addPos(0x0E, KC_BACKSPACE); addPos(0x0F, KC_TAB);
    addPos(0x1C, KC_ENTER);     addPos(0x1D, KC_LCTL);
    addPos(0x2A, KC_LSFT);      addPos(0x36, KC_RSFT);
    addPos(0x38, KC_LALT);      addPos(0x3A, KC_CAPS);
    addPos(0x45, KC_PAUSE);     addPos(0x46, KC_SCROLL_LOCK);
    addPos(0x4A, KC_KP_MINUS);  addPos(0x4E, KC_KP_PLUS);
    // Extended
    addPos(0x52 + EXT, KC_INSERT);     addPos(0x53 + EXT, KC_DELETE);
    addPos(0x47 + EXT, KC_HOME);       addPos(0x4F + EXT, KC_END);
    addPos(0x49 + EXT, KC_PAGE_UP);    addPos(0x51 + EXT, KC_PAGE_DOWN);
    addPos(0x48 + EXT, KC_UP);         addPos(0x50 + EXT, KC_DOWN);
    addPos(0x4B + EXT, KC_LEFT);       addPos(0x4D + EXT, KC_RIGHT);
    addPos(0x1C + EXT, KC_KP_ENTER);   addPos(0x1D + EXT, KC_RCTL);
    addPos(0x35 + EXT, KC_KP_SLASH);   addPos(0x37 + EXT, KC_PRINT_SCREEN);
    addPos(0x38 + EXT, KC_RALT);       addPos(0x45 + EXT, KC_NUM_LOCK);
    addPos(0x5B + EXT, KC_LGUI);       addPos(0x5C + EXT, KC_RGUI);
    addPos(0x5D + EXT, KC_MENU);
}

void MainWindow::appendLog(const QString& msg)
{
    const QString ts = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_log->appendPlainText(QString("[%1] %2").arg(ts, msg));
}

// ------------- Slots -------------
void MainWindow::onConnectClicked()
{
    const QString text = m_vidpid->currentText();            // e.g. "0x2E3C-0x5745"
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

    appendLog(QString("尝试连接 VID=0x%1 PID=0x%2 ...")
              .arg(vid, 4, 16, QChar('0'))
              .arg(pid, 4, 16, QChar('0')));
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

void MainWindow::onReloadConfigClicked()
{
    LayoutConfig::load();
    applyLayoutForBoard(m_currentBoardId);
    appendLog("config.json 已重载");
}

void MainWindow::onDeviceConnected(uint32_t board_id)
{
    const auto& info = m_dev->info();
    m_statusBoard->setText(QString("已连接: board_id=0x%1  fw=%2")
                           .arg(board_id, 8, 16, QChar('0'))
                           .arg(QString::fromLatin1((const char*)info.Version).trimmed()));
    appendLog(QString("已连接键盘 board_id=0x%1 (如需自定义布局请在 config.json 的 Layouts 中添加此 id)")
              .arg(board_id, 8, 16, QChar('0')));
    applyLayoutForBoard(board_id);
}

void MainWindow::onDeviceDisconnected()
{
    m_btnConn->setEnabled(true);
    m_btnDisc->setEnabled(false);
    m_statusBoard->setText("未连接");
    appendLog("设备已断开");
}

void MainWindow::onGlobalKey(int scanCode, bool pressed)
{
    if (scanCode > 0x200) return;  // 滤除异常叠加值

    auto it = m_scanToPos.find(scanCode);
    if (it == m_scanToPos.end()) {
        if (pressed) appendLog(QString("未映射按键 scan=0x%1").arg(scanCode, 0, 16));
        return;
    }
    QPushButton* btn = m_grid->getButton(it.value().first, it.value().second);
    if (!btn) return;

    if (pressed) {
        btn->setStyleSheet(KeyboardGrid::pressedButtonStyle());
    } else {
        btn->setStyleSheet(KeyboardGrid::defaultButtonStyle());
    }
}
