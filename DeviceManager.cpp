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

// ---------------------------------------------------------------------------
//  readParamPage — batch-read OHID_CMD_DRIVER_PARAM
//
//  Wire format (cmd 0x10, slave response):
//     [MKEY[2B] | Item[1B] | Value[2B]]  × GROUP_LEN_PARAM (=12)
//
//  We send a read request (no WRITE bit) with the same MKEY/Item layout
//  and the device fills in the Value field.
// ---------------------------------------------------------------------------
bool DeviceManager::readParamPage(uint8_t page,
                                  const QVector<uint16_t>& mkeys,
                                  QHash<uint16_t, uint16_t>* out,
                                  uint32_t perBatchTimeoutMs)
{
    if (!m_connected) return false;
    if (!out) return false;
    out->clear();
    if (mkeys.isEmpty()) return true;

    constexpr int N = 12;          // GROUP_LEN_PARAM
    uint16_t mk[N];
    enum OHID_PARAM_PAGE it[N];
    uint16_t vl[N];

    for (int base = 0; base < mkeys.size(); base += N) {
        // Fill the 12-slot request. Unused slots get MKEY=0xFFFF (sentinel).
        for (int i = 0; i < N; ++i) {
            if (base + i < mkeys.size()) {
                mk[i] = mkeys[base + i];
                it[i] = (enum OHID_PARAM_PAGE)page;
                vl[i] = 0xFFFF;
            } else {
                mk[i] = 0xFFFF;
                it[i] = (enum OHID_PARAM_PAGE)0xFF;
                vl[i] = 0xFFFF;
            }
        }

        std::memset(&m_pack, 0, sizeof(m_pack));
        OHIDM_Driver_Param(&m_pack, /*write*/0, mk, it, vl);

        m_hid.clear();
        int res = m_hid.Writes(m_pack.bin, OHID_pack_size(m_pack));
        if (res < 0) {
            qWarning("[DeviceManager] PARAM Writes failed");
            return false;
        }

        // Wait for matching response.
        const quint64 deadline = nowMs() + perBatchTimeoutMs;
        bool gotBatch = false;
        while (nowMs() < deadline) {
            std::memset(m_pack.bin, 0, sizeof(m_pack.bin));
            uint16_t size = m_hid.Reads(m_pack.bin, 64, 50);
            if (size <= 4) continue;
            int len = _OHIDS_Decode(&m_pack, size);
            if (len < OHID_HEAD_SIZE) continue;
            if (m_pack.ohid.cmd != OHID_CMD_DRIVER_PARAM) continue;

            // Parse 12 × 5-byte entries from offset 0.
            for (int i = 0; i < N; ++i) {
                const uint8_t off = (uint8_t)(i * 5);
                const uint16_t key  = OHID_get_16b(&m_pack, off);
                const uint8_t  item = OHID_get_8b (&m_pack, off + 2);
                const uint16_t val  = OHID_get_16b(&m_pack, off + 3);
                if (key == 0xFFFF) continue;
                if (item != page)   continue;
                out->insert(key, val);
            }
            gotBatch = true;
            break;
        }
        if (!gotBatch) {
            qWarning("[DeviceManager] PARAM batch timeout at base=%d", base);
            return false;
        }
    }
    return true;
}


// ===========================================================================
//  �ƹ���� (������ɫ / Ĭ�ϻ�ԭ)
//
//  ԭ�� (�� KeyboardFactoryTester::write_RGB_KEYS2 һ��):
//      �� MIX_RGB_COLOR  palette[8] = { color, ... }
//      �� MIX_RGB_IDX    idx = 0       (ʹ�� palette[0])
//      �� MIX_RGB_MODE   mode = RGB_MODE_LIGHT  (����)
//  ÿ�����������дһ֡ + ��һ֡ OHID_CMD_BASE_MIX ��Ӧ.
// ===========================================================================

static bool mix_word_blocking(InterfaceHID& hid, OpenAgreementHID_t& pack,
                              OHID_MIX_ORDER order, uint32_t* word, uint8_t word_len,
                              uint32_t timeoutMs)
{
    if (word_len > 14) word_len = 14;
    const quint64 deadline = (quint64)QDateTime::currentMSecsSinceEpoch() + timeoutMs;
    while ((quint64)QDateTime::currentMSecsSinceEpoch() < deadline) {
        hid.clear();
        std::memset(&pack, 0, sizeof(pack));
        OHIDM_Base_mix_word(&pack, /*write*/1, order, word, word_len);
        int res = hid.Writes(pack.bin, OHID_pack_size(pack));
        if (res < 0) return false;

        std::memset(pack.bin, 0, sizeof(pack.bin));
        uint16_t size = hid.Reads(pack.bin, 64, 100);
        if (size <= 4) continue;
        int len = _OHIDS_Decode(&pack, size);
        if (len < OHID_HEAD_SIZE) continue;
        if (pack.ohid.cmd != OHID_CMD_BASE_MIX) continue;
        return true;
    }
    return false;
}

static bool mix_arg_blocking(InterfaceHID& hid, OpenAgreementHID_t& pack,
                             OHID_MIX_ORDER order, uint8_t arg, uint32_t timeoutMs)
{
    const quint64 deadline = (quint64)QDateTime::currentMSecsSinceEpoch() + timeoutMs;
    while ((quint64)QDateTime::currentMSecsSinceEpoch() < deadline) {
        hid.clear();
        std::memset(&pack, 0, sizeof(pack));
        OHIDM_Base_mix_arg(&pack, /*write*/1, order, arg);
        int res = hid.Writes(pack.bin, OHID_pack_size(pack));
        if (res < 0) return false;

        std::memset(pack.bin, 0, sizeof(pack.bin));
        uint16_t size = hid.Reads(pack.bin, 64, 100);
        if (size <= 4) continue;
        int len = _OHIDS_Decode(&pack, size);
        if (len < OHID_HEAD_SIZE) continue;
        if (pack.ohid.cmd != OHID_CMD_BASE_MIX) continue;
        return true;
    }
    return false;
}

bool DeviceManager::setSolidColor(uint32_t argb, uint32_t timeoutMs)
{
    if (!m_connected) return false;
    const uint32_t per = timeoutMs / 3 + 50;

    uint32_t palette[8];
    palette[0] = argb;
    for (int i = 1; i < 8; ++i) palette[i] = 0;

    if (!mix_word_blocking(m_hid, m_pack, OHID_MIX_RGB_COLOR, palette, 8, per)) {
        qWarning("[DeviceManager] setSolidColor: MIX_RGB_COLOR failed");
        return false;
    }
    if (!mix_arg_blocking(m_hid, m_pack, OHID_MIX_RGB_IDX, /*idx=*/0, per)) {
        qWarning("[DeviceManager] setSolidColor: MIX_RGB_IDX failed");
        return false;
    }
    if (!mix_arg_blocking(m_hid, m_pack, OHID_MIX_RGB_MODE,
                          (uint8_t)RGB_MODE_LIGHT, per)) {
        qWarning("[DeviceManager] setSolidColor: MIX_RGB_MODE failed");
        return false;
    }
    return true;
}

bool DeviceManager::restoreDefaultLight(uint32_t timeoutMs)
{
    // ��ԭ: �����׹�. ����"����Ĭ��"Ӧ���ͺž���, �����һ����ȫ�ɼ�ֵ.
    return setSolidColor(0xFFFFFFFF, timeoutMs);
}


// ===========================================================================
//  RGB ȫ������ ��/д (���ڲ���ǰ�󱣴�/�ָ���Ч)
//  ��Ӧ KeyboardFactoryTester::OHIDRGB().
// ===========================================================================

bool DeviceManager::readRGBState(RGBState* out, uint32_t timeoutMs)
{
    if (!m_connected || !out) return false;
    const quint64 deadline = nowMs() + timeoutMs;

    uint32_t back = 0;
    uint32_t palettes[8] = {0,0,0,0,0,0,0,0};
    uint8_t gray=0, mode=0, speed=0, sleep=0, on=0, on_sleep=0, reverse=0;

    while (nowMs() < deadline) {
        m_hid.clear();
        std::memset(&m_pack, 0, sizeof(m_pack));
        OHIDM_Driver_RGB_PARAM(&m_pack, /*write*/0,
                               back, palettes,
                               gray, mode, speed, sleep, on, on_sleep, reverse);
        int res = m_hid.Writes(m_pack.bin, OHID_pack_size(m_pack));
        if (res < 0) { qWarning("[DeviceManager] readRGBState write failed"); return false; }

        std::memset(m_pack.bin, 0, sizeof(m_pack.bin));
        uint16_t size = m_hid.Reads(m_pack.bin, 64, 150);
        if (size <= 4) continue;
        int len = _OHIDS_Decode(&m_pack, size);
        if (len < OHID_HEAD_SIZE) continue;
        if (m_pack.ohid.cmd != OHID_CMD_DRIVER_RGB_PARAM) continue;

        out->back = OHID_get_32b(&m_pack, 0);
        for (int i = 0; i < 8; ++i)
            out->palettes[i] = OHID_get_32b(&m_pack, 4 + 4*i);
        out->gray     = OHID_get_8b(&m_pack, 4 + 4*8 + 0);
        out->mode     = OHID_get_8b(&m_pack, 4 + 4*8 + 1);
        out->speed    = OHID_get_8b(&m_pack, 4 + 4*8 + 2);
        out->sleep    = OHID_get_8b(&m_pack, 4 + 4*8 + 3);
        out->on       = OHID_get_8b(&m_pack, 4 + 4*8 + 4);
        out->on_sleep = OHID_get_8b(&m_pack, 4 + 4*8 + 5);
        out->reverse  = OHID_get_8b(&m_pack, 4 + 4*8 + 6);
        out->valid = true;
        return true;
    }
    qWarning("[DeviceManager] readRGBState timeout");
    return false;
}

bool DeviceManager::writeRGBState(const RGBState& st, uint32_t timeoutMs)
{
    if (!m_connected || !st.valid) return false;
    // Э��Ҫ�󴫿��޸ĵ�����; ����һ�ݵ����ر���.
    uint32_t back = st.back;
    uint32_t palettes[8];
    for (int i = 0; i < 8; ++i) palettes[i] = st.palettes[i];
    uint8_t gray=st.gray, mode=st.mode, speed=st.speed,
            sleep=st.sleep, on=st.on, on_sleep=st.on_sleep, reverse=st.reverse;

    const quint64 deadline = nowMs() + timeoutMs;
    while (nowMs() < deadline) {
        m_hid.clear();
        std::memset(&m_pack, 0, sizeof(m_pack));
        OHIDM_Driver_RGB_PARAM(&m_pack, /*write*/1,
                               back, palettes,
                               gray, mode, speed, sleep, on, on_sleep, reverse);
        int res = m_hid.Writes(m_pack.bin, OHID_pack_size(m_pack));
        if (res < 0) return false;

        std::memset(m_pack.bin, 0, sizeof(m_pack.bin));
        uint16_t size = m_hid.Reads(m_pack.bin, 64, 150);
        if (size <= 4) continue;
        int len = _OHIDS_Decode(&m_pack, size);
        if (len < OHID_HEAD_SIZE) continue;
        if (m_pack.ohid.cmd != OHID_CMD_DRIVER_RGB_PARAM) continue;
        return true;
    }
    qWarning("[DeviceManager] writeRGBState timeout");
    return false;
}
