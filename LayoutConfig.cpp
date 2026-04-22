#include "LayoutConfig.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QMap>

#include "Layout/HID_Usage_Tables.h"

bool                                LayoutConfig::s_loaded = false;
QMap<uint32_t, LayoutConfig::Entry> LayoutConfig::s_byId;
LayoutConfig::Entry                 LayoutConfig::s_default;
bool                                LayoutConfig::s_hasDefault = false;

// ---------------- Token → driverCode mapping ----------------
// Tokens are case-insensitive. Empty string or "_" / "-" / "xxSK" → xxSK.
uint16_t LayoutConfig::tokenToCode(const QString& tokenIn)
{
    const QString t = tokenIn.trimmed().toUpper();
    if (t.isEmpty() || t == "_" || t == "-" || t == "XXSK" || t == "NONE")
        return (uint16_t)xxSK;

    static const QMap<QString, uint16_t> kMap = {
        // Letters
        {"A",KC_A},{"B",KC_B},{"C",KC_C},{"D",KC_D},{"E",KC_E},{"F",KC_F},{"G",KC_G},
        {"H",KC_H},{"I",KC_I},{"J",KC_J},{"K",KC_K},{"L",KC_L},{"M",KC_M},{"N",KC_N},
        {"O",KC_O},{"P",KC_P},{"Q",KC_Q},{"R",KC_R},{"S",KC_S},{"T",KC_T},{"U",KC_U},
        {"V",KC_V},{"W",KC_W},{"X",KC_X},{"Y",KC_Y},{"Z",KC_Z},
        // Digits
        {"1",KC_1},{"2",KC_2},{"3",KC_3},{"4",KC_4},{"5",KC_5},
        {"6",KC_6},{"7",KC_7},{"8",KC_8},{"9",KC_9},{"0",KC_0},
        // F-keys
        {"F1",KC_F1},{"F2",KC_F2},{"F3",KC_F3},{"F4",KC_F4},
        {"F5",KC_F5},{"F6",KC_F6},{"F7",KC_F7},{"F8",KC_F8},
        {"F9",KC_F9},{"F10",KC_F10},{"F11",KC_F11},{"F12",KC_F12},
        // Punctuation / common keys
        {"ESC",KC_ESC},{"TAB",KC_TAB},{"CAPS",KC_CAPS},{"ENTER",KC_ENTER},{"ENT",KC_ENTER},
        {"SPACE",KC_SPACE},{"SPC",KC_SPACE},
        {"BKSP",KC_BACKSPACE},{"BSPC",KC_BACKSPACE},
        {"MINS",KC_MINS},{"-",KC_MINS},
        {"EQL",KC_EQUAL},{"=",KC_EQUAL},
        {"LBRC",KC_LEFT_BRACKET},{"[",KC_LEFT_BRACKET},
        {"RBRC",KC_RIGHT_BRACKET},{"]",KC_RIGHT_BRACKET},
        {"BSLS",KC_BACKSLASH},
        {"SCLN",KC_SEMICOLON},{";",KC_SEMICOLON},
        {"QUOT",KC_QUOTE},{"'",KC_QUOTE},
        {"GRV",KC_GRAVE},{"`",KC_GRAVE},
        {"COMM",KC_COMMA},{",",KC_COMMA},
        {"DOT",KC_DOT},{".",KC_DOT},
        {"SLSH",KC_SLASH},{"/",KC_SLASH},
        // Modifiers
        {"LSFT",KC_LSFT},{"RSFT",KC_RSFT},
        {"LCTL",KC_LCTL},{"RCTL",KC_RCTL},
        {"LALT",KC_LALT},{"RALT",KC_RALT},
        {"LGUI",KC_LGUI},{"RGUI",KC_RGUI},{"LWIN",KC_LGUI},{"RWIN",KC_RGUI},
        {"MENU",KC_MENU},{"APP",KC_MENU},
        // Navigation
        {"INS",KC_INSERT},{"INSERT",KC_INSERT},
        {"DEL",KC_DELETE},{"DELETE",KC_DELETE},
        {"HOME",KC_HOME},{"END",KC_END},
        {"PGUP",KC_PAGE_UP},{"PGDN",KC_PAGE_DOWN},
        {"UP",KC_UP},{"DOWN",KC_DOWN},{"LEFT",KC_LEFT},{"RIGHT",KC_RIGHT},
        {"PRTSC",KC_PRINT_SCREEN},{"PSCR",KC_PRINT_SCREEN},
        {"SCRLK",KC_SCROLL_LOCK},{"SLCK",KC_SCROLL_LOCK},
        {"PAUSE",KC_PAUSE},{"PAUS",KC_PAUSE},
        {"NUM",KC_NUM_LOCK},{"NLCK",KC_NUM_LOCK},
        // Numpad
        {"KP0",KC_KP_0},{"KP1",KC_KP_1},{"KP2",KC_KP_2},{"KP3",KC_KP_3},{"KP4",KC_KP_4},
        {"KP5",KC_KP_5},{"KP6",KC_KP_6},{"KP7",KC_KP_7},{"KP8",KC_KP_8},{"KP9",KC_KP_9},
        {"KP_DOT",KC_KP_DOT},{"KP_PLUS",KC_KP_PLUS},{"KP_MINUS",KC_KP_MINUS},
        {"KP_MUL",KC_KP_ASTERISK},{"KP_AST",KC_KP_ASTERISK},
        {"KP_DIV",KC_KP_SLASH},{"KP_ENTER",KC_KP_ENTER},
        // Function layer key
        {"FN",FK_FN1},{"FN1",FK_FN1},{"FK_FN1",FK_FN1},
    };

    auto it = kMap.find(t);
    if (it != kMap.end())
        return it.value();

    // Fallback: allow raw hex like "0x0029"
    bool ok = false;
    uint16_t v = t.toUShort(&ok, 16);
    if (ok) return v;

    qWarning() << "[LayoutConfig] Unknown token:" << tokenIn;
    return (uint16_t)xxSK;
}

bool LayoutConfig::parseEntry(const QJsonObject& obj, Entry& out)
{
    // init all to xxSK
    for (int r = 0; r < TKB_ROWS; ++r)
        for (int c = 0; c < TKB_COLS; ++c)
            out.matrix.Matrix[r][c] = (uint16_t)xxSK;

    const QJsonArray rows = obj.value("rows").toArray();
    if (rows.isEmpty()) {
        qWarning() << "[LayoutConfig] entry missing 'rows'";
        return false;
    }

    for (int r = 0; r < rows.size() && r < TKB_ROWS; ++r) {
        const QJsonArray cols = rows.at(r).toArray();
        for (int c = 0; c < cols.size() && c < TKB_COLS; ++c) {
            out.matrix.Matrix[r][c] = tokenToCode(cols.at(c).toString());
        }
    }

    // Optional spacings: { "cols":[..], "rows":[..] }
    if (obj.contains("spacings")) {
        const QJsonObject sp = obj.value("spacings").toObject();
        const QJsonArray colsArr = sp.value("cols").toArray();
        const QJsonArray rowsArr = sp.value("rows").toArray();
        out.colSpacings.clear();
        out.rowSpacings.clear();
        for (const auto& v : colsArr) out.colSpacings.push_back(v.toInt(1));
        for (const auto& v : rowsArr) out.rowSpacings.push_back(v.toInt(1));
        out.hasSpacings = !out.colSpacings.isEmpty() || !out.rowSpacings.isEmpty();
    }

    // Optional key_gaps: { "row,col": gapValue, ... }
    // gapValue 单位为 1列宽, 表示该键左侧额外插入的空白距离.
    out.keyGaps.clear();
    if (obj.contains("key_gaps")) {
        const QJsonObject gapObj = obj.value("key_gaps").toObject();
        for (auto it = gapObj.begin(); it != gapObj.end(); ++it) {
            const QStringList parts = it.key().split(',');
            if (parts.size() != 2) continue;
            bool ok1 = false, ok2 = false;
            int gr = parts[0].trimmed().toInt(&ok1);
            int gc = parts[1].trimmed().toInt(&ok2);
            if (!ok1 || !ok2 || gr < 0 || gr >= TKB_ROWS || gc < 0 || gc >= TKB_COLS) continue;
            out.keyGaps.insert({gr, gc}, it.value().toDouble(0.0));
        }
    }
    return true;
}

void LayoutConfig::load()
{
    s_byId.clear();
    s_hasDefault = false;
    s_loaded = true;

    const QString path = QCoreApplication::applicationDirPath() + "/config.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "[LayoutConfig] Cannot open" << path;
        return;
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "[LayoutConfig] JSON parse error:" << err.errorString();
        return;
    }

    const QJsonObject root = doc.object();
    const QJsonObject layouts = root.value("Layouts").toObject();

    for (auto it = layouts.begin(); it != layouts.end(); ++it) {
        const QString key = it.key();
        const QJsonObject obj = it.value().toObject();
        Entry e;
        if (!parseEntry(obj, e)) continue;

        if (key.compare("default", Qt::CaseInsensitive) == 0) {
            s_default = e;
            s_hasDefault = true;
        } else {
            bool ok = false;
            uint32_t id = key.toUInt(&ok, 16);
            if (!ok) {
                qWarning() << "[LayoutConfig] Bad board_id key:" << key;
                continue;
            }
            s_byId.insert(id, e);
        }
    }

    qDebug() << "[LayoutConfig] loaded" << s_byId.size()
             << "board layouts, hasDefault=" << s_hasDefault;
}

void LayoutConfig::ensureLoaded()
{
    if (!s_loaded) load();
}

const LayoutConfig::Entry& LayoutConfig::resolve(uint32_t board_id)
{
    ensureLoaded();
    auto it = s_byId.find(board_id);
    if (it != s_byId.end())
        return it.value();

    if (s_hasDefault)
        return s_default;

    // Fallback: build an Entry from the hardcoded OHID layout.
    static Entry fallback;
    static bool  fbInit = false;
    static uint32_t fbBoard = 0;
    if (!fbInit || fbBoard != board_id) {
        const tkb_Half_matrix_t* hc = OHID_board_layout_Get(board_id);
        if (hc) {
            fallback.matrix = *hc;
        } else {
            for (int r = 0; r < TKB_ROWS; ++r)
                for (int c = 0; c < TKB_COLS; ++c)
                    fallback.matrix.Matrix[r][c] = (uint16_t)xxSK;
        }
        fallback.colSpacings.clear();
        fallback.rowSpacings.clear();
        fallback.keyGaps.clear();
        fallback.hasSpacings = false;
        fbInit = true;
        fbBoard = board_id;
    }
    return fallback;
}

const tkb_Half_matrix_t* LayoutConfig::resolveMatrix(uint32_t board_id)
{
    return &resolve(board_id).matrix;
}
