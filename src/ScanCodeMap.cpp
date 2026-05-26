#include "ScanCodeMap.h"

extern "C" {
#include "Layout/HID_Usage_Tables.h"
}

namespace {

QHash<int, uint16_t> buildTable()
{
    QHash<int, uint16_t> m;
    const int EXT = 0x100;

    auto add = [&](int scan, uint16_t kc) { m.insert(scan, kc); };

    // -- Function row + ESC --
    add(0x01, KC_ESC);
    add(0x3B, KC_F1);  add(0x3C, KC_F2);  add(0x3D, KC_F3);  add(0x3E, KC_F4);
    add(0x3F, KC_F5);  add(0x40, KC_F6);  add(0x41, KC_F7);  add(0x42, KC_F8);
    add(0x43, KC_F9);  add(0x44, KC_F10); add(0x57, KC_F11); add(0x58, KC_F12);

    // -- Number row --
    add(0x29, KC_GRAVE);
    add(0x02, KC_1);  add(0x03, KC_2);  add(0x04, KC_3);  add(0x05, KC_4);
    add(0x06, KC_5);  add(0x07, KC_6);  add(0x08, KC_7);  add(0x09, KC_8);
    add(0x0A, KC_9);  add(0x0B, KC_0);
    add(0x0C, KC_MINS); add(0x0D, KC_EQUAL); add(0x0E, KC_BACKSPACE);

    // -- QWERTY row --
    add(0x0F, KC_TAB);
    add(0x10, KC_Q); add(0x11, KC_W); add(0x12, KC_E); add(0x13, KC_R);
    add(0x14, KC_T); add(0x15, KC_Y); add(0x16, KC_U); add(0x17, KC_I);
    add(0x18, KC_O); add(0x19, KC_P);
    add(0x1A, KC_LEFT_BRACKET); add(0x1B, KC_RIGHT_BRACKET);
    add(0x2B, KC_BACKSLASH);

    // -- ASDF row --
    add(0x3A, KC_CAPS);
    add(0x1E, KC_A); add(0x1F, KC_S); add(0x20, KC_D); add(0x21, KC_F);
    add(0x22, KC_G); add(0x23, KC_H); add(0x24, KC_J); add(0x25, KC_K);
    add(0x26, KC_L);
    add(0x27, KC_SEMICOLON); add(0x28, KC_QUOTE);
    add(0x1C, KC_ENTER);

    // -- ZXCV row --
    add(0x2A, KC_LSFT);
    add(0x2C, KC_Z); add(0x2D, KC_X); add(0x2E, KC_C); add(0x2F, KC_V);
    add(0x30, KC_B); add(0x31, KC_N); add(0x32, KC_M);
    add(0x33, KC_COMMA); add(0x34, KC_DOT); add(0x35, KC_SLASH);
    add(0x36, KC_RSFT);
    add(0x36 + EXT, KC_RSFT);          // some keyboards mark RShift as "extended"
    add(0x56, KC_NONUS_BACKSLASH);     // ISO key (between LSHIFT and Z)

    // -- Bottom row --
    add(0x1D, KC_LCTL);
    add(0x1D + EXT, KC_RCTL);
    add(0x38, KC_LALT);
    add(0x38 + EXT, KC_RALT);
    add(0x5B + EXT, KC_LGUI);
    add(0x5C + EXT, KC_RGUI);
    add(0x5D + EXT, KC_APPLICATION);   // Menu / App
    add(0x39, KC_SPACE);

    // -- Navigation cluster (extended) --
    add(0x52 + EXT, KC_INSERT);
    add(0x53 + EXT, KC_DELETE);
    add(0x47 + EXT, KC_HOME);
    add(0x4F + EXT, KC_END);
    add(0x49 + EXT, KC_PAGE_UP);
    add(0x51 + EXT, KC_PAGE_DOWN);

    // -- Arrows --
    add(0x48 + EXT, KC_UP);
    add(0x50 + EXT, KC_DOWN);
    add(0x4B + EXT, KC_LEFT);
    add(0x4D + EXT, KC_RIGHT);

    // -- Top right cluster --
    add(0x37 + EXT, KC_PRINT_SCREEN);
    add(0x46, KC_SCROLL_LOCK);
    add(0x45, KC_PAUSE);
    // Pause produces unusual codes; KeyHook normalises to 0x5B+EXT or 0x45.
    // Older KeyHook remapped Pause to 0x15B which is left-Win; we use the
    // sane 0x45 / 0x46 mapping above. (Real Pause hardware sequence is rare.)

    // -- Numpad --
    add(0x45 + EXT, KC_NUM_LOCK);      // Extended NumLock from some keyboards
    add(0x35 + EXT, KC_KP_SLASH);
    add(0x37, KC_KP_ASTERISK);
    add(0x4A, KC_KP_MINUS);
    add(0x4E, KC_KP_PLUS);
    add(0x1C + EXT, KC_KP_ENTER);
    add(0x52, KC_KP_0);
    add(0x4F, KC_KP_1); add(0x50, KC_KP_2); add(0x51, KC_KP_3);
    add(0x4B, KC_KP_4); add(0x4C, KC_KP_5); add(0x4D, KC_KP_6);
    add(0x47, KC_KP_7); add(0x48, KC_KP_8); add(0x49, KC_KP_9);
    add(0x53, KC_KP_DOT);

    // -- Multimedia (Consumer page) — KeyHook normalises scancode-0 to these
    add(0x10 + EXT, KC_PTRACK);      // Prev Track
    add(0x19 + EXT, KC_NTRACK);      // Next Track
    add(0x20 + EXT, KC_MUTE);        // Mute
    add(0x22 + EXT, KC_PLAY);        // Play/Pause
    add(0x24 + EXT, KC_CSTOP);       // Stop
    add(0x2E + EXT, KC_VDEC);        // Volume Down
    add(0x30 + EXT, KC_VINC);        // Volume Up

    // -- Browser / launcher --
    add(0x65 + EXT, KC_SEARCH);      // Browser Search
    add(0x66 + EXT, KC_MARKS);       // Bookmarks
    add(0x67 + EXT, KC_REFRESH);     // Refresh
    add(0x32 + EXT, KC_WWW);         // Browser Home
    add(0x68 + EXT, KC_AC_STOP);     // Browser Stop
    add(0x69 + EXT, KC_FORWARD);     // Browser Forward
    add(0x6A + EXT, KC_AC_BACK);     // Browser Back
    add(0x6C + EXT, KC_EMAIL);       // Mail
    add(0x6B + EXT, KC_PC);          // My Computer / files
    add(0x21 + EXT, KC_CALC);        // Calculator (some kb)
    add(0x6D + EXT, KC_MEDIA);       // Media-player launcher

    // -- Power / sleep / wake (system control page) --
    add(0x5E + EXT, KC_POWER);
    add(0x5F + EXT, KC_SLEEP);
    add(0x63 + EXT, KC_WAKE);

    return m;
}

} // namespace

const QHash<int, uint16_t>& ScanCodeMap::table()
{
    static const QHash<int, uint16_t> t = buildTable();
    return t;
}

uint16_t ScanCodeMap::scanToKc(int scan)
{
    return table().value(scan, (uint16_t)KC_NO);
}

int ScanCodeMap::kcToScan(uint16_t code)
{
    static QHash<uint16_t, int> rev;
    if (rev.isEmpty()) {
        const auto& t = table();
        for (auto it = t.begin(); it != t.end(); ++it)
            rev.insert(it.value(), it.key());
    }
    return rev.value(code, 0);
}
