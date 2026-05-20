#include "DeviceManager.h"

#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <cstring>

extern "C" {
#include "Layout/HID_Usage_Tables.h"
#include "Layout/OHID_Layout.h"
}

quint64 DeviceManager::nowMs()
{
    return (quint64)QDateTime::currentMSecsSinceEpoch();
}

DeviceManager::DeviceManager(QObject* parent) : QObject(parent)
{
    std::memset(&m_pack, 0, sizeof(m_pack));
    m_rpTimer = new QTimer(this);
    m_rpTimer->setInterval(30);
    connect(m_rpTimer, &QTimer::timeout, this, &DeviceManager::pollAutoReport);
}

DeviceManager::~DeviceManager()
{
    if (m_rpTimer) m_rpTimer->stop();
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

    // 一开始先发一次 AUTO_RP_HH off, 防止设备处于上次 session 残留的持续上报状态.
    // 不等 ACK, 紧接着会被 clear() 清掉.
    std::memset(&m_pack, 0, sizeof(m_pack));
    OHIDM_Base_mix_arg2(&m_pack, /*write*/1, OHID_MIX_AUTO_RP_HH, /*on=*/0, /*delay=*/0);
    m_hid.Writes(m_pack.bin, OHID_pack_size(m_pack));

    while (nowMs() < deadline) {
        m_hid.clear();
        OHIDM_None(&m_pack, OHID_CMD_BASE_SYNC);
        int res = m_hid.Writes(m_pack.bin, OHID_pack_size(m_pack));
        if (res < 0) {
            qWarning("[DeviceManager] Writes failed");
            return -1;
        }
        // 单次发送后, 在 ~400ms 窗口内连续读, 跳过 RP 等非 SYNC 帧.
        // 如果设备在持续推 RP, 单次 Reads(100ms) 多半返回的是 RP 帧, 之前
        // 直接 continue 会立刻 clear() 把真正的 SYNC ACK 一起丢掉.
        const quint64 innerDeadline = qMin(nowMs() + 400, deadline);
        while (nowMs() < innerDeadline) {
            std::memset(m_pack.bin, 0, sizeof(m_pack.bin));
            uint16_t size = m_hid.Reads(m_pack.bin, 64, 50);
            if (size <= 4) continue;
            int len = _OHIDS_Decode(&m_pack, size);
            if (len < 4) continue;
            if (m_pack.ohid.cmd != OHID_CMD_BASE_SYNC) continue;  // skip RP/MIX
            if (len < (OHID_HEAD_SIZE + 20)) continue;

            std::memset(m_info.SN, 0, sizeof(m_info.SN));
            std::memset(m_info.Version, 0, sizeof(m_info.Version));
            m_info.board_id = OHID_get_32b(&m_pack, 0);
            m_info.fwSize   = ((uint32_t)OHID_get_16b(&m_pack, 4)) << 8;
            m_info.Mode     = OHID_get_8b(&m_pack, 6);
            OHID_get_bin(&m_pack, 7,    m_info.SN,      sizeof(m_info.SN));
            OHID_get_bin(&m_pack, 7+17, m_info.Version, sizeof(m_info.Version));
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
            buildRpKeyList();
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
    if (m_rpTimer) m_rpTimer->stop();
    m_rpActive = false;
    m_lastVoltageMv = 0;
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

// ===========================================================================
//  按键自动上报 (OHID MIX AUTO_RP_HH = 0x30)
//
//  开启时设备会以 OHID_CMD_DRIVER_RP (0x1A) 持续上报数据帧:
//      data[0]    : TYPE (RP_TYPE), 当前 firmware 只发 RP_KEYSH{0..3}
//      data[1..58]: 29 × uint16 (小端), 每个 uint16 是一个键的霍尔 mV 读数
//
//  29 个键的顺序: 由 layout 矩阵按 row-major 展开 (跳过 xxSK), 取
//  [n*29, n*29+28] 这一段对应 RP_KEYSH{n} (n=0..3, 共 116 个键).
//
//  我们在 connect/sync 完成时调用 buildRpKeyList() 把 index→(row,col)
//  查表建好; 轮询时按 type 低 2 位定位组号并依次 emit
//  keyVoltageChanged(row,col,mv). 同时维护整盘 mV 缓存, 计算最大值
//  发射 voltageChanged 给 HUD 概览.
// ===========================================================================

void DeviceManager::setExternalRpKeyPositions(const QVector<QPair<int,int>>& positions)
{
    m_externalRpKeyList.clear();
    m_externalRpKeyList.reserve(positions.size());
    for (const auto& p : positions) {
        RpKeyPos rp; rp.row = (uint8_t)p.first; rp.col = (uint8_t)p.second;
        m_externalRpKeyList.append(rp);
    }
    // 若已连接, 立即重建以让设置即时生效
    if (m_connected) buildRpKeyList();
}

void DeviceManager::buildRpKeyList()
{
    m_rpKeyList.clear();

    // 优先使用 MainWindow 提供的外部列表 (基于实际加载的 layout 文件,
    // 与固件矩阵一致); 否则回退到内置 OHID_board_layout_Get 的近似.
    if (!m_externalRpKeyList.isEmpty()) {
        m_rpKeyList = m_externalRpKeyList;
        emit debugMessage(QString("[AutoRP] 使用外部 RP key list: %1 个按键 (来自 layout 文件)")
                          .arg(m_rpKeyList.size()));
        std::memset(m_voltMatrix, 0, sizeof(m_voltMatrix));
        return;
    }

    const struct tkb_Half_matrix_t* const layout = OHID_board_layout_Get(m_info.board_id);
    if (!layout) {
        emit debugMessage(QString("[AutoRP] buildRpKeyList: 未找到 board_id=0x%1 的 layout")
                          .arg((quint32)m_info.board_id, 8, 16, QChar('0')));
        return;
    }
    emit debugMessage(QString("[AutoRP] buildRpKeyList (fallback): board_id=0x%1")
                      .arg((quint32)m_info.board_id, 8, 16, QChar('0')));
    // 固件枚举规则: 仅当 (kc>=KC_A) && (非编码器) 时计入. WP98 无编码器, 编码器检查略.
    for (int row = 0; row < TKB_ROWS; ++row) {
        for (int col = 0; col < TKB_COLS; ++col) {
            const uint16_t kc = layout->Matrix[row][col];
            if (kc < (uint16_t)KC_A) continue;
            RpKeyPos p; p.row = (uint8_t)row; p.col = (uint8_t)col;
            m_rpKeyList.append(p);
        }
    }
    emit debugMessage(QString("[AutoRP] 构建 RP key list: %1 个有效按键").arg(m_rpKeyList.size()));
    std::memset(m_voltMatrix, 0, sizeof(m_voltMatrix));
}

static bool mix_arg2_blocking(InterfaceHID& hid, OpenAgreementHID_t& pack,
                              OHID_MIX_ORDER order, uint8_t arg1, uint8_t arg2,
                              uint32_t timeoutMs)
{
    const quint64 deadline = (quint64)QDateTime::currentMSecsSinceEpoch() + timeoutMs;
    while ((quint64)QDateTime::currentMSecsSinceEpoch() < deadline) {
        hid.clear();
        std::memset(&pack, 0, sizeof(pack));
        OHIDM_Base_mix_arg2(&pack, /*write*/1, order, arg1, arg2);
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

bool DeviceManager::startAutoReport(uint8_t delay, uint32_t /*timeoutMs*/)
{
    if (!m_connected) return false;
    // 直接发命令, 不等 ACK —— 等 ACK 会在 UI 线程同步阻塞.
    // ACK 如果到来会被 pollAutoReport 读到后忽略 (非 DRIVER_RP cmd 直接 continue).
    std::memset(&m_pack, 0, sizeof(m_pack));
    OHIDM_Base_mix_arg2(&m_pack, /*write*/1, OHID_MIX_AUTO_RP_HH, /*on=*/1, /*delay=*/delay);
    int res = m_hid.Writes(m_pack.bin, OHID_pack_size(m_pack));
    emit debugMessage(QString("[AutoRP] 发送 AUTO_RP_HH on delay=%1 write_res=%2").arg(delay).arg(res));
    m_rpActive = true;
    m_lastVoltageMv = 0;
    if (m_rpTimer && !m_rpTimer->isActive()) m_rpTimer->start();
    return true;
}

bool DeviceManager::stopAutoReport(uint32_t timeoutMs)
{
    if (m_rpTimer) m_rpTimer->stop();
    m_rpActive = false;
    if (!m_connected) return true;
    // 关闭时设备不再上报, 用原始的"重发直到 ACK"方式即可.
    const bool ok = mix_arg2_blocking(m_hid, m_pack, OHID_MIX_AUTO_RP_HH,
                                      /*on=*/0, /*delay=*/0, timeoutMs);
    if (!ok) qWarning("[DeviceManager] stopAutoReport: AUTO_RP_HH off failed");
    return ok;
}

void DeviceManager::pollAutoReport()
{
    if (!m_connected || !m_rpActive) return;

    union OpenAgreementHID_t pack;
    int framesThisTick = 0;
    uint8_t lastOtherCmd = 0;
    bool gotAnyRp = false;
    uint16_t maxMvSeen = 0;
    static bool first_rp_logged = false;

    // 同一轮询周期内尽量把累积的报文都消费掉; 上限 64 防止饿死 UI.
    for (int i = 0; i < 64; ++i) {
        std::memset(pack.bin, 0, sizeof(pack.bin));
        // timeout=0: 完全非阻塞, 无数据立即返回 0, 不会阻塞 UI.
        int size = m_hid.Reads(pack.bin, 64, 0);
        if (size <= 0) break;  // 0=无数据 <0=error, 本轮已无包可读
        ++framesThisTick;
        int len = _OHIDS_Decode(&pack, (uint16_t)size);
        if (len < OHID_HEAD_SIZE) continue;
        if (pack.ohid.cmd != OHID_CMD_DRIVER_RP) {
            lastOtherCmd = pack.ohid.cmd;
            continue;
        }
        // 数据布局: data[0]=TYPE, data[1..58]=29×uint16 (小端) mV.
        const uint8_t dlen = pack.ohid.length;
        if (dlen < 1) continue;
        const uint8_t type = OHID_get_8b(&pack, 0);
        // RP_KEYSH=0x04, 低 2 位是组号; RP_KEYS=0x00 (byte 形式, 暂不支持).
        if ((type & 0xFC) != (uint8_t)RP_KEYSH) continue;
        const int groupN = type & 0x03;
        // 每组最多 29 个 mV. 取 (dlen-1)/2 防越界.
        int per = ((int)dlen - 1) / 2;
        if (per > 29) per = 29;

        if (!first_rp_logged) {
            first_rp_logged = true;
            emit debugMessage(QString("[AutoRP] 首帧 RP_KEYSH%1 dlen=%2 per=%3 keyList=%4")
                              .arg(groupN).arg(dlen).arg(per).arg(m_rpKeyList.size()));
        }

        for (int j = 0; j < per; ++j) {
            const int idx = groupN * 29 + j;
            if (idx >= m_rpKeyList.size()) break;
            const uint16_t mv = OHID_get_16b(&pack, (uint8_t)(1 + j * 2));
            const auto& p = m_rpKeyList[idx];
            // 只在数值变化时下发 (减少 UI 信号风暴).
            if (m_voltMatrix[p.row][p.col] != mv) {
                m_voltMatrix[p.row][p.col] = mv;
                emit keyVoltageChanged(p.row, p.col, mv);
            }
            if (mv > maxMvSeen) maxMvSeen = mv;
        }
        gotAnyRp = true;
    }

    // 统计调试: 每 ~1s 报告一次 (定时器 30ms × 33).
    ++m_rpPollTicks;
    m_rpFramesTotal += framesThisTick;
    if (gotAnyRp) ++m_rpFramesRp;
    if (m_rpPollTicks % 33 == 0) {
        emit debugMessage(QString("[AutoRP] 轮询统计: ticks=%1 总帧=%2 RP帧=%3"
                                  " lastOtherCmd=0x%4")
                          .arg(m_rpPollTicks).arg(m_rpFramesTotal).arg(m_rpFramesRp)
                          .arg(lastOtherCmd, 2, 16, QChar('0')));
    }

    if (gotAnyRp) {
        // 概览电压 = 当前缓存矩阵的最大值
        uint16_t mx = 0;
        for (int r = 0; r < 6; ++r)
            for (int c = 0; c < 32; ++c)
                if (m_voltMatrix[r][c] > mx) mx = m_voltMatrix[r][c];
        if (mx != m_lastVoltageMv) {
            m_lastVoltageMv = mx;
            emit voltageChanged(mx);
        }
    }
}
