#ifndef LAYOUTCONFIG_H
#define LAYOUTCONFIG_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QPair>
#include <cstdint>

extern "C" {
#include "Layout/OHID_Layout.h"
}

/**
 * LayoutConfig
 *  - 从 applicationDirPath()/config.json 读取 Layouts 字典.
 *  - key 为 board_id (16 进制字符串, 如 "0x1750010") 或 "default".
 *  - value 包含 rows (token matrix) 和 spacings (可选, row/col 的宽度比例).
 *  - resolve(board_id) 优先返回该 id 的配置, 否则返回 default, 否则返回代码内置的
 *    OHID_board_layout_Get() 矩阵. 返回的矩阵指针恒定有效 (内部 static 存储).
 */
class LayoutConfig
{
public:
    struct Entry {
        tkb_Half_matrix_t        matrix;                    // driverCode 矩阵
        QVector<int>             colSpacings;               // 每列宽度比例 (size 可 < TKB_COLS; 缺省为 1)
        QVector<int>             rowSpacings;               // 每行高度比例
        QMap<QPair<int,int>, double> keyGaps;               // (row,col) → 该键左侧额外间距 (单位: 1列宽)
        bool                     hasSpacings = false;
    };

    // 加载/重新加载 config.json
    static void load();

    // 根据 board_id 解析布局; 返回的 Entry 引用在进程生命期内有效.
    static const Entry& resolve(uint32_t board_id);

    // 仅返回矩阵 (方便和旧代码兼容).
    static const tkb_Half_matrix_t* resolveMatrix(uint32_t board_id);

private:
    static void ensureLoaded();
    static bool parseEntry(const class QJsonObject& obj, Entry& out);
    static uint16_t tokenToCode(const QString& token);

    static bool                 s_loaded;
    static QMap<uint32_t, Entry> s_byId;
    static Entry                s_default;
    static bool                 s_hasDefault;
};

#endif // LAYOUTCONFIG_H
