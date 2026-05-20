#ifndef KEYBOARDVIEW_H
#define KEYBOARDVIEW_H

#include <QWidget>
#include <QVector>
#include <QHash>
#include <cstdint>

#include "KeyLayout.h"

class QPushButton;

// KeyboardView renders a KeyLayout using one (or two, for ISO-Enter) absolute-
// positioned QPushButton per KeyDef. Each button stores the index of its KeyDef
// in property("keyIndex"), and a shared `keyClicked(index)` signal lets the
// outer MainWindow react.
//
// State coloring API:
//   setKeyPressed(index, true|false)
//   setKeyPassed(index)
//   setKeyFailed(index)
//   resetState()
//
// The widget never owns the layout — it just observes a KeyLayout reference.
class KeyboardView : public QWidget
{
    Q_OBJECT
public:
    explicit KeyboardView(QWidget* parent = nullptr);

    // (Re)build child buttons from the given layout. Subsequent calls discard
    // the old buttons. Caller keeps ownership of the layout.
    void setLayout(const KeyLayout& layout);

    // Number of keys in the current layout.
    int  keyCount() const { return m_primaryBtns.size(); }

    // The KeyDef for index, or nullptr.
    const KeyDef* keyAt(int index) const;

    // Find first key index with given KC_* code; -1 if none.
    int findIndexByCode(uint16_t code) const;

    // State manipulation.
    enum State { Idle, Pressed, Passed, Failed, Disabled };
    void setKeyState(int index, State s);
    State keyState(int index) const;
    void  resetState();
    // Mark a key as not testable (e.g. Fn). Disabled keys are excluded from
    // totalTestable() / passedCount() and are styled greyed-out.
    void  setKeyDisabled(int index);
    // Replace the visible text on a key (both rectangles if hasSecond).
    void setKeyLabel(int index, const QString& text);
    // 设置 / 清除 单个按键的实时霍尔电压 (mV). 设为 -1 表示清除.
    // 文字会以原 label + 换行 + "X.XXV" 的形式显示在按钮上.
    void setKeyVoltage(int index, int mv);
    void clearAllVoltages();
    // 根据 OHID 矩阵坐标找到对应的 keyIndex; 没匹配返回 -1.
    int  findIndexByRowCol(int row, int col) const;
    // Theme: true = dark (default), false = light.
    void setDarkTheme(bool dark);
    // Convenience: counts.
    int passedCount() const;
    int totalTestable() const;     // excludes decals + KC_NO

signals:
    void keyClicked(int keyIndex);

protected:
    void resizeEvent(QResizeEvent* ev) override;
    void paintEvent(QPaintEvent* ev) override;

private slots:
    void onChildClicked();

private:
    void clearButtons();
    void rebuildGeometry();
    void styleButton(int index);

    // Compute scale factor so the keyboard fits inside `size()`.
    qreal currentScale() const;

    KeyLayout            m_layout;     // own copy for sizing math
    QVector<QPushButton*> m_primaryBtns;
    QVector<QPushButton*> m_secondBtns; // index parallel; entries may be null
    QVector<State>        m_states;
    QVector<int>          m_voltagesMv; // -1 if unset, else mV
    QHash<uint16_t, int>  m_codeIndex;  // first index per KC code
    bool                  m_darkTheme = true;
};

#endif // KEYBOARDVIEW_H
