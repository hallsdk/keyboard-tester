#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QVector>
#include <cstdint>

class QTimer;

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

    // 读取指定参数页 (OHID_PARAM_PAGE：OHID_PAGE_FN0/FN1/...) 下
    // 一批键码 mkeys 对应的参数值。返回成功时
    // *out 包含 mkey -> value 的映射。需要设备已连接。
    bool readParamPage(uint8_t page,
                       const QVector<uint16_t>& mkeys,
                       QHash<uint16_t, uint16_t>* out,
                       uint32_t perBatchTimeoutMs = 200);

    // 设置整块键盘为单一恒亮色 (用于灯光工厂测试).
    // argb: 0xAARRGGBB; alpha 通常 0xFF, 0x00000000 = 熄灭.
    // 内部依次发: MIX_RGB_COLOR (调色板 palette[0]=color) → MIX_RGB_IDX (0)
    //             → MIX_RGB_MODE (RGB_MODE_LIGHT).
    bool setSolidColor(uint32_t argb, uint32_t timeoutMs = 500);

    // 还原成键盘默认灯效 (RGB_MODE_LIGHT, palette[0]=白色, idx=0).
    // 实际"默认"应由具体型号决定, 这里给一个通用的恒亮白光.
    bool restoreDefaultLight(uint32_t timeoutMs = 500);

    // ----- RGB 全量状态 (用于测试前保存 / 测试后还原) -----
    struct RGBState {
        uint32_t back = 0;
        uint32_t palettes[8] = {0,0,0,0,0,0,0,0};
        uint8_t  gray = 0;
        uint8_t  mode = 0;
        uint8_t  speed = 0;
        uint8_t  sleep = 0;
        uint8_t  on = 0;
        uint8_t  on_sleep = 0;
        uint8_t  reverse = 0;
        bool     valid = false;
    };
    // 读取设备当前的灯效参数 (OHID_CMD_DRIVER_RGB_PARAM, write=0)
    bool readRGBState(RGBState* out, uint32_t timeoutMs = 800);
    // 写入完整灯效参数; 用于把保存的状态恢复回去.
    bool writeRGBState(const RGBState& st, uint32_t timeoutMs = 800);

    bool isConnected() const { return m_connected; }
    const BoardInfo& info() const { return m_info; }

    // ----- 按键自动上报 (OHID MIX AUTO_RP_HH) -----
    // 开启后设备会以 OHID_CMD_DRIVER_RP (0x1A) 主动上报数据;
    // DeviceManager 内部以定时器轮询 HID 通道,并将报文最后一个 uint16
    // 作为电压发射 voltageChanged(mv).
    // delay: 上报延时 (单位由固件定义, 通常 ms).
    bool startAutoReport(uint8_t delay = 10, uint32_t timeoutMs = 500);
    bool stopAutoReport(uint32_t timeoutMs = 500);
    bool isAutoReporting() const { return m_rpActive; }
    uint16_t lastVoltageMv() const { return m_lastVoltageMv; }

signals:
    void connected(uint32_t board_id);
    void disconnected();
    // 整体电压 (取自整块键盘 mV 的最大值, 用于 HUD 概览)
    void voltageChanged(uint16_t mv);
    // 单个按键的实时霍尔 mV 读数 (来自 DRIVER_RP / RP_KEYSH 上报).
    // row, col 为 OHID 矩阵坐标 (0..5 / 0..20).
    void keyVoltageChanged(int row, int col, uint16_t mv);
    // 诊断日志 (转发到 UI 日志窗口)
    void debugMessage(const QString& msg);

private slots:
    void pollAutoReport();

public:
    // 外部 (MainWindow) 在加载完设备的 layout 文件后, 用其(行,列)序列
    // 覆盖默认从 OHID_board_layout_Get 推断的 mkey 列表. 顺序必须严格
    // 按"行优先, 跳过 xxSK / decal", 与固件 get_mkey() 一致.
    void setExternalRpKeyPositions(const QVector<QPair<int,int>>& positions);

private:
    int  scanAndOpen(uint16_t vid, uint16_t pid);  // populates m_pathList
    bool openNextPath(int& startIdx);              // opens next raw-HID candidate path
    int  syncBoard(uint32_t timeoutMs);
    // 根据 m_info.board_id 取出对应 layout, 按 row-major 顺序构造
    // RP_KEYSH 报文的 mkey 索引到 (row,col) 的查表 (跳过 xxSK).
    void buildRpKeyList();

    InterfaceHID            m_hid;
    InterfaceList           m_pathList;
    union OpenAgreementHID_t m_pack;
    BoardInfo               m_info;
    bool                    m_connected = false;
    quint64                 m_deadlineMs = 0;

    // --- 按键上报轮询 ---
    QTimer*  m_rpTimer       = nullptr;
    bool     m_rpActive      = false;
    uint16_t m_lastVoltageMv = 0;
    // RP_KEYSH 索引 → (row,col) 表 (在 connect/sync 完成后建立)
    struct RpKeyPos { uint8_t row; uint8_t col; };
    QVector<RpKeyPos> m_rpKeyList;
    // 若 MainWindow 通过 setExternalRpKeyPositions 提供了基于真实
    // 设备 layout 文件的列表, 在 buildRpKeyList() 中优先使用.
    QVector<RpKeyPos> m_externalRpKeyList;
    // 当前缓存的整盘电压 (mV), 用于 voltageChanged 概览
    uint16_t m_voltMatrix[6][32] = {{0}};
    // 诊断计数
    uint32_t m_rpPollTicks   = 0;
    uint32_t m_rpFramesTotal = 0;
    uint32_t m_rpFramesRp    = 0;

    static quint64 nowMs();
};

#endif // DEVICEMANAGER_H
