#include "DeviceManager.h"

#include <QDateTime>
#include <QDebug>
#include <cstring>

quint64 DeviceManager::nowMs()
{
    return (quint64)QDateTime::currentMSecsSinceEpoch();
}

DeviceManager::DeviceManager(QObject* parent) : QObject(parent)
{
    std::memset(&m_pack, 0, sizeof(m_pack));
}

DeviceManager::~DeviceManager()
{
    m_hid.close();
}

// Returns false if the path ends with the standard HID keyboard class suffix
// (\kbd). Windows blocks raw hid_write on those interfaces.
static bool isRawHidPath(const char* path)
{
    const size_t n = std::strlen(path);
    if (n < 4) return false;
    return std::strcmp(path + n - 4, "\\kbd") != 0;
}

int DeviceManager::scanAndOpen(uint16_t vid, uint16_t pid)
{
    m_pathList.init();
    m_hid.close();
    m_hid.Init(m_pathList, vid, pid);
    qDebug("[DeviceManager] scan VID=0x%04X PID=0x%04X found=%d", vid, pid, m_pathList.size());

    if (m_pathList.size() <= 0) return -1;
    return 0;
}

bool DeviceManager::openNextPath(int& startIdx)
{
    char path[256 + 4];
    for (int i = startIdx; i < m_pathList.size(); ++i) {
        std::memset(path, 0, sizeof(path));
        m_pathList.pull(i, path, sizeof(path));
        qDebug("[DeviceManager]   path[%d]=%s", i, path);
        const size_t plen = std::strlen(path);
        if (plen < 20) continue;
        if (!isRawHidPath(path)) {
            qDebug("[DeviceManager]   path[%d] skipped (\\kbd interface)", i);
            continue;
        }
        if (m_hid.Open(path) == 0) {
            qDebug("[DeviceManager] opened path[%d]", i);
            startIdx = i + 1;
            return true;
        }
    }
    return false;
}

int DeviceManager::syncBoard(uint32_t timeoutMs)
{
    const quint64 deadline = nowMs() + timeoutMs;
    int len = -1;
    uint16_t size = 0;

    while (nowMs() < deadline) {
        m_hid.clear();
        OHIDM_None(&m_pack, OHID_CMD_BASE_SYNC);
        int res = m_hid.Writes(m_pack.bin, OHID_pack_size(m_pack));
        if (res < 0) {
            qWarning("[DeviceManager] Writes failed");
            return -1;
        }
        std::memset(m_pack.bin, 0, sizeof(m_pack.bin));

        size = m_hid.Reads(m_pack.bin, 64, 100);
        if (size > 4) len = _OHIDS_Decode(&m_pack, size);
        if (len < (OHID_HEAD_SIZE + 20)) continue;

        std::memset(m_info.SN, 0, sizeof(m_info.SN));
        std::memset(m_info.Version, 0, sizeof(m_info.Version));

        m_info.board_id = OHID_get_32b(&m_pack, 0);
        m_info.fwSize   = ((uint32_t)OHID_get_16b(&m_pack, 4)) << 8;
        m_info.Mode     = OHID_get_8b(&m_pack, 6);
        OHID_get_bin(&m_pack, 7,    m_info.SN,      sizeof(m_info.SN));
        OHID_get_bin(&m_pack, 7+17, m_info.Version, sizeof(m_info.Version));

        if (len >= 4 && m_pack.ohid.cmd == OHID_CMD_BASE_SYNC) {
            qDebug("[DeviceManager] SYNC OK: board_id=0x%08X fwSize=%u mode=0x%02X",
                   (uint32_t)m_info.board_id, m_info.fwSize, m_info.Mode);
            return 0;
        }
    }
    return -3;
}

bool DeviceManager::tryConnect(uint16_t vid, uint16_t pid, uint32_t timeoutMs)
{
    if (scanAndOpen(vid, pid) != 0) return false;

    int idx = 0;
    while (openNextPath(idx)) {
        if (syncBoard(timeoutMs) == 0) {
            m_connected = true;
            emit connected(m_info.board_id);
            return true;
        }
        m_hid.close();
    }
    return false;
}

void DeviceManager::disconnect()
{
    if (!m_connected) return;
    m_hid.close();
    m_connected = false;
    emit disconnected();
}
