/************************ (C) COPYLEFT 2018 Merafour *************************
* File Name          : OHID_Layout.c
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.0.0
* Last Modified Date : 2024.04.13
* Description        : OHID_Layout.
* Description        : 如无必要,勿增实体.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#include <stdint.h>
#include "OHID.h"
#include "OHID/OHID_Board.h"
#include "Layout/OHID_Layout.h"
#include "Layout/HID_Usage_Tables.h"
#include <stdlib.h>
#include <stdio.h>
// #include <QDebug>
static const struct tkb_Half_matrix_t TK51Q = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21  EC
      {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_PSCR,  KC_SCRL,  KC_PAUS,   KC_VDEC,  KC_MEDIA, KC_PLAY,  KC_VINC},
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   KC_HOME,  KC_PGUP,   KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   KC_END,   KC_PGDN,   KC_P7,    KC_P8,    KC_P9,    KC_PPLS},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      xxSK,     xxSK,     xxSK,      KC_P4,    KC_P5,    KC_P6,    xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   xxSK,        xxSK,     KC_UP,    xxSK,      KC_P1,    KC_P2,    KC_P3,    KC_PENT},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_APP,    KC_RCTL,     KC_LEFT,  KC_DOWN,  KC_RIGHT,  KC_P0,    xxSK,     KC_KP_DOT,xxSK},
    },
};
static const struct tkb_Half_matrix_t K104 = {
    // 标准 104键
    .Matrix = {
        {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_PSCR,  KC_SCRL,  KC_PAUSE,  xxSK,    xxSK,    xxSK,    xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   KC_HOME,  KC_PGUP,   KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   KC_END,   KC_PGDN,   KC_P7,   KC_P8,   KC_P9,   KC_PPLS},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      xxSK,     xxSK,     xxSK,      KC_P4,   KC_P5,   KC_P6,   xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, xxSK,      KC_RSFT,      xxSK,     KC_UP,    xxSK,      KC_P1,   KC_P2,   KC_P3,   KC_PENT},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_APP,    KC_RCTL,     KC_LEFT,  KC_DOWN,  KC_RIGHT,  KC_P0,   xxSK,    KC_PDOT, xxSK},
    },
    // 标准 104键
//    .Names = {
//        {"Esc",     "",        "F1",      "F2",      "F3",     "F4",       "F5",         "F6",     "F7",     "F8",       "F9",     "F10",   "F11",     "F12",       "PrtSc",  "ScrLK",  "Pause",   "",     "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Insert", "Home",   "PgUp",    "Num",  "/",      "*",     "-"},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "Delete", "End",    "PgDN",    "7",    "8",      "9",     "+"},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "",       "",       "",        "4",    "5",      "6",     ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "",        "RShift",    "",       "Up",     "",        "1",    "2",      "3",     "Enter"},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "",         "RAlt",   "Fn",    "Menu",    "RCtrl",     "Left",   "Down",   "Right",   "0",     "",      ".",     ""},
//    },
};
static const struct tkb_Half_matrix_t TK51QHS370 = {
    // 标准 104键
    .Matrix = { // Win
                // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL11   COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21  EC
                {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_PSCR,  KC_SCRL,  KC_PAUS,   KC_CALC,  KC_MEDIA, xxSK,     xxSK},
                {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   KC_HOME,  KC_PGUP,   KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS},
                {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   KC_END,   KC_PGDN,   KC_P7,    KC_P8,    KC_P9,    KC_PPLS},
                {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      xxSK,     xxSK,     xxSK,      KC_P4,    KC_P5,    KC_P6,    xxSK},
                {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,      xxSK,     xxSK,     KC_UP,    xxSK,      KC_P1,    KC_P2,    KC_P3,    KC_PENT},
                {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_APP,    KC_RCTL,     KC_LEFT,  KC_DOWN,  KC_RIGHT,  KC_P0,    xxSK,     KC_KP_DOT,xxSK},
              },
    // 标准 104键
//    .Names = {
//        {"Esc",     "",        "F1",      "F2",      "F3",     "F4",       "F5",         "F6",     "F7",     "F8",       "F9",     "F10",   "F11",     "F12",       "PrtSc",  "ScrLK",  "Pause",   "",     "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Insert", "Home",   "PgUp",    "Num",  "/",      "*",     "-"},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "Delete", "End",    "PgDN",    "7",    "8",      "9",     "+"},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "",       "",       "",        "4",    "5",      "6",     ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "",        "RShift",    "",       "Up",     "",        "1",    "2",      "3",     "Enter"},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "",         "RAlt",   "Fn",    "Menu",    "RCtrl",     "Left",   "Down",   "Right",   "0",     "",      ".",     ""},
//    },
};
static const struct tkb_Half_matrix_t K99 = {
    .Matrix = {
        {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_DEL,   KC_INS,   KC_PGUP,   KC_PGDN,  xxSK,    xxSK,    xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_NUM,   KC_PSLS,  KC_PAST,   KC_PMNS,  xxSK,    xxSK,    xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_P7,    KC_P8,    KC_P9,     KC_PPLS,  xxSK,    xxSK,    xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_P4,    KC_P5,    KC_P6,     xxSK,     xxSK,    xxSK,    xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_P1,    KC_P2,    KC_P3,     KC_PENT,  xxSK,    xxSK,    xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, KC_P0,    KC_PDOT,   xxSK,     xxSK,    xxSK,    xxSK},
    },
//    .Names = {
//        {"Esc",     "",        "F1",      "F2",      "F3",     "F4",       "F5",         "F6",     "F7",     "F8",       "F9",     "F10",   "F11",     "F12",       "Del",    "Ins",    "PgUp",    "PgDN", "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Num",    "/",      "*",       "-",    "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "7",      "8",      "9",       "+",    "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "4",      "5",      "6",       "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "RShift",  "Up",        "1",      "2",      "3",       "Enter","",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "",         "Fn",     "RCtrl", "Left",    "Down",      "Right",   "0",     ".",       "0",    "",       "",      ""},
//    },
};

static const struct tkb_Half_matrix_t WP98 = {
    .Matrix = { // Win
        // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL11    COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21  EC
        {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_DEL,   KC_INS,   KC_PGUP,   KC_PGDN,  xxSK,     xxSK,     xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_NUM,   KC_PSLS,  KC_PAST,   KC_PMNS,  xxSK,     KC_HOME,     xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_P7,    KC_P8,    KC_P9,     KC_PPLS,  xxSK,     KC_END,     xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_P4,    KC_P5,    KC_P6,     xxSK,     xxSK,     xxSK,     xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_P1,    KC_P2,    KC_P3,     KC_PENT,  xxSK,     xxSK,     xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, KC_P0,    KC_KP_DOT, xxSK,     xxSK,     xxSK,     xxSK},
      },

};

static const struct tkb_Half_matrix_t MAG98_51WH = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21  EC
      {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_HOME,  KC_SCRL,  KC_INS,    xxSK,     xxSK,     xxSK,     xxSK},
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_PGUP,  KC_NUM,   KC_PSLS,   KC_PAST,  KC_PMNS,  xxSK,     xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_PGDN,  KC_P7,    KC_P8,     KC_P9,    KC_PPLS,  xxSK,     xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_DEL,   KC_P4,    KC_P5,     KC_P6,    xxSK,     xxSK,     xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, xxSK,      KC_RSFT,     KC_UP,    KC_P1,    KC_P2,     KC_P3,    KC_PENT,  xxSK,     xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_RCTL,   KC_LEFT,     KC_DOWN,  KC_RIGHT, KC_P0,     KC_KP_DOT,xxSK,     xxSK,     xxSK},
    },
};

static const struct tkb_Half_matrix_t V98H = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18    COL19    COL20    COL21  EC
      {KC_ESC,    KC_F1,     KC_F2,     KC_F3,     KC_F4,    KC_F5,      KC_F6,        KC_F7,    KC_F8,    KC_F9,       KC_F10,  KC_F11,  KC_F12,    KC_INS,      KC_DEL,   KC_PGUP,  KC_PGDN,   xxSK,    xxSK,    xxSK,    xxSK},//,  KC_VDEC},
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    xxSK,        KC_BSPC,  KC_NUM,   KC_PSLS,   KC_PAST, KC_PMNS, xxSK,    xxSK},//,  KC_VINC},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   xxSK,        KC_BSLS,  KC_P7,    KC_P8,     KC_P9,   KC_PPLS, xxSK,    xxSK},//,  KC_MUTE},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      xxSK,     KC_P4,    KC_P5,     KC_P6,   xxSK,    xxSK,    xxSK},//,     xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   xxSK,        KC_UP,    KC_P1,    KC_P2,     KC_P3,   KC_PENT, xxSK,    xxSK},//,     xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, xxSK,      KC_LEFT,     KC_DOWN,  KC_RIGHT, KC_P0,     KC_PDOT, xxSK,    xxSK,    xxSK},//,     xxSK},
    },
//    .Names = {
//        {"Esc",     "",        "F1",      "F2",      "F3",     "F4",       "F5",         "F6",     "F7",     "F8",       "F9",     "F10",   "F11",     "F12",       "Del",    "Ins",    "PgUp",    "PgDN", "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Num",    "/",      "*",       "-",    "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "7",      "8",      "9",       "+",    "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "4",      "5",      "6",       "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "RShift",  "Up",        "1",      "2",      "3",       "Enter","",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "",         "Fn",     "RCtrl", "Left",    "Down",      "Right",   "0",     ".",       "0",    "",       "",      ""},
//    },
};
static const struct tkb_Half_matrix_t TK50T = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21  EC
      {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_DEL,   KC_INS,   KC_PGUP,   KC_PGDN,  xxSK,     xxSK,     xxSK},
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_NUM,   KC_PSLS,  KC_PAST,   KC_PMNS,  xxSK,     xxSK,     xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_P7,    KC_P8,    KC_P9,     KC_PPLS,  xxSK,     xxSK,     xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_P4,    KC_P5,    KC_P6,     xxSK,     xxSK,     xxSK,     xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_P1,    KC_P2,    KC_P3,     KC_PENT,  xxSK,     xxSK,     xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  xxSK,    KC_LEFT,   KC_DOWN,     KC_RIGHT, KC_P0,    KC_KP_DOT, xxSK,     xxSK,     xxSK,     xxSK},
    },
};

// 75
static const struct tkb_Half_matrix_t TK51GF = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18    COL19    COL20    COL21  EC
      {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_HOME,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
};
static const struct tkb_Half_matrix_t ZONEX75 = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18    COL19    COL20    COL21  EC
      {KC_ESC,    KC_F1,     KC_F2,     KC_F3,     KC_F4,    KC_F5,      KC_F6,        KC_F7,    KC_F8,    KC_F9,       KC_F10,  KC_F11,  KC_F12,    KC_EC,       ____,     ____,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK}, // KC_EC
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
};
static const struct tkb_Half_matrix_t CK8971 = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18    COL19    COL20    COL21  EC
      {KC_ESC,    KC_F1,     KC_F2,     KC_F3,     KC_F4,    KC_F5,      KC_F6,        KC_F7,    KC_F8,    KC_F9,       KC_F10,  KC_F11,  KC_F12,    KC_EC,       KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK}, // KC_EC
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_END,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
};
static const struct tkb_Half_matrix_t BK75_370 = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18    COL19    COL20    COL21  EC
      {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_END,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
};
static const struct tkb_Half_matrix_t TK52L_370 = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18    COL19    COL20    COL21  EC
      {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
};
static const struct tkb_Half_matrix_t K87 = {
    .Matrix = {
        {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_PSCR,  KC_SCRL,  KC_PAUSE,  xxSK,    xxSK,    xxSK,    xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   KC_HOME,  KC_PGUP,   xxSK,    xxSK,    xxSK,    xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   KC_END,   KC_PGDN,   xxSK,    xxSK,    xxSK,    xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, xxSK,      KC_RSFT,     xxSK,     KC_UP,    xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_APP,    KC_RCTL,     KC_LEFT,  KC_DOWN,  KC_RIGHT,  xxSK,    xxSK,    xxSK,    xxSK},
    },
//    .Names = {
//        {"Esc",     "",        "F1",      "F2",      "F3",     "F4",       "F5",         "F6",     "F7",     "F8",       "F9",     "F10",   "F11",     "F12",       "PrtSc",  "ScrLK",  "Pause",   "",     "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Insert", "Home",   "PgUp",    "",     "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "Delete", "End",    "PgDN",    "",     "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "",       "",       "",        "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "",        "RShift",    "",       "Up",     "",        "",     "",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "",         "RAlt",   "Fn",    "Menu",    "RCtrl",     "Left",   "Down",   "Right",   "",     "",       "",      ""},
//    },
};
static const struct tkb_Half_matrix_t K84 = {
    .Matrix = {
        {KC_ESC,    KC_F1,     KC_F2,     KC_F3,     KC_F4,    KC_F5,      KC_F6,        KC_F7,    KC_F8,    KC_F9,       KC_F10,  KC_F11,  KC_F12,    KC_PSCR,     xxSK,     KC_DEL,   xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     xxSK,     KC_HOME,  xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     xxSK,     KC_PGUP,  xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      xxSK,     KC_PGDN,  xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, xxSK,      KC_RSFT,     KC_UP,    KC_END,   xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_RCTL,   KC_LEFT,     KC_DOWN,  KC_RIGHT, xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
//    .Names = {
//        {"Esc",     "F1",      "F2",      "F3",      "F4",     "F5",       "F6",         "F7",     "F8",     "F9",       "F10",    "F11",   "F12",     "PrtSc",     "",       "Del",    "",        "",     "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "",       "Home",   "",        "",     "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "",       "PgUp",   "",        "",     "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "",        "Enter",     "",       "PgDN",   "",        "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "",        "RShift",    "Up",     "End",    "",        "",     "",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "",         "RAlt",   "Fn",    "RCtrl",   "Left",      "Down",   "Right",   "",        "",    "",       "",      ""},
//    },
};
static const struct tkb_Half_matrix_t K82 = {
    .Matrix = {
        {KC_ESC,    KC_F1,     KC_F2,     KC_F3,     KC_F4,    KC_F5,      KC_F6,        KC_F7,    KC_F8,    KC_F9,       KC_F10,  KC_F11,  KC_F12,    KC_DEL,      xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     xxSK,     KC_HOME,  xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     xxSK,     KC_PGUP,  xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      xxSK,     KC_PGDN,  xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, xxSK,      KC_RSFT,     KC_UP,    KC_END,   xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_RCTL,   KC_LEFT,     KC_DOWN,  KC_RIGHT, xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
//    .Names = {
//        {"Esc",     "F1",      "F2",      "F3",      "F4",     "F5",       "F6",         "F7",     "F8",     "F9",       "F10",    "F11",   "F12",     "Del",       "",       "Del",    "",        "",     "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "",       "Home",   "",        "",     "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "",       "PgUp",   "",        "",     "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "",        "Enter",     "",       "PgDN",   "",        "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "",        "RShift",    "Up",     "End",    "",        "",     "",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "",         "RAlt",   "Fn",    "RCtrl",   "Left",      "Down",   "Right",   "",        "",    "",       "",      ""},
//    },
};
static const struct tkb_Half_matrix_t K81A = {
    .Matrix = {
        {KC_ESC,    KC_F1,     KC_F2,     KC_F3,     KC_F4,    KC_F5,      KC_F6,        KC_F7,    KC_F8,    KC_F9,       KC_F10,  KC_F11,  KC_F12,    KC_INS,      xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
//    .Names = {
//        {"Esc",     "F1",      "F2",      "F3",      "F4",     "F5",       "F6",         "F7",     "F8",     "F9",       "F10",    "F11",   "F12",     "Ins",       "",       "",       "",        "",     "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Del",    "",       "",        "",     "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "PgUp",   "",       "",        "",     "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "PgDN",   "",       "",        "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "RShift",  "Up",        "",       "",       "",        "",     "",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "RAlt",     "Fn",     "RCtrl", "Left",    "Down",      "Right",  "",       "",        "",     "",       "",      ""},
//    },
};
static const struct tkb_Half_matrix_t K81B = {
    .Matrix = {
        {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_INS,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
//    .Names = {
//        {"Esc",     "",        "F1",      "F2",      "F3",     "F4",       "F5",         "F6",     "F7",     "F8",       "F9",     "F10",   "F11",     "F12",       "",       "",       "",   "",     "",       "",      ""},
//        {"~"  ,     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Del",    "",       "",   "",     "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "Ins",    "",       "",   "",     "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "PgUp",   "",       "",   "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "",        "RShift",    "PgDN",   "",       "",   "",     "",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "RAlt",     "Fn",     "RCtrl", "Left",    "Down",      "Right",  "",       "",   "",     "",       "",      ""},
//    },
};
static const struct tkb_Half_matrix_t K68 = {
    .Matrix = {
        {xxSK,      xxSK,      xxSK,      xxSK,      xxSK,     xxSK,       xxSK,         xxSK,     xxSK,     xxSK,        xxSK,    xxSK,    xxSK,      xxSK,        xxSK,     xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_ESC,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGUP,  xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_PGDN,  xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_LCTL,   KC_LSFT,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  xxSK,    KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
    },
//    .Names = {
//        {"",        "",        "",        "",        "",       "",         "",           "",       "",       "",         "",       "",      "",        "",          "",       "",       "",        "",     "",       "",      ""},
//        {"Esc",     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Insert", "",       "",        "",     "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "Delete", "",       "",        "",     "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "PgUp",   "",       "",        "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "RShift",  "Up",        "PgDN",   "",       "",        "",     "",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "RAlt",     "Fn",     "RCtrl", "Left",    "Down",      "Right",  "",       "",        "",     "",       "",      ""},
//    },
};
static const struct tkb_Half_matrix_t Q60 = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21  EC
      {xxSK,      xxSK,      xxSK,      xxSK,      xxSK,     xxSK,       xxSK,         xxSK,     xxSK,     xxSK,        xxSK,    xxSK,    xxSK,      xxSK,        xxSK,     xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_ESC,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_PGUP,  xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_PGDN,  xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_HOME,  xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_END,   xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
    },
};
static const struct tkb_Half_matrix_t M60 = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21  EC
      {xxSK,      xxSK,      xxSK,      xxSK,      xxSK,     xxSK,       xxSK,         xxSK,     xxSK,     xxSK,        xxSK,    xxSK,    xxSK,      xxSK,        xxSK,     xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_ESC,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     xxSK,     xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     xxSK,     xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      xxSK,     xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, xxSK,      KC_RSFT,     xxSK,     xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_APP,    KC_RCTL,     xxSK,     xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
    },
//    .Names = {
//        {"",        "",        "",        "",        "",       "",         "",           "",       "",       "",         "",       "",      "",        "",          "",       "",       "",        "",     "",       "",      ""},
//        {"Esc",     "1",       "2",       "3",       "4",      "5",        "6",          "7",      "8",      "9",        "0",      "-",     "=",       "Back",      "Insert", "",       "",        "",     "",       "",      ""},
//        {"Tab",     "Q",       "W",       "E",       "R",      "T",        "Y",          "U",      "I",      "O",        "P",      "{",     "}",       "|",         "Delete", "",       "",        "",     "",       "",      ""},
//        {"CAP",     "A",       "S",       "D",       "F",      "G",        "H",          "J",      "K",      "L",        ";",      "'",     "" ,       "Enter",     "PgUp",   "",       "",        "",     "",       "",      ""},
//        {"LShift",  "",        "Z",       "X",       "C",      "V",        "B",          "N",      "M",      ",",        ".",      "/",     "RShift",  "Up",        "PgDN",   "",       "",        "",     "",       "",      ""},
//        {"LCtrl",   "Win",     "LAlt",    "",        "",       "",         "Space",      "",       "",       "RAlt",     "Fn",     "RCtrl", "Left",    "Down",      "Right",  "",       "",        "",     "",       "",      ""},
//    },
};
static const struct tkb_Half_matrix_t KC65 = {
    .Matrix = {
        {xxSK,      xxSK,      xxSK,      xxSK,      xxSK,     xxSK,       xxSK,         xxSK,     xxSK,     xxSK,        xxSK,    xxSK,    xxSK,      xxSK,        xxSK,     xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_ESC,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_DEL,   xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGUP,  xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_PGDN,  xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  xxSK,    KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK, xxSK, xxSK, xxSK, xxSK, xxSK},
    },
};
static const struct tkb_Half_matrix_t Y16 = {
    .Matrix = { // Win
    // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL111   COL12    COL13      COL14        COL15     COL16     COL17      COL18    COL19    COL20    COL21  EC
      {xxSK,      xxSK,      xxSK,      xxSK,      xxSK,     xxSK,       xxSK,         xxSK,     xxSK,     xxSK,        xxSK,    xxSK,    xxSK,      xxSK,        xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK}, // KC_EC
      {KC_ESC,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       KC_END,   ____,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    },
};

static const struct tkb_Half_matrix_t WP75 = {
     .Matrix = { // Win
        // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL11    COL12    COL13      COL14        COL15     COL16     COL17      COL18    COL19    COL20    COL21  EC
      {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_DEL,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK}, // KC_EC
      {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_PGUP,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGDN,  xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       xxSK,     xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
      {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     KC_RALT,     FK_FN1,  KC_RCTL, KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,    xxSK,    xxSK,    xxSK},
    }
};

static const struct tkb_Half_matrix_t XP75 = {
    .Matrix = {  // Win
        // COL1   	  COL2       COL3       COL4       COL5      COL6        COL7          COL8      COL9      COL10        COL11    COL12    COL13      COL14        COL15     COL16     COL17      COL18     COL19     COL20     COL21  EC
        {KC_ESC,    xxSK,      KC_F1,     KC_F2,     KC_F3,    KC_F4,      KC_F5,        KC_F6,    KC_F7,    KC_F8,       KC_F9,   KC_F10,  KC_F11,    KC_F12,      KC_DEL,   xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
        {KC_GRV,    KC_1,      KC_2,      KC_3,      KC_4,     KC_5,       KC_6,         KC_7,     KC_8,     KC_9,        KC_0,    KC_MINS, KC_EQL,    KC_BSPC,     KC_INS,   xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
        {KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,     KC_T,       KC_Y,         KC_U,     KC_I,     KC_O,        KC_P,    KC_LBRC, KC_RBRC,   KC_BSLS,     KC_PGUP,  xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
        {KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,     KC_G,       KC_H,         KC_J,     KC_K,     KC_L,        KC_SCLN, KC_QUOT, xxSK,      KC_ENT,      KC_PGDN,  xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
        {KC_LSFT,   xxSK,      KC_Z,      KC_X,      KC_C,     KC_V,       KC_B,         KC_N,     KC_M,     KC_COMM,     KC_DOT,  KC_SLSH, KC_RSFT,   KC_UP,       xxSK,     xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
        {KC_LCTL,   KC_LGUI,   KC_LALT,   xxSK,      xxSK,     xxSK,       KC_SPC,       xxSK,     xxSK,     xxSK,        KC_RALT, FK_FN1,  KC_LEFT,   KC_DOWN,     KC_RIGHT, xxSK,     xxSK,      xxSK,     xxSK,     xxSK,     xxSK},
    }
};

const struct OHID_Key_t  OHID_Keys[] = {
    // keyboard
    {____                    , "____"},
    {KC_A                    , "A"},
    {KC_B                    , "B"},
    {KC_C                    , "C"},
    {KC_D                    , "D"},
    {KC_E                    , "E"},
    {KC_F                    , "F"},
    {KC_G                    , "G"},
    {KC_H                    , "H"},
    {KC_I                    , "I"},
    {KC_J                    , "J"},
    {KC_K                    , "K"},
    {KC_L                    , "L"},
    {KC_M                    , "M"},
    {KC_N                    , "N"},
    {KC_O                    , "O"},
    {KC_P                    , "P"},
    {KC_Q                    , "Q"},
    {KC_R                    , "R"},
    {KC_S                    , "S"},
    {KC_T                    , "T"},
    {KC_U                    , "U"},
    {KC_V                    , "V"},
    {KC_W                    , "W"},
    {KC_X                    , "X"},
    {KC_Y                    , "Y"},
    {KC_Z                    , "Z"},
    {KC_1                    , "1"},
    {KC_2                    , "2"},
    {KC_3                    , "3"},
    {KC_4                    , "4"},
    {KC_5                    , "5"},
    {KC_6                    , "6"},
    {KC_7                    , "7"},
    {KC_8                    , "8"},
    {KC_9                    , "9"},
    {KC_0                    , "0"},
    {KC_ENTER                , "ENTER"},
    {KC_ESCAPE               , "ESC"},
    {KC_BACKSPACE            , "BACK"},
    {KC_TAB                  , "TAB"},
    {KC_SPACE                , "SPACE"},
    {KC_MINUS                , "-_"},
    {KC_EQUAL                , "=+"},
    {KC_LEFT_BRACKET         , "[{"},
    {KC_RIGHT_BRACKET        , "}]"},
    {KC_BACKSLASH            , "\\|"},
    {KC_NONUS_HASH           , ""},
    {KC_SEMICOLON            , ";:"},
    {KC_QUOTE                , "'\""},
    {KC_GRAVE                , "`~"},
    {KC_COMMA                , ",<"},
    {KC_DOT                  , ".>"},
    {KC_SLASH                , "/?"},
    {KC_CAPS_LOCK            , "CAP"},
    {KC_F1                   , "F1 "},
    {KC_F2                   , "F2 "},
    {KC_F3                   , "F3 "},
    {KC_F4                   , "F4 "},
    {KC_F5                   , "F5 "},
    {KC_F6                   , "F6 "},
    {KC_F7                   , "F7 "},
    {KC_F8                   , "F8 "},
    {KC_F9                   , "F9 "},
    {KC_F10                  , "F10"},
    {KC_F11                  , "F11"},
    {KC_F12                  , "F12"},
    {KC_PRINT_SCREEN         , "PRINT"},
    {KC_SCROLL_LOCK          , "SCROLL"},
    {KC_PAUSE                , "PAUSE"},
    {KC_INSERT               , "Ins"},
    {KC_HOME                 , "Home"},
    {KC_PAGE_UP              , "PgUp"},
    {KC_DELETE               , "Del"},
    {KC_END                  , "End"},
    {KC_PAGE_DOWN            , "PgDn"},
    {KC_RIGHT                , "Right"},
    {KC_LEFT                 , "Left"},
    {KC_DOWN                 , "Down"},
    {KC_UP                   , "UP"},
    {KC_NUM_LOCK             , "Num"},
    {KC_KP_SLASH             , "Num\n/"},
    {KC_KP_ASTERISK          , "Num\n*"},
    {KC_KP_MINUS             , "Num\n-"},
    {KC_KP_PLUS              , "Num\n+"},
    {KC_KP_ENTER             , "Return"},
    {KC_KP_1                 , "Num\n1"},
    {KC_KP_2                 , "Num\n2"},
    {KC_KP_3                 , "Num\n3"},
    {KC_KP_4                 , "Num\n4"},
    {KC_KP_5                 , "Num\n5"},
    {KC_KP_6                 , "Num\n6"},
    {KC_KP_7                 , "Num\n7"},
    {KC_KP_8                 , "Num\n8"},
    {KC_KP_9                 , "Num\n9"},
    {KC_KP_0                 , "Num\n0"},
    {KC_KP_DOT               , "Num\n."},
    {KC_NONUS_BACKSLASH      , "BACKSL"},
    {KC_APPLICATION          , "APP"},
    {KC_KB_POWER             , "POWER"},
    {KC_KP_EQUAL             , "EQUAL"},
    {KC_F13                  , "F13"},
    {KC_F14                  , "F14"},
    {KC_F15                  , "F15"},
    {KC_F16                  , "F16"},
    {KC_F17                  , "F17"},
    {KC_F18                  , "F18"},
    {KC_F19                  , "F19"},
    {KC_F20                  , "F20"},
    {KC_F21                  , "F21"},
    {KC_F22                  , "F22"},
    {KC_F23                  , "F23"},
    {KC_F24                  , "F24"},
    {KC_EXECUTE              , "EXECUTE"},
    {KC_HELP                 , "HELP"},
    {KC_MENU                 , "MENU"},
    {KC_SELECT               , "SELECT"},
    {KC_CSTOP                , "STOP"},
    {KC_AGAIN                , "AGAIN"},
    {KC_UNDO                 , "UNDO"},
    {KC_CUT                  , "CUT"},
    {KC_COPY                 , "COPY"},
    {KC_PASTE                , "PASTE"},
    {KC_FIND                 , "FIND"},
    {KC_KB_MUTE              , "MUTE"},
    {KC_KB_VOLUME_UP         , "Vol+"},
    {KC_KB_VOLUME_DOWN       , "Vol-"},
    {KC_LOCKING_CAPS_LOCK    , "CAPS"},
    {KC_LOCKING_NUM_LOCK     , "NUM"},
    {KC_LOCKING_SCROLL_LOCK  , "SCROLL"},
    {KC_KP_COMMA             , "COMMA"},
    {KC_KP_EQUAL_AS400       , "EQUAL"},
    {KC_INTERNATIONAL_1      , "L1"},
    {KC_INTERNATIONAL_2      , "L2"},
    {KC_INTERNATIONAL_3      , "L3"},
    {KC_INTERNATIONAL_4      , "L4"},
    {KC_INTERNATIONAL_5      , "L5"},
    {KC_INTERNATIONAL_6      , "L6"},
    {KC_INTERNATIONAL_7      , "L7"},
    {KC_INTERNATIONAL_8      , "L8"},
    {KC_INTERNATIONAL_9      , "L9"},
    {KC_LANGUAGE_1           , "E1"},
    {KC_LANGUAGE_2           , "E2"},
    {KC_LANGUAGE_3           , "E3"},
    {KC_LANGUAGE_4           , "E4"},
    {KC_LANGUAGE_5           , "E5"},
    {KC_LANGUAGE_6           , "E6"},
    {KC_LANGUAGE_7           , "E7"},
    {KC_LANGUAGE_8           , "E8"},
    {KC_LANGUAGE_9           , "E9"},
    {KC_ALTERNATE_ERASE      , "ERASE"},
    {KC_SYSTEM_REQUEST       , "REQUEST"},
    {KC_CANCEL               , "CANCEL"},
    {KC_CLEAR                , "CLEAR"},
    {KC_PRIOR                , "PRIOR"},
    {KC_RETURN               , "RETURN"},
    {KC_SEPARATOR            , "SEPARAT"},
    {KC_OUT                  , "OUT"},
    {KC_OPER                 , "OPER"},
    {KC_CLEAR_AGAIN          , "CLEAR"},
    {KC_CRSEL                , "CRSEL"},
    {KC_EXSEL                , "EXSEL"},
    {KC_LCTL                 , "LCtrl"},
    {KC_LSFT                 , "LShift"},
    {KC_LALT                 , "LAlt"},
    {KC_LGUI                 , "LWin"},
    {KC_RCTL                 , "Rtrl"},
    {KC_RSFT                 , "RShift"},
    {KC_RALT                 , "RAlt"},
    {KC_RGUI                 , "RWin"},
    // ---
    {KC(FK,FN0)              , "Fn0 "},
    {FK_FN1                  , "Fn  "},
    {FK_FN2                  , "Fn2 "},
    {FK_FN3                  , "Fn3 "},
    {FK_FN4                  , "Fn4 "},
    {FK_FN5                  , "Fn5 "},
    {FK_FN6                  , "Fn6 "},
    {FK_FN7                  , "Fn7 "},
    {FK_FN8                  , "Fn8 "},
    {FK_FN9                  , "Fn9 "},
    {FK_FN10                 , "Fn10"},
    {FK_FN11                 , "Fn11"},
    {FK_FN12                 , "Fn12"},
    {FK_FN13                 , "Fn13"},
    {FK_FN14                 , "Fn14"},
    {FK_FN15                 , "Fn15"},
    {FK_FN16                 , "Fn16"},
    {FK_LWIN                 , "LockWin"},
    {KC_POWER                , "POWER"},
    {KC_SLEEP                , "SLEEP"},
    {KC_NTRACK               , ">>"},
    {KC_PTRACK               , "<<"},
    {KC_PLAY                 , "PLAY"},
    {KC_MUTE                 , "MUTE"},
    {KC_VINC                 , "Vol+"},
    {KC_VDEC                 , "Vol-"},
    {KC_MEDIA                , "MEDIA"},
    {KC_EMAIL                , "EMAIL"},
    {KC_CALC                 , "CALC"},
    {KC_PC                   , "PC"},
    {KC_SEARCH               , "SEARCH"},
    {KC_WWW                  , "WWW"},
    {KC_AC_BACK              , "BACK"},
    {KC_FORWARD              , "FORWARD"},
    {KC_REFRESH              , "REFRESH"},
    {KC_MARKS                , "MARKS"},
    {xxSK                    , "xxSK"},
    {FK_COLOR                , "COLOR"},
    {FK_LIGHTH               , "LIGHTH"},
    {FK_SPEEDL               , "SPEEDL"},
    {FK_LIGHTL               , "LIGHTL"},
    {FK_SPEEDH               , "SPEEDH"},
};
const uint16_t OHID_keys_Size = sizeof(OHID_Keys)/sizeof(OHID_Keys[0]);

const char *OHID_key_name(const uint16_t key_value)
{
    static const char empty[] = "Empty";
    uint16_t kk;
    for(kk=0; kk<OHID_keys_Size; kk++)
    {
        if(key_value==OHID_Keys[kk].key) return OHID_Keys[kk].name;
    }
    return empty;
}

uint8_t OHID_layout_pos(const struct tkb_Half_matrix_t* const Layout, const uint16_t key_value)
{
    uint8_t row, col;
    uint8_t pos = 0xFF;
    // 根据键值获取按键位置, 0xFF为无效, 表示空键
    if(xxSK==key_value) return 0xFF;
    for(row=0; row<6; row++)
    {
        for(col=0; col<21; col++)
        {
            if(key_value==Layout->Matrix[row][col])
            {
                // 分别用3bit 和 5bit存储键的行和列
                //pos = ((row<<5)&0xE0) | (col&0x1F);
                pos = OHID_POS(row,col);
                return pos;
            }
        }
    }
    return 0xFF;//pos;
}
static const struct tkb_Half_matrix_t* OHID_layout_Get(const uint16_t board_type);
void OHID_layout(struct tkb_Half_matrix_t* const Layout, const uint16_t board_type)
{
    uint8_t row, col;
    const struct tkb_Half_matrix_t* const layout = OHID_layout_Get(board_type);
    for(row=0; row<6; row++)
    {
        for(col=0; col<21; col++)
        {
            Layout->Matrix[row][col] = layout->Matrix[row][col];
        }
    }
}

static const struct tkb_Half_matrix_t* OHID_layout_Get(const uint16_t board_type)
{
    //qDebug("[%s--%d] Type:0x%04X 0x%04X\r\n", __func__, __LINE__, board_type, OHID_KB_MAGNETIC_K82);
    switch (board_type)
    {
        case OHID_KB_MAGNETIC_K75:
            return &WP75;
        case OHID_KB_MACHINE_K82:
        case OHID_KB_MAGNETIC_K82:
        case OHID_KB_OPTICAL_K82:
            return &K82;
        case OHID_KB_MACHINE_K98:
        case OHID_KB_MAGNETIC_K98:
        case OHID_KB_OPTICAL_K98:
        return &V98H;
        case OHID_KB_MAGNETIC_K80: return &TK51GF;
        case OHID_BOARD_SUB_KEYBOARD_MAGNETIC_K104:
        case OHID_BOARD_SUB_KEYBOARD_OPTICAL_K104:
        case OHID_BOARD_SUB_KEYBOARD_MACHINE_K104:
        default:
            return &K104;

    }
    return &K104;
}

uint16_t OHID_board_valid_keys_count(const uint32_t board_id)
{
    const struct tkb_Half_matrix_t* const Layout = OHID_board_layout_Get(board_id);
    uint16_t count = 0;
    uint8_t row, col;
    
    if(NULL == Layout) return 0;
    
    for(row = 0; row < 6; row++)
    {
        for(col = 0; col < 21; col++)
        {
            // 统计非 xxSK 和非 ____ 的按键
            if((Layout->Matrix[row][col] != xxSK) && (Layout->Matrix[row][col] != ____))
            {
                count++;
            }
        }
    }
    
    return count - 1; // DO NOT need to Count FN
}


static const struct tkb_Half_matrix_t* __OHID_board_layout_Get(const uint32_t board_id)
{
    //printf("[%s--%d] board_id:0x%04X 0x%04X MAG98_51WH:0x%08X V98H:0x%08X\r\n", __func__, __LINE__, board_id, OHID_BOARD_51WH, &MAG98_51WH, &V98H);
    //printf("[%s--%d] board_id:0x%04X 0x%04X MAG98_51WH:0x%08X V98H:0x%08X\r\n", __func__, __LINE__, board_id, OHID_BOARD_J60, &M60, &Q60);
    switch (board_id)
    {
        // STEP2: 在这里添加boardid对应的配列
        case OHID_BOARD_FS370_XP75:
            return &XP75;
        case 0x1750010:
        case OHID_BOARD_FS370_WP75:
        // case OHID_BOARD_FS370_XP75:
            return &WP75;

        case OHID_BOARD_M98:
        case OHID_BOARD_FS370_WP98S:
        case OHID_BOARD_BIN_WP98_V2_5C:
        case OHID_BOARD_BIN_MP98_V2_5C:
            return &WP98;

        case OHID_BOARD_51WH:
            //printf("[%s--%d] board_id:0x%04X 0x%04X\r\n", __func__, __LINE__, board_id, OHID_BOARD_51WH);
            //fflush(stdout);
            return &MAG98_51WH;
        case OHID_BOARD_EK75PLUS:
            //printf("[%s--%d] board_id:0x%04X 0x%04X\r\n", __func__, __LINE__, board_id, OHID_BOARD_51WH);
            //fflush(stdout);
            return &TK51GF;
        case OHID_BOARD_HS_GK6950:
        case OHID_BOARD_KC65: return &KC65;
        case OHID_BOARD_BK75H370: return &BK75_370;
        case OHID_BOARD_TK52L370: return &TK52L_370;
        case OHID_BOARD_TK50T: return &TK50T;
        case OHID_BOARD_J60: return &M60;
        case OHID_BOARD_JNS60:
        case OHID_BOARD_MJH60:
        case OHID_BOARD_M60B:
        case OHID_BOARD_M60BHS:
        case OHID_BOARD_DEEP60HS370:
        case OHID_BOARD_M60: return &M60;
        case OHID_BOARD_TK51DHS370: return &TK51GF;
        case OHID_BOARD_TKG332:
        case OHID_BOARD_AE68:

        case OHID_BOARD_Q60: return &Q60;
        case OHID_BOARD_51Q: return &TK51Q;
        case OHID_BOARD_ZONEX75: return &ZONEX75;
        case OHID_BOARD_ZONEX75HS: return &ZONEX75;
        case OHID_BOARD_CK8971X75: return &CK8971;
        case OHID_BOARD_TK51QHS370: return &TK51QHS370;
        case OHID_BOARD_Y16:
            return &Y16;
        default:
            fflush(stdout);
            return NULL;

    }
    //fflush(stdout);
    return NULL;
}

const struct tkb_Half_matrix_t* OHID_board_layout_Get(const uint32_t board_id)
{
    //const struct tkb_Half_matrix_t* const Layout = OHID_layout_Get(OHID_GET_SUB(Board.board_id));
    const struct tkb_Half_matrix_t* volatile Layout = __OHID_board_layout_Get(board_id);
    // printf("boarddddid: 0x%08X", (uint16_t)board_id);
    // printf("SUB: 0x%04X", (uint16_t)OHID_GET_SUB(board_id));

    if(NULL==Layout) Layout = OHID_layout_Get(OHID_GET_SUB(board_id));
    //fflush(stdout);
    if(NULL==Layout) return &K104;
    return Layout;
    //return OHID_layout_Get(board_id);
}

/************************** (C) COPYRIGHT Merafour **************************/
