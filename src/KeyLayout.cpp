#include "KeyLayout.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

extern "C" {
#include "Layout/HID_Usage_Tables.h"
#include "Layout/OHID_Layout.h"
}

// ---------------------------------------------------------------------------
//  Code-name table: maps the strings users write in JSON (e.g. "KC_ESC",
//  "KC_VINC", "ESC", "VOL_UP") to the OHID 16-bit key codes.
//  Both the canonical "KC_*" names and a few aliases are accepted.
//  Comparison is case-insensitive.
// ---------------------------------------------------------------------------
namespace {

struct CodeAlias { const char* name; uint16_t code; };

static const CodeAlias kAliases[] = {
    // Letters
    {"A",KC_A},{"B",KC_B},{"C",KC_C},{"D",KC_D},{"E",KC_E},{"F",KC_F},{"G",KC_G},
    {"H",KC_H},{"I",KC_I},{"J",KC_J},{"K",KC_K},{"L",KC_L},{"M",KC_M},{"N",KC_N},
    {"O",KC_O},{"P",KC_P},{"Q",KC_Q},{"R",KC_R},{"S",KC_S},{"T",KC_T},{"U",KC_U},
    {"V",KC_V},{"W",KC_W},{"X",KC_X},{"Y",KC_Y},{"Z",KC_Z},
    // Digits
    {"1",KC_1},{"2",KC_2},{"3",KC_3},{"4",KC_4},{"5",KC_5},
    {"6",KC_6},{"7",KC_7},{"8",KC_8},{"9",KC_9},{"0",KC_0},
    // F1..F24
    {"F1",KC_F1},{"F2",KC_F2},{"F3",KC_F3},{"F4",KC_F4},
    {"F5",KC_F5},{"F6",KC_F6},{"F7",KC_F7},{"F8",KC_F8},
    {"F9",KC_F9},{"F10",KC_F10},{"F11",KC_F11},{"F12",KC_F12},
    {"F13",KC_F13},{"F14",KC_F14},{"F15",KC_F15},{"F16",KC_F16},
    {"F17",KC_F17},{"F18",KC_F18},{"F19",KC_F19},{"F20",KC_F20},
    {"F21",KC_F21},{"F22",KC_F22},{"F23",KC_F23},{"F24",KC_F24},
    // Common keys
    {"ESC",KC_ESC},{"ESCAPE",KC_ESC},
    {"TAB",KC_TAB},
    {"CAPS",KC_CAPS},{"CAPSLOCK",KC_CAPS},{"CAPS_LOCK",KC_CAPS},
    {"ENT",KC_ENTER},{"ENTER",KC_ENTER},{"RETURN",KC_ENTER},
    {"SPC",KC_SPACE},{"SPACE",KC_SPACE},
    {"BSPC",KC_BACKSPACE},{"BKSP",KC_BACKSPACE},{"BACKSPACE",KC_BACKSPACE},
    {"MINS",KC_MINS},{"MINUS",KC_MINS},{"-",KC_MINS},
    {"EQL",KC_EQUAL},{"EQUAL",KC_EQUAL},{"=",KC_EQUAL},
    {"LBRC",KC_LEFT_BRACKET},{"[",KC_LEFT_BRACKET},
    {"RBRC",KC_RIGHT_BRACKET},{"]",KC_RIGHT_BRACKET},
    {"BSLS",KC_BACKSLASH},{"BACKSLASH",KC_BACKSLASH},{"\\",KC_BACKSLASH},
    {"SCLN",KC_SEMICOLON},{"SEMICOLON",KC_SEMICOLON},{";",KC_SEMICOLON},
    {"QUOT",KC_QUOTE},{"QUOTE",KC_QUOTE},{"'",KC_QUOTE},
    {"GRV",KC_GRAVE},{"GRAVE",KC_GRAVE},{"`",KC_GRAVE},
    {"COMM",KC_COMMA},{"COMMA",KC_COMMA},{",",KC_COMMA},
    {"DOT",KC_DOT},{"PERIOD",KC_DOT},{".",KC_DOT},
    {"SLSH",KC_SLASH},{"SLASH",KC_SLASH},{"/",KC_SLASH},
    // Modifiers
    {"LSFT",KC_LSFT},{"LSHIFT",KC_LSFT},{"SHIFT",KC_LSFT},
    {"RSFT",KC_RSFT},{"RSHIFT",KC_RSFT},
    {"LCTL",KC_LCTL},{"LCTRL",KC_LCTL},{"CTRL",KC_LCTL},
    {"RCTL",KC_RCTL},{"RCTRL",KC_RCTL},
    {"LALT",KC_LALT},{"ALT",KC_LALT},
    {"RALT",KC_RALT},
    {"LGUI",KC_LGUI},{"LWIN",KC_LGUI},{"WIN",KC_LGUI},
    {"RGUI",KC_RGUI},{"RWIN",KC_RGUI},
    {"MENU",KC_MENU},{"APP",KC_APPLICATION},{"APPLICATION",KC_APPLICATION},
    // Navigation cluster
    {"INS",KC_INSERT},{"INSERT",KC_INSERT},
    {"DEL",KC_DELETE},{"DELETE",KC_DELETE},
    {"HOME",KC_HOME},{"END",KC_END},
    {"PGUP",KC_PAGE_UP},{"PAGE_UP",KC_PAGE_UP},
    {"PGDN",KC_PAGE_DOWN},{"PAGE_DOWN",KC_PAGE_DOWN},
    {"UP",KC_UP},{"DOWN",KC_DOWN},{"LEFT",KC_LEFT},{"RIGHT",KC_RIGHT},
    {"PSCR",KC_PRINT_SCREEN},{"PRTSC",KC_PRINT_SCREEN},{"PRINT_SCREEN",KC_PRINT_SCREEN},
    {"SCRL",KC_SCROLL_LOCK},{"SLCK",KC_SCROLL_LOCK},{"SCROLL_LOCK",KC_SCROLL_LOCK},
    {"PAUS",KC_PAUSE},{"PAUSE",KC_PAUSE},{"BREAK",KC_PAUSE},
    // Numpad
    {"NUM",KC_NUM_LOCK},{"NLCK",KC_NUM_LOCK},{"NUM_LOCK",KC_NUM_LOCK},
    {"P0",KC_KP_0},{"P1",KC_KP_1},{"P2",KC_KP_2},{"P3",KC_KP_3},{"P4",KC_KP_4},
    {"P5",KC_KP_5},{"P6",KC_KP_6},{"P7",KC_KP_7},{"P8",KC_KP_8},{"P9",KC_KP_9},
    {"PDOT",KC_KP_DOT},{"KP_DOT",KC_KP_DOT},
    {"PPLS",KC_KP_PLUS},{"KP_PLUS",KC_KP_PLUS},
    {"PMNS",KC_KP_MINUS},{"KP_MINUS",KC_KP_MINUS},
    {"PAST",KC_KP_ASTERISK},{"KP_ASTERISK",KC_KP_ASTERISK},{"KP_MUL",KC_KP_ASTERISK},
    {"PSLS",KC_KP_SLASH},{"KP_SLASH",KC_KP_SLASH},{"KP_DIV",KC_KP_SLASH},
    {"PENT",KC_KP_ENTER},{"KP_ENTER",KC_KP_ENTER},
    // FN function-layer key
    {"FN",FK_FN1},{"FN1",FK_FN1},
    {"FN2",FK_FN2},{"FN3",FK_FN3},{"FN4",FK_FN4},
    {"FN5",FK_FN5},{"FN6",FK_FN6},{"FN7",FK_FN7},{"FN8",FK_FN8},
    {"FN9",FK_FN9},{"FN10",FK_FN10},{"FN11",FK_FN11},{"FN12",FK_FN12},
    {"FN13",FK_FN13},{"FN14",FK_FN14},{"FN15",FK_FN15},{"FN16",FK_FN16},
    // Consumer / multimedia keys
    {"MUTE",KC_MUTE},
    {"VINC",KC_VINC},{"VOL_UP",KC_VINC},{"VOLU",KC_VINC},
    {"VDEC",KC_VDEC},{"VOL_DN",KC_VDEC},{"VOLD",KC_VDEC},
    {"PLAY",KC_PLAY},{"PLAY_PAUSE",KC_PLAY},
    {"NTRACK",KC_NTRACK},{"NEXT",KC_NTRACK},{"NEXT_TRACK",KC_NTRACK},
    {"PTRACK",KC_PTRACK},{"PREV",KC_PTRACK},{"PREV_TRACK",KC_PTRACK},
    {"CSTOP",KC_CSTOP},{"MSTOP",KC_CSTOP},
    {"MEDIA",KC_MEDIA},{"MEDIA_PLAYER",KC_MEDIA},
    {"CALC",KC_CALC},{"CALCULATOR",KC_CALC},
    {"EMAIL",KC_EMAIL},
    {"PC",KC_PC},{"MYPC",KC_PC},{"MY_COMPUTER",KC_PC},
    {"WWW",KC_WWW},{"BROWSER",KC_WWW},{"HOME_BROWSER",KC_WWW},
    {"SEARCH",KC_SEARCH},
    {"BACK",KC_AC_BACK},{"AC_BACK",KC_AC_BACK},
    {"FORWARD",KC_FORWARD},
    {"REFRESH",KC_REFRESH},
    {"MARKS",KC_MARKS},{"BOOKMARKS",KC_MARKS},
    {"AC_STOP",KC_AC_STOP},
    {"SCRN_UP",KC_SCREENI},{"BRIGHT_UP",KC_SCREENI},
    {"SCRN_DN",KC_SCREEND},{"BRIGHT_DN",KC_SCREEND},
    {"MIC",KC_MIC},{"MICROPHONE",KC_MIC},
    // System / power
    {"POWER",KC_POWER},
    {"SLEEP",KC_SLEEP},
    {"WAKE",KC_WAKE},
    // Pass-through / blanks
    {"_",KC_NO},{"-NONE-",KC_NO},{"NONE",KC_NO},{"XXSK",KC_NO},
    {"____",KC_TRANSPARENT},{"TRNS",KC_TRANSPARENT},{"TRANSPARENT",KC_TRANSPARENT},
};

static QHash<QString, uint16_t>& aliasMap()
{
    static QHash<QString, uint16_t> m;
    if (m.isEmpty()) {
        for (const auto& a : kAliases) m.insert(QString::fromLatin1(a.name).toUpper(), a.code);
    }
    return m;
}

// Reverse map: KC_* code → short display label.
static QString defaultLabelFor(uint16_t code)
{
    if (code >= KC_A && code <= KC_Z)
        return QString(QChar('A' + (code - KC_A)));
    if (code >= KC_1 && code <= KC_9) return QString::number(code - KC_1 + 1);
    if (code == KC_0) return QStringLiteral("0");
    if (code >= KC_F1 && code <= KC_F12)
        return QString("F%1").arg(code - KC_F1 + 1);
    if (code >= KC_F13 && code <= KC_F24)
        return QString("F%1").arg(code - KC_F13 + 13);

    switch (code) {
    case KC_ESC: return "Esc"; case KC_TAB: return "Tab"; case KC_CAPS: return "Caps";
    case KC_ENTER: return "Enter"; case KC_SPACE: return "Space"; case KC_BACKSPACE: return "Bksp";
    case KC_MINS: return "-"; case KC_EQUAL: return "="; case KC_LEFT_BRACKET: return "[";
    case KC_RIGHT_BRACKET: return "]"; case KC_BACKSLASH: return "\\";
    case KC_SEMICOLON: return ";"; case KC_QUOTE: return "'"; case KC_GRAVE: return "`";
    case KC_COMMA: return ","; case KC_DOT: return "."; case KC_SLASH: return "/";
    case KC_LSFT: case KC_RSFT: return "Shift";
    case KC_LCTL: case KC_RCTL: return "Ctrl";
    case KC_LALT: case KC_RALT: return "Alt";
    case KC_LGUI: case KC_RGUI: return "Win";
    case KC_MENU: case KC_APPLICATION: return "Menu";
    case KC_INSERT: return "Ins"; case KC_DELETE: return "Del";
    case KC_HOME: return "Home"; case KC_END: return "End";
    case KC_PAGE_UP: return "PgUp"; case KC_PAGE_DOWN: return "PgDn";
    case KC_UP: return QString::fromUtf8("↑");
    case KC_DOWN: return QString::fromUtf8("↓");
    case KC_LEFT: return QString::fromUtf8("←");
    case KC_RIGHT: return QString::fromUtf8("→");
    case KC_PRINT_SCREEN: return "PrtSc";
    case KC_SCROLL_LOCK: return "ScrLk"; case KC_PAUSE: return "Pause";
    case KC_NUM_LOCK: return "Num";
    case KC_KP_0: return "0"; case KC_KP_1: return "1"; case KC_KP_2: return "2";
    case KC_KP_3: return "3"; case KC_KP_4: return "4"; case KC_KP_5: return "5";
    case KC_KP_6: return "6"; case KC_KP_7: return "7"; case KC_KP_8: return "8";
    case KC_KP_9: return "9";
    case KC_KP_DOT: return "."; case KC_KP_PLUS: return "+"; case KC_KP_MINUS: return "-";
    case KC_KP_ASTERISK: return "*"; case KC_KP_SLASH: return "/"; case KC_KP_ENTER: return "Enter";
    case FK_FN1: return "Fn";
    case FK_FN2: return "Fn2"; case FK_FN3: return "Fn3"; case FK_FN4: return "Fn4";
    case FK_FN5: return "Fn5"; case FK_FN6: return "Fn6"; case FK_FN7: return "Fn7"; case FK_FN8: return "Fn8";
    case FK_FN9: return "Fn9"; case FK_FN10: return "Fn10"; case FK_FN11: return "Fn11"; case FK_FN12: return "Fn12";
    case FK_FN13: return "Fn13"; case FK_FN14: return "Fn14"; case FK_FN15: return "Fn15"; case FK_FN16: return "Fn16";
    // Multimedia
    case KC_MUTE: return QString::fromUtf8("🔇");
    case KC_VINC: return QString::fromUtf8("🔊+");
    case KC_VDEC: return QString::fromUtf8("🔉-");
    case KC_PLAY: return QString::fromUtf8("▶||");
    case KC_NTRACK: return QString::fromUtf8("⏭");
    case KC_PTRACK: return QString::fromUtf8("⏮");
    case KC_CSTOP:  return QString::fromUtf8("⏹");
    case KC_MEDIA:  return "Media";
    case KC_CALC:   return "Calc";
    case KC_EMAIL:  return "Mail";
    case KC_PC:     return "MyPC";
    case KC_WWW:    return "Web";
    case KC_SEARCH: return "Search";
    case KC_AC_BACK: return QString::fromUtf8("◀");
    case KC_FORWARD: return QString::fromUtf8("▶");
    case KC_REFRESH: return QString::fromUtf8("↻");
    case KC_MARKS:   return QString::fromUtf8("★");
    case KC_SCREENI: return QString::fromUtf8("☀+");
    case KC_SCREEND: return QString::fromUtf8("☀-");
    case KC_POWER:   return "Power";
    case KC_SLEEP:   return "Sleep";
    case KC_WAKE:    return "Wake";
    case KC_MIC:     return QString::fromUtf8("🎤");
    case KC_NO:      return QString();
    case KC_TRANSPARENT: return QString();
    }
    return QString("0x%1").arg(code, 4, 16, QChar('0'));
}

} // namespace


// ===========================================================================
// KeyDef::isMultimedia
// ===========================================================================
bool KeyDef::isMultimedia() const
{
    const uint16_t page = code & UT_MASK;
    return page == UT_CONSUMER || page == UT_DESK;
}


// ===========================================================================
// KeyLayout
// ===========================================================================
KeyLayout::KeyLayout() = default;

uint16_t KeyLayout::parseCode(const QString& s, bool* ok)
{
    const QString t = s.trimmed();
    if (ok) *ok = true;

    if (t.isEmpty()) return KC_NO;

    // 0x hex / decimal numeric
    if (t.startsWith("0x", Qt::CaseInsensitive)) {
        bool okHex = false;
        uint16_t v = t.mid(2).toUShort(&okHex, 16);
        if (okHex) return v;
    }

    // Try alias map (KC_ / FK_ prefix optional, case-insensitive).
    QString key = t.toUpper();
    if (key.startsWith("KC_")) key.remove(0, 3);
    else if (key.startsWith("FK_")) key.remove(0, 3);
    auto it = aliasMap().find(key);
    if (it != aliasMap().end()) return it.value();

    if (ok) *ok = false;
    qWarning() << "[KeyLayout] Unknown key code:" << s;
    return KC_NO;
}

QString KeyLayout::codeToShortName(uint16_t code)
{
    return defaultLabelFor(code);
}

QString KeyLayout::codeToHumanName(uint16_t code)
{
    switch (code) {
    // Consumer / multimedia — show a descriptive English name
    case KC_MUTE:    return "Mute (静音)";
    case KC_VINC:    return "Volume Up (音量+)";
    case KC_VDEC:    return "Volume Down (音量-)";
    case KC_PLAY:    return "Play/Pause (播放/暂停)";
    case KC_NTRACK:  return "Next Track (下一曲)";
    case KC_PTRACK:  return "Prev Track (上一曲)";
    case KC_CSTOP:   return "Stop (停止)";
    case KC_MEDIA:   return "Media Player (媒体播放器)";
    case KC_CALC:    return "Calculator (计算器)";
    case KC_EMAIL:   return "Email (邮件)";
    case KC_PC:      return "My Computer (我的电脑)";
    case KC_WWW:     return "Browser Home (浏览器主页)";
    case KC_SEARCH:  return "Browser Search (浏览器搜索)";
    case KC_AC_BACK: return "Browser Back (后退)";
    case KC_FORWARD: return "Browser Forward (前进)";
    case KC_REFRESH: return "Browser Refresh (刷新)";
    case KC_MARKS:   return "Bookmarks (收藏)";
    case KC_AC_STOP: return "Browser Stop";
    case KC_SCREENI: return "Brightness Up (亮度+)";
    case KC_SCREEND: return "Brightness Down (亮度-)";
    case KC_MIC:     return "Microphone (麦克风)";
    case KC_POWER:   return "Power (电源)";
    case KC_SLEEP:   return "Sleep (睡眠)";
    case KC_WAKE:    return "Wake (唤醒)";
    default: break;
    }
    return defaultLabelFor(code);
}

QSize KeyLayout::boundingPixelSize() const
{
    qreal maxX = 0, maxY = 0;
    for (const auto& k : m_keys) {
        maxX = qMax(maxX, k.x  + k.w );
        maxY = qMax(maxY, k.y  + k.h );
        if (k.hasSecond) {
            maxX = qMax(maxX, k.x2 + k.w2);
            maxY = qMax(maxY, k.y2 + k.h2);
        }
    }
    return QSize(int(maxX * m_unitPx) + 2 * m_margin,
                 int(maxY * m_unitPx) + 2 * m_margin);
}

bool KeyLayout::loadFromFile(const QString& path, QString* errMsg)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (errMsg) *errMsg = QString("Cannot open: %1").arg(path);
        return false;
    }
    const QByteArray bytes = f.readAll();
    f.close();

    // Pick parser by extension or by content sniff.
    const QString ext = QFileInfo(path).suffix().toLower();
    bool ok;
    if (ext == "c" || ext == "h" || ext == "txt") {
        ok = loadFromMatrixText(bytes, QFileInfo(path).baseName(), errMsg);
    } else {
        // JSON is the default; sniff for a leading '{' to be safe.
        const QByteArray trimmed = bytes.trimmed();
        if (!trimmed.isEmpty() && trimmed.at(0) == '{') {
            ok = loadFromJson(bytes, errMsg);
        } else {
            ok = loadFromMatrixText(bytes, QFileInfo(path).baseName(), errMsg);
        }
    }
    if (ok && m_name.isEmpty()) m_name = QFileInfo(path).baseName();
    return ok;
}

bool KeyLayout::loadFromJson(const QByteArray& bytes, QString* errMsg)
{
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &pe);
    if (pe.error != QJsonParseError::NoError) {
        if (errMsg) *errMsg = QString("JSON parse: %1 @ offset %2").arg(pe.errorString()).arg(pe.offset);
        return false;
    }
    if (!doc.isObject()) {
        if (errMsg) *errMsg = "Root must be an object";
        return false;
    }
    const QJsonObject root = doc.object();

    m_name = root.value("name").toString();
    if (root.contains("unit"))   m_unitPx = root.value("unit").toInt(54);
    if (root.contains("margin")) m_margin = root.value("margin").toInt(12);
    if (m_unitPx < 16) m_unitPx = 16;
    if (m_unitPx > 200) m_unitPx = 200;

    // ------------------------------------------------------------------
    // Matrix mode:  "matrix": [["KC_ESC","xxSK", ...], ...]
    //   optional   "widths": [[1,1,...], ...]
    // ------------------------------------------------------------------
    if (root.contains("matrix")) {
        const QJsonArray mat = root.value("matrix").toArray();
        if (mat.isEmpty()) {
            if (errMsg) *errMsg = "'matrix' must be a non-empty array of rows";
            return false;
        }
        QVector<QStringList> rows;
        rows.reserve(mat.size());
        for (const auto& rv : mat) {
            const QJsonArray ra = rv.toArray();
            QStringList row;
            row.reserve(ra.size());
            for (const auto& cv : ra) row << cv.toString();
            rows << row;
        }

        QVector<QVector<double>> widths;
        if (root.contains("widths")) {
            const QJsonArray wa = root.value("widths").toArray();
            for (const auto& rv : wa) {
                const QJsonArray ra = rv.toArray();
                QVector<double> row;
                row.reserve(ra.size());
                for (const auto& cv : ra) row << cv.toDouble(1.0);
                widths << row;
            }
        }

        if (!buildFromMatrix(rows, widths, errMsg)) return false;
        if (m_name.isEmpty()) m_name = "Untitled Matrix";
        return true;
    }

    // ------------------------------------------------------------------
    // KLE-style "keys" mode.
    // ------------------------------------------------------------------
    const QJsonArray keys  = root.value("keys").toArray();
    if (keys.isEmpty()) {
        if (errMsg) *errMsg = "Layout has neither 'keys' nor 'matrix'";
        return false;
    }

    m_keys.clear();
    m_keys.reserve(keys.size());
    for (const auto& v : keys) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        KeyDef k;
        k.x = o.value("x").toDouble(0.0);
        k.y = o.value("y").toDouble(0.0);
        k.w = o.value("w").toDouble(1.0);
        k.h = o.value("h").toDouble(1.0);

        if (o.contains("x2") || o.contains("y2") || o.contains("w2") || o.contains("h2")) {
            k.hasSecond = true;
            k.x2 = o.value("x2").toDouble(k.x);
            k.y2 = o.value("y2").toDouble(k.y);
            k.w2 = o.value("w2").toDouble(1.0);
            k.h2 = o.value("h2").toDouble(1.0);
        }

        k.codeName = o.value("code").toString();
        k.code     = parseCode(k.codeName);

        if (o.contains("label"))
            k.label = o.value("label").toString();
        else
            k.label = defaultLabelFor(k.code);

        k.row   = o.value("row").toInt(-1);
        k.col   = o.value("col").toInt(-1);
        k.decal = o.value("decal").toBool(false);

        m_keys.push_back(k);
    }

    if (m_name.isEmpty()) m_name = "Untitled Layout";
    return true;
}


// ===========================================================================
// Matrix / C-style text loader
// ===========================================================================
//
// Accepted input examples:
//
//   .Matrix = {
//       {KC_ESC,   xxSK,    KC_F1, ..., xxSK},
//       {KC_GRV,   KC_1,    KC_2,  ..., xxSK},
//       ...
//   }
//
// or simply:
//
//   {KC_ESC, xxSK, KC_F1, ...}
//   {KC_GRV, KC_1, KC_2, ...}
//
// Tokens: KC_XXX, FK_XXX, xxSK, ____ (lower- or upper-case). Trailing commas
// are tolerated; comments  // ...  and  /* ... */  are stripped.
//
bool KeyLayout::loadFromMatrixText(const QByteArray& text,
                                   const QString& name,
                                   QString* errMsg)
{
    QString src = QString::fromUtf8(text);

    // Strip block comments /* ... */
    {
        int p = 0;
        while ((p = src.indexOf("/*", p)) >= 0) {
            int q = src.indexOf("*/", p + 2);
            if (q < 0) { src.truncate(p); break; }
            src.remove(p, q - p + 2);
        }
    }
    // Strip line comments // ...
    {
        QStringList out;
        for (QString line : src.split('\n')) {
            const int p = line.indexOf("//");
            if (p >= 0) line.truncate(p);
            out << line;
        }
        src = out.join('\n');
    }

    // Find every brace-enclosed row that looks like a token list.
    // Match { ... } at the innermost level (no nested braces inside a row).
    QVector<QStringList> rows;
    int i = 0;
    while (i < src.size()) {
        const int open = src.indexOf('{', i);
        if (open < 0) break;
        const int close = src.indexOf('}', open + 1);
        if (close < 0) break;

        const QString inside = src.mid(open + 1, close - open - 1);
        // Skip nested-braces rows (e.g. the outer .Matrix = { … } block).
        if (inside.contains('{')) { i = open + 1; continue; }

        // Split into tokens, strip whitespace.
        QStringList tokens;
        for (const QString& raw : inside.split(',')) {
            const QString t = raw.trimmed();
            if (t.isEmpty()) continue;
            tokens << t;
        }

        // Heuristic: a "row" must have at least 3 tokens AND every token
        // must look like an identifier (KC_..., FK_..., xxSK, ____, etc.)
        static const QRegularExpression tokRe(R"(^[A-Za-z_][A-Za-z0-9_]*$)");
        bool looksLikeRow = tokens.size() >= 3;
        for (const QString& t : tokens) {
            if (!tokRe.match(t).hasMatch()) { looksLikeRow = false; break; }
        }
        if (looksLikeRow) rows << tokens;

        i = close + 1;
    }

    if (rows.isEmpty()) {
        if (errMsg) *errMsg = "No KC_* matrix rows found in input";
        return false;
    }

    m_name = name.isEmpty() ? QStringLiteral("Matrix Layout") : name;
    if (m_unitPx <= 0) m_unitPx = 54;
    if (m_margin < 0)  m_margin = 12;
    return buildFromMatrix(rows, /*widths*/{}, errMsg);
}


// Build keys from a 2-D token grid. Each cell is 1u × 1u at (col, row).
// Empty / spacer tokens (xxSK, ____, NONE, KC_NO) become invisible holes.
// `widths` is optional; if provided and parallel to `rows`, the value at
// (r,c) overrides the per-cell width (height stays 1u).
bool KeyLayout::buildFromMatrix(const QVector<QStringList>& rows,
                                const QVector<QVector<double>>& widths,
                                QString* errMsg)
{
    if (rows.isEmpty()) {
        if (errMsg) *errMsg = "Empty matrix";
        return false;
    }

    m_keys.clear();

    int maxCols = 0;
    for (const auto& r : rows) maxCols = qMax(maxCols, r.size());

    for (int r = 0; r < rows.size(); ++r) {
        const QStringList& row = rows[r];
        double cursorX = 0.0;
        for (int c = 0; c < row.size(); ++c) {
            const QString tok = row[c];
            const QString up  = tok.toUpper();
            const bool isHole =
                up == "XXSK" || up == "____" ||
                up == "KC_NO" || up == "NO" || up == "NONE" ||
                up == "_";

            double w = 1.0;
            if (r < widths.size() && c < widths[r].size())
                w = widths[r][c] > 0.0 ? widths[r][c] : 1.0;

            if (!isHole) {
                KeyDef k;
                k.x = cursorX;
                k.y = double(r);
                k.w = w;
                k.h = 1.0;
                k.codeName = tok;
                k.code     = parseCode(tok);
                k.label    = defaultLabelFor(k.code);
                if (k.label.isEmpty()) k.label = tok;     // fall-back: show raw token
                k.row = r;
                k.col = c;
                m_keys.push_back(k);
            }

            cursorX += w;
        }
    }

    if (m_keys.isEmpty()) {
        if (errMsg) *errMsg = "Matrix contained only empty cells";
        return false;
    }

    // Stash dimensions so the view can size itself.
    Q_UNUSED(maxCols);
    return true;
}
