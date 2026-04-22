#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QString>
#include <cstdint>

#include "hid/InterfaceHID.h"
#include "OHID.h"
#include "OHID/OHID_Pack.h"
#include "OHID/OHID_KeyBoard.h"

/**
 * DeviceManager
 *  - 搜索/打开匹配 VID/PID 的 HID 键盘, 执行 OHID 同步, 读取 board_id 等信息.
 *  - 成功时发射 connected(board_id); 断开发射 disconnected().
 *  - 设计为阻塞扫描, 由调用方在后台任务或短时主线程同步中触发.
 */
class DeviceManager : public QObject
{
    Q_OBJECT
public:
    struct BoardInfo {
        uint32_t board_id = 0;
        uint8_t  SN[32]   = {0};
        uint8_t  Version[32] = {0};
        uint32_t fwSize   = 0;
        uint8_t  Mode     = 0;
    };

    explicit DeviceManager(QObject* parent = nullptr);
    ~DeviceManager() override;

    // 尝试连接 vid/pid 指定的设备. 成功时返回 true 并通过 emit connected() 通知.
    bool tryConnect(uint16_t vid, uint16_t pid, uint32_t timeoutMs = 3000);
    void disconnect();

    bool isConnected() const { return m_connected; }
    const BoardInfo& info() const { return m_info; }

signals:
    void connected(uint32_t board_id);
    void disconnected();

private:
    int  scanAndOpen(uint16_t vid, uint16_t pid);  // populates m_pathList
    bool openNextPath(int& startIdx);              // opens next raw-HID candidate path
    int  syncBoard(uint32_t timeoutMs);

    InterfaceHID            m_hid;
    InterfaceList           m_pathList;
    union OpenAgreementHID_t m_pack;
    BoardInfo               m_info;
    bool                    m_connected = false;
    quint64                 m_deadlineMs = 0;

    static quint64 nowMs();
};

#endif // DEVICEMANAGER_H
