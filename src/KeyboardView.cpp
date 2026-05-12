#include "KeyboardView.h"

#include <QPushButton>
#include <QResizeEvent>
#include <QPainter>
#include <QPaintEvent>

extern "C" {
#include "Layout/HID_Usage_Tables.h"
}

// ---- Style snippets -------------------------------------------------------
namespace {

QString styleFor(KeyboardView::State s, bool decal)
{
    if (decal) {
        return QStringLiteral(
            "QPushButton { background: transparent; color: rgb(120,120,120);"
            " border: 1px dashed rgb(80,80,80); border-radius: 4px; font-size: 10px; }");
    }
    switch (s) {
    case KeyboardView::Pressed:
        return QStringLiteral(
            "QPushButton { background-color: rgb(255,153,0); color: white;"
            " border: 1px solid rgb(200,120,0); border-radius: 5px;"
            " font-size: 11px; font-weight: 600; padding: 1px; }");
    case KeyboardView::Passed:
        return QStringLiteral(
            "QPushButton { background-color: rgb(46,160,67); color: white;"
            " border: 1px solid rgb(30,120,50); border-radius: 5px;"
            " font-size: 11px; padding: 1px; }");
    case KeyboardView::Failed:
        return QStringLiteral(
            "QPushButton { background-color: rgb(218,54,51); color: white;"
            " border: 1px solid rgb(170,30,30); border-radius: 5px;"
            " font-size: 11px; padding: 1px; }");
    case KeyboardView::Disabled:
        return QStringLiteral(
            "QPushButton { background-color: rgb(40,42,48); color: rgb(110,115,125);"
            " border: 1px dashed rgb(75,80,90); border-radius: 5px;"
            " font-size: 10px; font-style: italic; padding: 1px; }");
    case KeyboardView::Idle:
    default:
        return QStringLiteral(
            "QPushButton { background-color: rgb(45,55,72); color: rgb(220,240,250);"
            " border: 1px solid rgb(70,90,110); border-radius: 5px;"
            " font-size: 11px; padding: 1px; }"
            "QPushButton:hover { background-color: rgb(60,75,95); }"
            "QPushButton:disabled { background-color: rgb(28,32,40); color: rgb(90,90,90);"
            " border: 1px dashed rgb(60,60,60); }");
    }
}

} // namespace


KeyboardView::KeyboardView(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: rgb(20,24,30);");
    setMinimumSize(400, 200);
}

void KeyboardView::clearButtons()
{
    for (auto* b : m_primaryBtns) if (b) b->deleteLater();
    for (auto* b : m_secondBtns)  if (b) b->deleteLater();
    m_primaryBtns.clear();
    m_secondBtns.clear();
    m_states.clear();
    m_codeIndex.clear();
}

void KeyboardView::setLayout(const KeyLayout& layout)
{
    clearButtons();
    m_layout = layout;

    const auto& keys = m_layout.keys();
    m_primaryBtns.resize(keys.size());
    m_secondBtns.resize(keys.size());
    m_states.resize(keys.size());

    for (int i = 0; i < keys.size(); ++i) {
        const KeyDef& k = keys.at(i);
        QPushButton* btn = new QPushButton(k.label, this);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setProperty("keyIndex", i);
        btn->setEnabled(!(k.decal || k.code == KC_NO));
        connect(btn, &QPushButton::clicked, this, &KeyboardView::onChildClicked);
        m_primaryBtns[i] = btn;
        btn->show();

        if (k.hasSecond) {
            QPushButton* b2 = new QPushButton(k.label, this);
            b2->setFocusPolicy(Qt::NoFocus);
            b2->setProperty("keyIndex", i);
            b2->setEnabled(!(k.decal || k.code == KC_NO));
            connect(b2, &QPushButton::clicked, this, &KeyboardView::onChildClicked);
            m_secondBtns[i] = b2;
            b2->show();
        }

        m_states[i] = Idle;
        styleButton(i);

        if (k.code != KC_NO && !k.decal && !m_codeIndex.contains(k.code))
            m_codeIndex.insert(k.code, i);
    }

    rebuildGeometry();
    update();
}

const KeyDef* KeyboardView::keyAt(int index) const
{
    if (index < 0 || index >= m_layout.keys().size()) return nullptr;
    return &m_layout.keys().at(index);
}

int KeyboardView::findIndexByCode(uint16_t code) const
{
    auto it = m_codeIndex.find(code);
    if (it == m_codeIndex.end()) return -1;
    return it.value();
}

void KeyboardView::setKeyState(int index, State s)
{
    if (index < 0 || index >= m_states.size()) return;
    if (m_states[index] == s) return;
    m_states[index] = s;
    styleButton(index);
}

KeyboardView::State KeyboardView::keyState(int index) const
{
    if (index < 0 || index >= m_states.size()) return Idle;
    return m_states[index];
}

void KeyboardView::resetState()
{
    for (int i = 0; i < m_states.size(); ++i) {
        m_states[i] = Idle;
        styleButton(i);
    }
}

void KeyboardView::setKeyLabel(int index, const QString& text)
{
    if (index < 0 || index >= m_primaryBtns.size()) return;
    if (m_primaryBtns[index]) m_primaryBtns[index]->setText(text);
    if (m_secondBtns[index])  m_secondBtns[index]->setText(text);
}

int KeyboardView::passedCount() const
{
    int n = 0;
    for (auto s : m_states) if (s == Passed) ++n;
    return n;
}

int KeyboardView::totalTestable() const
{
    int n = 0;
    for (int i = 0; i < m_layout.keys().size(); ++i) {
        const auto& k = m_layout.keys()[i];
        if (k.decal || k.code == KC_NO) continue;
        if (i < m_states.size() && m_states[i] == Disabled) continue;
        ++n;
    }
    return n;
}

void KeyboardView::setKeyDisabled(int index)
{
    setKeyState(index, Disabled);
    if (index >= 0 && index < m_primaryBtns.size()) {
        if (m_primaryBtns[index]) m_primaryBtns[index]->setEnabled(false);
        if (m_secondBtns[index])  m_secondBtns[index]->setEnabled(false);
    }
}

void KeyboardView::styleButton(int i)
{
    const KeyDef* k = keyAt(i);
    if (!k) return;
    const QString s = styleFor(m_states[i], k->decal);
    if (m_primaryBtns[i]) m_primaryBtns[i]->setStyleSheet(s);
    if (m_secondBtns[i])  m_secondBtns[i]->setStyleSheet(s);
}

void KeyboardView::onChildClicked()
{
    QObject* src = sender();
    if (!src) return;
    bool ok = false;
    const int idx = src->property("keyIndex").toInt(&ok);
    if (!ok) return;
    emit keyClicked(idx);
}

qreal KeyboardView::currentScale() const
{
    const QSize need = m_layout.boundingPixelSize();
    if (need.width() <= 0 || need.height() <= 0) return 1.0;
    const qreal sx = qreal(width())  / qreal(need.width());
    const qreal sy = qreal(height()) / qreal(need.height());
    return qMax<qreal>(0.15, qMin(sx, sy));
}

void KeyboardView::rebuildGeometry()
{
    const qreal scale = currentScale();
    const qreal u  = m_layout.unitPx() * scale;
    const qreal m0 = m_layout.margin() * scale;

    // Center horizontally if there's extra space.
    const QSize need = m_layout.boundingPixelSize();
    const qreal usedW = need.width()  * scale;
    const qreal usedH = need.height() * scale;
    const qreal offX  = qMax<qreal>(0, (width()  - usedW) / 2.0);
    const qreal offY  = qMax<qreal>(0, (height() - usedH) / 2.0);

    const int gap = qMax(1, int(scale * 2.0));

    auto applyRect = [&](QPushButton* b, qreal kx, qreal ky, qreal kw, qreal kh)
    {
        if (!b) return;
        const int x = int(offX + m0 + kx * u) + gap / 2;
        const int y = int(offY + m0 + ky * u) + gap / 2;
        const int w = int(kw * u) - gap;
        const int h = int(kh * u) - gap;
        b->setGeometry(x, y, qMax(8, w), qMax(8, h));
    };

    const auto& keys = m_layout.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const KeyDef& k = keys.at(i);
        applyRect(m_primaryBtns[i], k.x, k.y, k.w, k.h);
        if (k.hasSecond) applyRect(m_secondBtns[i], k.x2, k.y2, k.w2, k.h2);
    }
}

void KeyboardView::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);
    rebuildGeometry();
}

void KeyboardView::paintEvent(QPaintEvent* ev)
{
    QWidget::paintEvent(ev);
    // Optional: paint layout name in the corner.
    if (m_layout.name().isEmpty()) return;
    QPainter p(this);
    p.setPen(QColor(150, 170, 190));
    p.setFont(QFont(font().family(), 9));
    p.drawText(rect().adjusted(8, 4, -8, -8),
               Qt::AlignTop | Qt::AlignLeft,
               QString("%1  |  %2 keys  |  passed %3 / %4")
                   .arg(m_layout.name())
                   .arg(m_layout.keys().size())
                   .arg(passedCount())
                   .arg(totalTestable()));
}
