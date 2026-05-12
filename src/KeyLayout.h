#ifndef KEYLAYOUT_H
#define KEYLAYOUT_H

#include <QString>
#include <QVector>
#include <QRectF>
#include <cstdint>

// A single key as described by the JSON layout.
// Geometry is in "key units" (KLE style); a unit is converted to pixels
// at render time by KeyboardWidget using KeyLayout::unitPx().
//
// Optional secondary rectangle (x2/y2/w2/h2) supports two-rect keys such as
// ISO Enter, big-ass Enter, or stepped Caps.
struct KeyDef {
    qreal x = 0.0;        // top-left in key units
    qreal y = 0.0;
    qreal w = 1.0;        // width  in key units (default 1.0)
    qreal h = 1.0;        // height in key units (default 1.0)

    bool   hasSecond = false;
    qreal  x2 = 0.0;
    qreal  y2 = 0.0;
    qreal  w2 = 1.0;
    qreal  h2 = 1.0;

    QString label;        // text shown on the key
    QString codeName;     // raw code name from JSON (for diagnostics)
    uint16_t code  = 0;   // resolved OHID key code (KC_*); 0 == KC_NO/none
    int  row = -1;        // optional HID matrix position (for device tests)
    int  col = -1;

    bool decal = false;   // pure spacer, not tested

    // Convenience
    bool isMultimedia() const;        // Consumer / Desktop page
    QRectF primaryRect()   const { return QRectF(x,  y,  w,  h ); }
    QRectF secondaryRect() const { return QRectF(x2, y2, w2, h2); }
};


class KeyLayout {
public:
    KeyLayout();

    // Load layout from JSON file. Returns true on success.
    bool loadFromFile(const QString& path, QString* errMsg = nullptr);

    // Load layout from raw JSON bytes (supports either "keys" or "matrix" mode).
    bool loadFromJson(const QByteArray& bytes, QString* errMsg = nullptr);

    // Load layout from a raw C-style matrix snippet (.c/.h/.txt). The parser
    // looks for one or more brace-enclosed rows of comma-separated KC_* /
    // FK_* / xxSK / ____ tokens (matching the format used in OHID_Layout.c)
    // and builds a row=col grid (1u per cell).
    bool loadFromMatrixText(const QByteArray& text,
                            const QString& name,
                            QString* errMsg = nullptr);

    // Helper used by both JSON and raw text loaders. Build keys from a 2-D
    // token grid. `widths` may be empty or parallel-shaped to `rows`.
    bool buildFromMatrix(const QVector<QStringList>& rows,
                         const QVector<QVector<double>>& widths,
                         QString* errMsg = nullptr);

    const QString& name() const { return m_name; }
    const QVector<KeyDef>& keys() const { return m_keys; }
    QVector<KeyDef>& keysMutable() { return m_keys; }

    // Render-time scaling.
    int   unitPx() const { return m_unitPx; }
    void  setUnitPx(int px) { m_unitPx = px; }

    int   margin() const { return m_margin; }

    // Layout bounding box in pixels (after scaling by unitPx).
    QSize boundingPixelSize() const;

    // Helpers: parse a code name like "KC_ESC" / "KC_MUTE" / "0x0029".
    static uint16_t parseCode(const QString& s, bool* ok = nullptr);

    // Build (lazily) a reverse map: KC_ value -> canonical name (best human label).
    static QString codeToShortName(uint16_t code);

    // Verbose human description for the status bar / log, e.g.
    //   KC_VINC -> "Volume Up", KC_PLAY -> "Play/Pause", KC_LEFT_BRACKET -> "[".
    // Falls back to codeToShortName() when no special name applies.
    static QString codeToHumanName(uint16_t code);

private:
    QString m_name;
    int     m_unitPx = 54;     // pixels per key unit
    int     m_margin = 12;     // outer margin in pixels
    QVector<KeyDef> m_keys;
};

#endif // KEYLAYOUT_H
