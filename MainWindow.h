#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QSet>
#include <cstdint>

class QComboBox;
class QPushButton;
class QLabel;
class QPlainTextEdit;

class KeyboardGrid;
class DeviceManager;
class KeyHook;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onReloadConfigClicked();
    void onDeviceConnected(uint32_t board_id);
    void onDeviceDisconnected();
    void onGlobalKey(int scanCode, bool pressed);

private:
    void buildUi();
    void applyLayoutForBoard(uint32_t board_id);
    void rebuildScanToPosMap(uint32_t board_id);
    void appendLog(const QString& msg);

    // UI
    KeyboardGrid*   m_grid      = nullptr;
    QComboBox*      m_vidpid    = nullptr;
    QPushButton*    m_btnConn   = nullptr;
    QPushButton*    m_btnDisc   = nullptr;
    QPushButton*    m_btnReload = nullptr;
    QLabel*         m_statusBoard = nullptr;
    QPlainTextEdit* m_log       = nullptr;

    // Device / hook
    DeviceManager*  m_dev  = nullptr;
    KeyHook*        m_hook = nullptr;

    // scan code → (row, col) of the current layout (主布局)
    QMap<int, QPair<int,int>> m_scanToPos;

    uint32_t m_currentBoardId = 0;
};

#endif // MAINWINDOW_H
