/*
 * FS370 / WP98S — 98 配列 (Magnetic K98) Win layer
 *   #define OHID_BOARD_FS370_WP98S OHID_BOARD_ID(OHID_DEVICE_KEYBOARD, OHID_KB_MAGNETIC_K98, 0x11)
 *   VID 0x36F9  PID 0xAB06
 *
 * 这个 .c 片段直接粘自 OHID_Layout.c 的 .FN0.Matrix 字段,
 * KeyboardTesterV2 会自动识别每一行 { ... } 并构建网格.
 */

.FN0.Matrix = {  // Win
    // COL1     COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL11    COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21
    {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_DEL,   KC_INS,   KC_PGUP,   KC_PGDN,  xxSK,     xxSK,     xxSK},
    {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_NUM,   KC_PSLS,  KC_PAST,   KC_PMNS,  xxSK,     xxSK,     xxSK},
    {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_P7,    KC_P8,    KC_P9,     KC_PPLS,  xxSK,     xxSK,     xxSK},
    {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_P4,    KC_P5,    KC_P6,     xxSK,     xxSK,     xxSK,     xxSK},
    {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_P1,    KC_P2,    KC_P3,     KC_PENT,  xxSK,     xxSK,     xxSK},
    {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, KC_P0,    KC_KP_DOT, xxSK,     xxSK,     xxSK,     xxSK},
}
