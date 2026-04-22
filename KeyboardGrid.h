#ifndef KEYBOARDGRID_H
#define KEYBOARDGRID_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QMap>
#include <QPair>
#include <cstdint>

#include "LayoutConfig.h"

/**
 * KeyboardGrid
 *  - 依据 LayoutConfig::Entry 动态生成 TKB_ROWS x TKB_COLS 的按键网格.
 *  - xxSK 位置留空 (不创建按钮).
 *  - 支持 spacings (列宽/行高比例) 让不等宽按键正确显示.
 *  - 窗口缩放时网格自动伸缩 (QGridLayout + QSizePolicy::Expanding).
 *  - 提供 getButton(r,c) / highlight(r,c,style) / resetAllColors() 接口.
 */
class KeyboardGrid : public QWidget
{
    Q_OBJECT
public:
    explicit KeyboardGrid(QWidget* parent = nullptr);
    ~KeyboardGrid() override = default;

    // 根据 board_id 重建网格 (会调用 LayoutConfig::resolve).
    void applyLayout(uint32_t board_id);

    // 直接传入 Entry 以重建 (方便 config 热更新场景).
    void applyLayout(const LayoutConfig::Entry& entry);

    QPushButton* getButton(int row, int col) const;

    // 将所有按钮恢复为默认样式.
    void resetAllColors();

    // 默认 / 按下样式表 (公开以便外部写入高亮时保持一致).
    static QString defaultButtonStyle();
    static QString pressedButtonStyle();

    // 根据 driverCode 查找位置 (仅主布局, 不含 FN 层). 不存在时返回 {-1,-1}.
    QPair<int,int> findPositionByDriverCode(uint16_t driverCode) const;

private:
    void clearGrid();
    void buildGrid(const LayoutConfig::Entry& entry);

    QGridLayout*      m_grid = nullptr;
    QPushButton*      m_matrix[TKB_ROWS][TKB_COLS];
    QMap<uint16_t, QPair<int,int>> m_driverToPos;
};

#endif // KEYBOARDGRID_H
