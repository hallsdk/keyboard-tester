#include "KeyboardGrid.h"

#include <QSizePolicy>
#include <QHBoxLayout>
#include <QDebug>

#include "Layout/HID_Usage_Tables.h"

static QString driverCodeToLabel(uint16_t code);

KeyboardGrid::KeyboardGrid(QWidget* parent)
    : QWidget(parent)
{
    for (int r = 0; r < TKB_ROWS; ++r)
        for (int c = 0; c < TKB_COLS; ++c)
            m_matrix[r][c] = nullptr;

    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(6, 6, 6, 6);
    m_grid->setHorizontalSpacing(0);
    m_grid->setVerticalSpacing(3);
    setLayout(m_grid);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void KeyboardGrid::clearGrid()
{
    // 每行在 m_grid (r, 0) 存放一个 rowWidget 容器; 删除它会级联删除子按钮.
    for (int r = 0; r < TKB_ROWS; ++r) {
        QLayoutItem* item = m_grid->itemAtPosition(r, 0);
        if (item && item->widget()) {
            m_grid->removeWidget(item->widget());
            item->widget()->deleteLater();
        }
        m_grid->setRowStretch(r, 0);
        for (int c = 0; c < TKB_COLS; ++c)
            m_matrix[r][c] = nullptr;
    }
    m_driverToPos.clear();
    m_grid->setColumnStretch(0, 0);
}

QString KeyboardGrid::defaultButtonStyle()
{
    return QStringLiteral(
        "QPushButton {"
        "  background-color: rgb(14, 31, 44);"
        "  color: rgb(85, 195, 183);"
        "  border: 1px solid rgb(30, 50, 70);"
        "  border-radius: 4px;"
        "  font-size: 10px;"
        "  padding: 2px;"
        "}");
}

QString KeyboardGrid::pressedButtonStyle()
{
    return QStringLiteral(
        "QPushButton {"
        "  background-color: rgb(255, 153, 0);"
        "  color: white;"
        "  border: 1px solid rgb(200, 120, 0);"
        "  border-radius: 4px;"
        "  font-size: 10px;"
        "  padding: 2px;"
        "}");
}

void KeyboardGrid::buildGrid(const LayoutConfig::Entry& entry)
{
    clearGrid();

    const auto& mat = entry.matrix;
    const int SCALE = 100;  // 每"1列宽"对应的 stretch 单位

    for (int r = 0; r < TKB_ROWS; ++r) {
        // 跳过全空行
        bool hasKey = false;
        for (int c = 0; c < TKB_COLS; ++c)
            if (mat.Matrix[r][c] != (uint16_t)xxSK) { hasKey = true; break; }
        if (!hasKey) continue;

        // 每行一个容器 widget + QHBoxLayout
        QWidget* rowWidget = new QWidget(this);
        QHBoxLayout* hbox = new QHBoxLayout(rowWidget);
        hbox->setContentsMargins(0, 0, 0, 0);
        hbox->setSpacing(0);

        bool prevVisible = false;
        for (int c = 0; c < TKB_COLS; ++c) {
            int colWidth = (entry.hasSpacings && c < entry.colSpacings.size())
                           ? entry.colSpacings[c] : 1;
            uint16_t code = mat.Matrix[r][c];
            double   gap  = entry.keyGaps.value({r, c}, 0.0);

            if (code == (uint16_t)xxSK) {
                // 空列作为弹性间距占位符
                hbox->addStretch(colWidth * SCALE);
                prevVisible = false;
            } else {
                // 相邻可见按键之间固定 3px 间隔
                if (prevVisible) hbox->addSpacing(3);
                // key_gaps 指定的额外左侧间距 (弹性, 随窗口缩放)
                if (gap > 0.0) hbox->addStretch(qRound(gap * SCALE));

                QPushButton* btn = new QPushButton(driverCodeToLabel(code), rowWidget);
                btn->setFocusPolicy(Qt::NoFocus);
                btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                btn->setMinimumSize(24, 24);
                btn->setStyleSheet(defaultButtonStyle());
                hbox->addWidget(btn, colWidth * SCALE);

                m_matrix[r][c] = btn;
                m_driverToPos.insert(code, {r, c});
                prevVisible = true;
            }
        }

        int rowHeight = (entry.hasSpacings && r < entry.rowSpacings.size())
                        ? entry.rowSpacings[r] : 1;
        m_grid->addWidget(rowWidget, r, 0);
        m_grid->setRowStretch(r, rowHeight * 10);
    }

    m_grid->setColumnStretch(0, 1);
}

void KeyboardGrid::applyLayout(uint32_t board_id)
{
    const LayoutConfig::Entry& e = LayoutConfig::resolve(board_id);
    buildGrid(e);
}

void KeyboardGrid::applyLayout(const LayoutConfig::Entry& entry)
{
    buildGrid(entry);
}

QPushButton* KeyboardGrid::getButton(int row, int col) const
{
    if (row < 0 || row >= TKB_ROWS || col < 0 || col >= TKB_COLS) return nullptr;
    return m_matrix[row][col];
}

void KeyboardGrid::resetAllColors()
{
    const QString s = defaultButtonStyle();
    for (int r = 0; r < TKB_ROWS; ++r)
        for (int c = 0; c < TKB_COLS; ++c)
            if (m_matrix[r][c]) m_matrix[r][c]->setStyleSheet(s);
}

QPair<int,int> KeyboardGrid::findPositionByDriverCode(uint16_t driverCode) const
{
    auto it = m_driverToPos.find(driverCode);
    if (it == m_driverToPos.end()) return {-1, -1};
    return it.value();
}

// -------- Cosmetic label generation for buttons --------
static QString driverCodeToLabel(uint16_t code)
{
    // Letters
    if (code >= KC_A && code <= KC_Z)
        return QString(QChar('A' + (code - KC_A)));
    // Digits
    if (code >= KC_1 && code <= KC_9) return QString::number(code - KC_1 + 1);
    if (code == KC_0) return "0";
    // F-keys
    if (code >= KC_F1 && code <= KC_F12)
        return QString("F%1").arg(code - KC_F1 + 1);

    switch (code) {
    case KC_ESC:           return "Esc";
    case KC_TAB:           return "Tab";
    case KC_CAPS:          return "Caps";
    case KC_ENTER:         return "Enter";
    case KC_SPACE:         return "Space";
    case KC_BACKSPACE:     return "Bksp";
    case KC_MINS:          return "-";
    case KC_EQUAL:         return "=";
    case KC_LEFT_BRACKET:  return "[";
    case KC_RIGHT_BRACKET: return "]";
    case KC_BACKSLASH:     return "\\";
    case KC_SEMICOLON:     return ";";
    case KC_QUOTE:         return "'";
    case KC_GRAVE:         return "`";
    case KC_COMMA:         return ",";
    case KC_DOT:           return ".";
    case KC_SLASH:         return "/";
    case KC_LSFT:          return "Shift";
    case KC_RSFT:          return "Shift";
    case KC_LCTL:          return "Ctrl";
    case KC_RCTL:          return "Ctrl";
    case KC_LALT:          return "Alt";
    case KC_RALT:          return "Alt";
    case KC_LGUI:          return "Win";
    case KC_RGUI:          return "Win";
    case KC_MENU:          return "Menu";
    case KC_APPLICATION:   return "Menu";
    case KC_INSERT:        return "Ins";
    case KC_DELETE:        return "Del";
    case KC_HOME:          return "Home";
    case KC_END:           return "End";
    case KC_PAGE_UP:       return "PgUp";
    case KC_PAGE_DOWN:     return "PgDn";
    case KC_UP:            return "↑";
    case KC_DOWN:          return "↓";
    case KC_LEFT:          return "←";
    case KC_RIGHT:         return "→";
    case KC_PRINT_SCREEN:  return "PrtSc";
    case KC_SCROLL_LOCK:   return "ScrLk";
    case KC_PAUSE:         return "Pause";
    case KC_NUM_LOCK:      return "Num";
    case KC_KP_0: return "0"; case KC_KP_1: return "1"; case KC_KP_2: return "2";
    case KC_KP_3: return "3"; case KC_KP_4: return "4"; case KC_KP_5: return "5";
    case KC_KP_6: return "6"; case KC_KP_7: return "7"; case KC_KP_8: return "8";
    case KC_KP_9: return "9";
    case KC_KP_DOT:       return ".";
    case KC_KP_PLUS:      return "+";
    case KC_KP_MINUS:     return "-";
    case KC_KP_ASTERISK:  return "*";
    case KC_KP_SLASH:     return "/";
    case KC_KP_ENTER:     return "Enter";
    // FK function layer keys
    case FK_FN1:          return "Fn";
    case FK_FN2:          return "Fn2";
    case FK_FN3:          return "Fn3";
    case FK_FN4:          return "Fn4";
    case FK_FN5:          return "Fn5";
    case FK_FN6:          return "Fn6";
    case FK_FN7:          return "Fn7";
    case FK_FN8:          return "Fn8";
    case FK_FN9:          return "Fn9";
    case FK_FN10:         return "Fn10";
    case FK_FN11:         return "Fn11";
    case FK_FN12:         return "Fn12";
    case FK_FN13:         return "Fn13";
    case FK_FN14:         return "Fn14";
    case FK_FN15:         return "Fn15";
    case FK_FN16:         return "Fn16";
    }
    return QString("0x%1").arg(code, 4, 16, QChar('0'));
}
