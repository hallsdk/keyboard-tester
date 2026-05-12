#ifndef SCANCODEMAP_H
#define SCANCODEMAP_H

#include <QHash>
#include <cstdint>

// Build-once mapping between Windows PS/2 scan codes (with 0x100 added for
// LLKHF_EXTENDED) and OHID KC_* codes — including the consumer-page
// multimedia keys (KC_MUTE, KC_VINC, KC_PLAY, …).
//
// KeyHook is expected to follow the convention used by the legacy project:
//   scanCode = pkbhs->scanCode (+ 0x100 if extended)
//   for keys whose scanCode comes through as 0, KeyHook substitutes the
//   well-known PS/2 codes (e.g. VK_VOLUME_UP → 0x130).
//
// scanToKc(scan)  → KC_NO if unknown
// kcToScan(code)  → 0     if unknown
class ScanCodeMap
{
public:
    static uint16_t scanToKc(int scan);
    static int      kcToScan(uint16_t code);

    static const QHash<int, uint16_t>& table();
};

#endif // SCANCODEMAP_H
