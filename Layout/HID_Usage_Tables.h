/************************ (C) COPYLEFT 2018 Merafour *************************
* File Name          : HID_Usage_Tables.h
* Author             : Merafour
* Last Modified Date : 2024.03.30
* Description        : HID 用途表,<HID Usage Tables>,
* Description        : 参考 QMK
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _HID_USAGE_TABLES_H_
#define _HID_USAGE_TABLES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*************************** 编码重定义 ******************************************
* 将 <HID Usage Tables> 中的编码统一为 16bit
******************************************************************************/
#define UTG_ID(code)                        ((0xF<<12)&code)      // Get ID
#define UTG_KB(code)                        (0xFF&code)           // Get Keyboard/Keypad
#define UTG_CONSUMER(code)                  (0x3FF&code)          // Get Consumer
#define UTG_GAME(code)                      (0xFFF&code)          // Get Game Controls
#define UTG_SPORT(code)                     (0xFFF&code)          // Get Sport Controls
#define UTG_VR(code)                        (0xFFF&code)          // Get VR Controls
#define UTG_GRNERIC(code)                   (0xFFF&code)          // Get Generic Desktop
#define UTG_FK(code)                        (0xFFF&code)          // Get function keys
#define UTG_MK(code)                        (0xFFF&code)          // Get mix keys
#define UTG_MOUSE(code)                     (0xFFF&code)          // Get Mouse

/*************************** 编码定义 ************************************/
#define HID_UT_CODE(UT_ID,CODE)             ( (UT_ID&0xF000)|(CODE&0x0FFF) )
// 扩展编码, 32bit :
#define UT_CODE_WORD(row,col,TrPs,CODE)     ( ((row<<21)&0x00E00000)|((col<<16)&0x001F0000)|((TrPs<<24)&0x0F000000)|(CODE&0x0000FFFF) )
#define UTG_CODE_WORD_ROW(CODE)             ( (CODE>>21)&0x07 )
#define UTG_CODE_WORD_COL(CODE)             ( (CODE>>16)&0x1F )
#define UTG_CODE_WORD_TRPS(CODE)            ( (CODE>>24)&0x0F )
#define FK_GET_SEG(CODE)                    ( (CODE>> 8)&0x0F )
/**************************** 功能键 *********************************/
#define FK_CODE(HID_FK_ID,CODE) ( (HID_UT_ID_FK)|(HID_FK_ID&0x0F00)|(CODE&0x00FF) )
/*************************** Mouse  ********************************/
#define MOUSE_CODE(HID_MOUSE_ID,CODE)       ( (UT(MOUSE))|(HID_MOUSE_ID&0x0F00)|(CODE&0x00FF) )
#define MOUSEG_ID(code)                     ((code>>8)&0xF)      // Get ID
#define MOUSEG_VALUE(code)                  ((code)&0xFF)        // Get Value
/*************************** Group Helpers ********************************/
#define IS_INTERNAL_KEYCODE(code)           ((code) >= KC_NO && (code) <= KC_TRANSPARENT)
#define IS_KEYBOARD_KEYCODE(code)           (UT(KB)      ==(code&UT(MASK)))
#define IS_SYSTEM_KEYCODE(code)             (UT(DESK)    ==(code&UT(MASK)))
#define IS_CONSUMER_KEYCODE(code)           (UT(CONSUMER)==(code&UT(MASK)))

#define  HID_UT_PREFIX(prefix,segment)       prefix##segment
#define  UT(segment)                         HID_UT_PREFIX(HID_UT_ID_,segment)
#define  FK_ID(segment)                      HID_UT_PREFIX(HID_FK_ID_,segment)
#define  _KC(prefix,segment)                 KC_##prefix##_##segment
#define  KC(prefix,segment)                  _KC(prefix,segment)
enum HID_UT_t {
    UT(MASK)          = (0x000FU<<12),             // ID mask
    UT(KB)            = (0x0000U<<12),             // 10 Keyboard/Keypad Page (0x07)
    UT(DESK)          = (0x0001U<<12),             // 4 Generic Desktop Page (0x01)
    UT(VR)            = (0x0003U<<12),             // 6 VR Controls Page (0x03)
    UT(SPORT)         = (0x0004U<<12),             // 7 Sport Controls Page (0x04)
    UT(GAME)          = (0x0005U<<12),             // 8 Game Controls Page (0x05)
    UT(CONSUMER)      = (0x000CU<<12),             // 15 Consumer Page (0x0C)
    UT(MOUSE)         = (0x0006U<<12),             // Mouse
    UT(FK)            = (0x000FU<<12),             // function keys, my define
/*************************** Alias ********************************/
    UT_MASK           = UT(MASK)     ,         // ID mask
    UT_KB             = UT(KB)       ,         // 10 Keyboard/Keypad Page (0x07)
    UT_DESK           = UT(DESK)     ,         // 4 Generic Desktop Page (0x01)
    UT_VR             = UT(VR)       ,         // 6 VR Controls Page (0x03)
    UT_SPORT          = UT(SPORT)    ,         // 7 Sport Controls Page (0x04)
    UT_GAME           = UT(GAME)     ,         // 8 Game Controls Page (0x05)
    UT_CONSUMER       = UT(CONSUMER) ,         // 15 Consumer Page (0x0C)
    UT_MOUSE          = UT(MOUSE)    ,         // Mouse
    UT_FK             = UT(FK)       ,         // function keys, my define
/*************************** 功能键(function keys) ******************************************/
    FK_ID(FN)         = (0x0000U<<8),    // switch layout
    FK_ID(SYS)        = (0x0001U<<8),    // system Ctrl
    FK_ID(RGB)        = (0x0002U<<8),    // RGB Ctrl
    FK_ID(MK)         = (0x0003U<<8),    // mix keys, my define
    FK_ID(EC)         = (0x0004U<<8),    // 编码器
/*************************** 定义<HID Usage Tables>中的编码 ********************************/
    KC_NO             =  0x0000U,   // 无效编码
    KC_TRANSPARENT    =  0x0001U,   // 层穿透
    KC_ENCODER        =  0x00FFU,   // 编码器
    KC_INVALID        =  0xFFFFU,   // 无效按键
/*************************** Alias ********************************/
    XXXXXXX           = KC_NO,
    XXXX              = KC_NO,
    _______           = KC_TRANSPARENT,
    ____              = KC_TRANSPARENT,
    xxSK	          = KC_NO,
    KC_EC	          = KC_ENCODER,
/*************************** <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) ********************************/
//                                                             // ID(Dec)      ID(Hex)      Name
    KC_00                    = HID_UT_CODE(UT(KB),0x0000),     // 0            00           Reserved (no event indicated)9 N/A √ √ √ 4/101/104
    KC_01                    = HID_UT_CODE(UT(KB),0x0001),     // 1            01           Keyboard ErrorRollOver9 N/A √ √ √ 4/101/104
    KC_02                    = HID_UT_CODE(UT(KB),0x0002),     // 2            02           Keyboard POSTFail9 N/A √ √ √ 4/101/104
    KC_03                    = HID_UT_CODE(UT(KB),0x0003),     // 3            03           Keyboard ErrorUndefined9 N/A √ √ √ 4/101/104
    KC_A                     = HID_UT_CODE(UT(KB),0x0004),     // 4            04           Keyboard a and A4 31 √ √ √ 4/101/104
    KC_B                     = HID_UT_CODE(UT(KB),0x0005),     // 5            05           Keyboard b and B 50 √ √ √ 4/101/104
    KC_C                     = HID_UT_CODE(UT(KB),0x0006),     // 6            06           Keyboard c and C4 48 √ √ √ 4/101/104
    KC_D                     = HID_UT_CODE(UT(KB),0x0007),     // 7            07           Keyboard d and D 33 √ √ √ 4/101/104
    KC_E                     = HID_UT_CODE(UT(KB),0x0008),     // 8            08           Keyboard e and E 19 √ √ √ 4/101/104
    KC_F                     = HID_UT_CODE(UT(KB),0x0009),     // 9            09           Keyboard f and F 34 √ √ √ 4/101/104
    KC_G                     = HID_UT_CODE(UT(KB),0x000A),     // 10           0A           Keyboard g and G 35 √ √ √ 4/101/104
    KC_H                     = HID_UT_CODE(UT(KB),0x000B),     // 11           0B           Keyboard h and H 36 √ √ √ 4/101/104
    KC_I                     = HID_UT_CODE(UT(KB),0x000C),     // 12           0C           Keyboard i and I 24 √ √ √ 4/101/104
    KC_J                     = HID_UT_CODE(UT(KB),0x000D),     // 13           0D           Keyboard j and J 37 √ √ √ 4/101/104
    KC_K                     = HID_UT_CODE(UT(KB),0x000E),     // 14           0E           Keyboard k and K 38 √ √ √ 4/101/104
    KC_L                     = HID_UT_CODE(UT(KB),0x000F),     // 15           0F           Keyboard l and L 39 √ √ √ 4/101/104
    KC_M                     = HID_UT_CODE(UT(KB),0x0010),     // 16           10           Keyboard m and M4 52 √ √ √ 4/101/104
    KC_N                     = HID_UT_CODE(UT(KB),0x0011),     // 17           11           Keyboard n and N 51 √ √ √ 4/101/104
    KC_O                     = HID_UT_CODE(UT(KB),0x0012),     // 18           12           Keyboard o and O4 25 √ √ √ 4/101/104
    KC_P                     = HID_UT_CODE(UT(KB),0x0013),     // 19           13           Keyboard p and P4 26 √ √ √ 4/101/104
    KC_Q                     = HID_UT_CODE(UT(KB),0x0014),     // 20           14           Keyboard q and Q4 17 √ √ √ 4/101/104
    KC_R                     = HID_UT_CODE(UT(KB),0x0015),     // 21           15           Keyboard r and R 20 √ √ √ 4/101/104
    KC_S                     = HID_UT_CODE(UT(KB),0x0016),     // 22           16           Keyboard s and S4 32 √ √ √ 4/101/104
    KC_T                     = HID_UT_CODE(UT(KB),0x0017),     // 23           17           Keyboard t and T 21 √ √ √ 4/101/104
    KC_U                     = HID_UT_CODE(UT(KB),0x0018),     // 24           18           Keyboard u and U 23 √ √ √ 4/101/104
    KC_V                     = HID_UT_CODE(UT(KB),0x0019),     // 25           19           Keyboard v and V 49 √ √ √ 4/101/104
    KC_W                     = HID_UT_CODE(UT(KB),0x001A),     // 26           1A           Keyboard w and W4 18 √ √ √ 4/101/104
    KC_X                     = HID_UT_CODE(UT(KB),0x001B),     // 27           1B           Keyboard x and X4 47 √ √ √ 4/101/104
    KC_Y                     = HID_UT_CODE(UT(KB),0x001C),     // 28           1C           Keyboard y and Y4 22 √ √ √ 4/101/104
    KC_Z                     = HID_UT_CODE(UT(KB),0x001D),     // 29           1D           Keyboard z and Z4 46 √ √ √ 4/101/104
    KC_1                     = HID_UT_CODE(UT(KB),0x001E),     // 30           1E           Keyboard 1 and !4 2 √ √ √ 4/101/104
    KC_2                     = HID_UT_CODE(UT(KB),0x001F),     // 31           1F           Keyboard 2 and @4 3 √ √ √ 4/101/104
    KC_3                     = HID_UT_CODE(UT(KB),0x0020),     // 32           20           Keyboard 3 and #4 4 √ √ √ 4/101/104
    KC_4                     = HID_UT_CODE(UT(KB),0x0021),     // 33           21           Keyboard 4 and $4 5 √ √ √ 4/101/104
    KC_5                     = HID_UT_CODE(UT(KB),0x0022),     // 34           22           Keyboard 5 and %4 6 √ √ √ 4/101/104
    KC_6                     = HID_UT_CODE(UT(KB),0x0023),     // 35           23           Keyboard 6 and ^4 7 √ √ √ 4/101/104
    KC_7                     = HID_UT_CODE(UT(KB),0x0024),     // 36           24           Keyboard 7 and &4 8 √ √ √ 4/101/104
    KC_8                     = HID_UT_CODE(UT(KB),0x0025),     // 37           25           Keyboard 8 and *4 9 √ √ √ 4/101/104
    KC_9                     = HID_UT_CODE(UT(KB),0x0026),     // 38           26           Keyboard 9 and (4 10 √ √ √ 4/101/104
    KC_0                     = HID_UT_CODE(UT(KB),0x0027),     // 39           27           Keyboard 0 and )4 11 √ √ √ 4/101/104
    KC_ENTER                 = HID_UT_CODE(UT(KB),0x0028),     // 40           28           Keyboard Return (ENTER)5 43 √ √ √ 4/101/104
    KC_ESCAPE                = HID_UT_CODE(UT(KB),0x0029),     // 41           29           Keyboard ESCAPE 110 √ √ √ 4/101/104
    KC_BACKSPACE             = HID_UT_CODE(UT(KB),0x002A),     // 42           2A           Keyboard DELETE (Backspace)13 15 √ √ √ 4/101/104
    KC_TAB                   = HID_UT_CODE(UT(KB),0x002B),     // 43           2B           Keyboard Tab 16 √ √ √ 4/101/104
    KC_SPACE                 = HID_UT_CODE(UT(KB),0x002C),     // 44           2C           Keyboard Spacebar 61 √ √ √ 4/101/104
    KC_MINUS                 = HID_UT_CODE(UT(KB),0x002D),     // 45           2D           Keyboard - and (underscore)4 12 √ √ √ 4/101/104
    KC_EQUAL                 = HID_UT_CODE(UT(KB),0x002E),     // 46           2E           Keyboard = and +4 13 √ √ √ 4/101/104
    KC_LEFT_BRACKET          = HID_UT_CODE(UT(KB),0x002F),     // 47           2F           Keyboard [ and {4 27 √ √ √ 4/101/104
    KC_RIGHT_BRACKET         = HID_UT_CODE(UT(KB),0x0030),     // 48           30           Keyboard ] and }4 28 √ √ √ 4/101/104
    KC_BACKSLASH             = HID_UT_CODE(UT(KB),0x0031),     // 49           31           Keyboard \ and | 29 √ √ √ 4/101/104
    KC_NONUS_HASH            = HID_UT_CODE(UT(KB),0x0032),     // 50           32           Keyboard Non-US # and ~2 42 √ √ √ 4/101/104
    KC_SEMICOLON             = HID_UT_CODE(UT(KB),0x0033),     // 51           33           Keyboard ; and :4 40 √ √ √ 4/101/104
    KC_QUOTE                 = HID_UT_CODE(UT(KB),0x0034),     // 52           34           Keyboard ‘ and “4 41 √ √ √ 4/101/104
    KC_GRAVE                 = HID_UT_CODE(UT(KB),0x0035),     // 53           35           Keyboard Grave Accent and Tilde4 1 √ √ √ 4/101/104
    KC_COMMA                 = HID_UT_CODE(UT(KB),0x0036),     // 54           36           Keyboard, and <4 53 √ √ √ 4/101/104
    KC_DOT                   = HID_UT_CODE(UT(KB),0x0037),     // 55           37           Keyboard . and >4 54 √ √ √ 4/101/104
    KC_SLASH                 = HID_UT_CODE(UT(KB),0x0038),     // 56           38           Keyboard / and ?4 55 √ √ √ 4/101/104
    KC_CAPS_LOCK             = HID_UT_CODE(UT(KB),0x0039),     // 57           39           Keyboard Caps Lock11 30 √ √ √ 4/101/104
    KC_F1                    = HID_UT_CODE(UT(KB),0x003A),     // 58           3A           Keyboard F1 112 √ √ √ 4/101/104
    KC_F2                    = HID_UT_CODE(UT(KB),0x003B),     // 59           3B           Keyboard F2 113 √ √ √ 4/101/104
    KC_F3                    = HID_UT_CODE(UT(KB),0x003C),     // 60           3C           Keyboard F3 114 √ √ √ 4/101/104
    KC_F4                    = HID_UT_CODE(UT(KB),0x003D),     // 61           3D           Keyboard F4 115 √ √ √ 4/101/104
    KC_F5                    = HID_UT_CODE(UT(KB),0x003E),     // 62           3E           Keyboard F5 116 √ √ √ 4/101/104
    KC_F6                    = HID_UT_CODE(UT(KB),0x003F),     // 63           3F           Keyboard F6 117 √ √ √ 4/101/104
    KC_F7                    = HID_UT_CODE(UT(KB),0x0040),     // 64           40           Keyboard F7 118 √ √ √ 4/101/104
    KC_F8                    = HID_UT_CODE(UT(KB),0x0041),     // 65           41           Keyboard F8 119 √ √ √ 4/101/104
    KC_F9                    = HID_UT_CODE(UT(KB),0x0042),     // 66           42           Keyboard F9 120 √ √ √ 4/101/104
    KC_F10                   = HID_UT_CODE(UT(KB),0x0043),     // 67           43           Keyboard F10 121 √ √ √ 4/101/104
    KC_F11                   = HID_UT_CODE(UT(KB),0x0044),     // 68           44           Keyboard F11 122 √ √ √ 101/104
    KC_F12                   = HID_UT_CODE(UT(KB),0x0045),     // 69           45           Keyboard F12 123 √ √ √ 101/104
    KC_PRINT_SCREEN          = HID_UT_CODE(UT(KB),0x0046),     // 70           46           Keyboard PrintScreen1 124 √ √ √ 101/104
    KC_SCROLL_LOCK           = HID_UT_CODE(UT(KB),0x0047),     // 71           47           Keyboard Scroll Lock11 125 √ √ √ 4/101/104
    KC_PAUSE                 = HID_UT_CODE(UT(KB),0x0048),     // 72           48           Keyboard Pause1 126 √ √ √ 101/104
    KC_INSERT                = HID_UT_CODE(UT(KB),0x0049),     // 73           49           Keyboard Insert1 75 √ √ √ 101/104
    KC_HOME                  = HID_UT_CODE(UT(KB),0x004A),     // 74           4A           Keyboard Home1 80 √ √ √ 101/104
    KC_PAGE_UP               = HID_UT_CODE(UT(KB),0x004B),     // 75           4B           Keyboard PageUp1 85 √ √ √ 101/104
    KC_DELETE                = HID_UT_CODE(UT(KB),0x004C),     // 76           4C           Keyboard Delete Forward1;14 76 √ √ √ 101/104
    KC_END                   = HID_UT_CODE(UT(KB),0x004D),     // 77           4D           Keyboard End1 81 √ √ √ 101/104
    KC_PAGE_DOWN             = HID_UT_CODE(UT(KB),0x004E),     // 78           4E           Keyboard PageDown1 86 √ √ √ 101/104
    KC_RIGHT                 = HID_UT_CODE(UT(KB),0x004F),     // 79           4F           Keyboard RightArrow1 89 √ √ √ 101/104
    KC_LEFT                  = HID_UT_CODE(UT(KB),0x0050),     // 80           50           Keyboard LeftArrow1 79 √ √ √ 101/104
    KC_DOWN                  = HID_UT_CODE(UT(KB),0x0051),     // 81           51           Keyboard DownArrow1 84 √ √ √ 101/104
    KC_UP                    = HID_UT_CODE(UT(KB),0x0052),     // 82           52           Keyboard UpArrow1 83 √ √ √ 101/104
    KC_NUM_LOCK              = HID_UT_CODE(UT(KB),0x0053),     // 83           53           Keypad Num Lock and Clear11 90 √ √ √ 101/104
    KC_KP_SLASH              = HID_UT_CODE(UT(KB),0x0054),     // 84           54           Keypad /1 95 √ √ √ 101/104
    KC_KP_ASTERISK           = HID_UT_CODE(UT(KB),0x0055),     // 85           55           Keypad * 100 √ √ √ 4/101/104
    KC_KP_MINUS              = HID_UT_CODE(UT(KB),0x0056),     // 86           56           Keypad - 105 √ √ √ 4/101/104
    KC_KP_PLUS               = HID_UT_CODE(UT(KB),0x0057),     // 87           57           Keypad + 106 √ √ √ 4/101/104
    KC_KP_ENTER              = HID_UT_CODE(UT(KB),0x0058),     // 88           58           Keypad ENTER5 108 √ √ √ 101/104
    KC_KP_1                  = HID_UT_CODE(UT(KB),0x0059),     // 89           59           Keypad 1 and End 93 √ √ √ 4/101/104
    KC_KP_2                  = HID_UT_CODE(UT(KB),0x005A),     // 90           5A           Keypad 2 and Down Arrow 98 √ √ √ 4/101/104
    KC_KP_3                  = HID_UT_CODE(UT(KB),0x005B),     // 91           5B           Keypad 3 and PageDn 103 √ √ √ 4/101/104
    KC_KP_4                  = HID_UT_CODE(UT(KB),0x005C),     // 92           5C           Keypad 4 and Left Arrow 92 √ √ √ 4/101/104
    KC_KP_5                  = HID_UT_CODE(UT(KB),0x005D),     // 93           5D           Keypad 5 97 √ √ √ 4/101/104
    KC_KP_6                  = HID_UT_CODE(UT(KB),0x005E),     // 94           5E           Keypad 6 and Right Arrow 102 √ √ √ 4/101/104
    KC_KP_7                  = HID_UT_CODE(UT(KB),0x005F),     // 95           5F           Keypad 7 and Home 91 √ √ √ 4/101/104
    KC_KP_8                  = HID_UT_CODE(UT(KB),0x0060),     // 96           60           Keypad 8 and Up Arrow 96 √ √ √ 4/101/104
    KC_KP_9                  = HID_UT_CODE(UT(KB),0x0061),     // 97           61           Keypad 9 and PageUp 101 √ √ √ 4/101/104
    KC_KP_0                  = HID_UT_CODE(UT(KB),0x0062),     // 98           62           Keypad 0 and Insert 99 √ √ √ 4/101/104
    KC_KP_DOT                = HID_UT_CODE(UT(KB),0x0063),     // 99           63           Keypad . and Delete 104 √ √ √ 4/101/104
    KC_NONUS_BACKSLASH       = HID_UT_CODE(UT(KB),0x0064),     // 100          64           Keyboard Non-US \ and |3;6 45 √ √ √ 4/101/104
    KC_APPLICATION           = HID_UT_CODE(UT(KB),0x0065),     // 101          65           Keyboard Application10 129 √ √ 104
    KC_KB_POWER              = HID_UT_CODE(UT(KB),0x0066),     // 102          66           Keyboard Power9 √ √
    KC_KP_EQUAL              = HID_UT_CODE(UT(KB),0x0067),     // 103          67           Keypad = √
    KC_F13                   = HID_UT_CODE(UT(KB),0x0068),     // 104          68           Keyboard F13 √
    KC_F14                   = HID_UT_CODE(UT(KB),0x0069),     // 105          69           Keyboard F14 √
    KC_F15                   = HID_UT_CODE(UT(KB),0x006A),     // 106          6A           Keyboard F15 √
    KC_F16                   = HID_UT_CODE(UT(KB),0x006B),     // 107          6B           Keyboard F16
    KC_F17                   = HID_UT_CODE(UT(KB),0x006C),     // 108          6C           Keyboard F17
    KC_F18                   = HID_UT_CODE(UT(KB),0x006D),     // 109          6D           Keyboard F18
    KC_F19                   = HID_UT_CODE(UT(KB),0x006E),     // 110          6E           Keyboard F19
    KC_F20                   = HID_UT_CODE(UT(KB),0x006F),     // 111          6F           Keyboard F20
    KC_F21                   = HID_UT_CODE(UT(KB),0x0070),     // 112          70           Keyboard F21
    KC_F22                   = HID_UT_CODE(UT(KB),0x0071),     // 113          71           Keyboard F22
    KC_F23                   = HID_UT_CODE(UT(KB),0x0072),     // 114          72           Keyboard F23
    KC_F24                   = HID_UT_CODE(UT(KB),0x0073),     // 115          73           Keyboard F24
    KC_EXECUTE               = HID_UT_CODE(UT(KB),0x0074),     // 116          74           Keyboard Execute √
    KC_HELP                  = HID_UT_CODE(UT(KB),0x0075),     // 117          75           Keyboard Help √
    KC_MENU                  = HID_UT_CODE(UT(KB),0x0076),     // 118          76           Keyboard Menu √
    KC_SELECT                = HID_UT_CODE(UT(KB),0x0077),     // 119          77           Keyboard Select √
    KC_KSTOP                 = HID_UT_CODE(UT(KB),0x0078),     // 120          78           Keyboard Stop √
    KC_AGAIN                 = HID_UT_CODE(UT(KB),0x0079),     // 121          79           Keyboard Again √
    KC_UNDO                  = HID_UT_CODE(UT(KB),0x007A),     // 122          7A           Keyboard Undo √
    KC_CUT                   = HID_UT_CODE(UT(KB),0x007B),     // 123          7B           Keyboard Cut √
    KC_COPY                  = HID_UT_CODE(UT(KB),0x007C),     // 124          7C           Keyboard Copy √
    KC_PASTE                 = HID_UT_CODE(UT(KB),0x007D),     // 125          7D           Keyboard Paste √
    KC_FIND                  = HID_UT_CODE(UT(KB),0x007E),     // 126          7E           Keyboard Find √
    KC_KB_MUTE               = HID_UT_CODE(UT(KB),0x007F),     // 127          7F           Keyboard Mute √
    KC_KB_VOLUME_UP          = HID_UT_CODE(UT(KB),0x0080),     // 128          80           Keyboard Volume Up √
    KC_KB_VOLUME_DOWN        = HID_UT_CODE(UT(KB),0x0081),     // 129          81           Keyboard Volume Down √
    KC_LOCKING_CAPS_LOCK     = HID_UT_CODE(UT(KB),0x0082),     // 130          82           Keyboard Locking Caps Lock12 √
    KC_LOCKING_NUM_LOCK      = HID_UT_CODE(UT(KB),0x0083),     // 131          83           Keyboard Locking Num Lock12 √
    KC_LOCKING_SCROLL_LOCK   = HID_UT_CODE(UT(KB),0x0084),     // 132          84           Keyboard Locking Scroll Lock12 √
    KC_KP_COMMA              = HID_UT_CODE(UT(KB),0x0085),     // 133          85           Keypad Comma27 107
    KC_KP_EQUAL_AS400        = HID_UT_CODE(UT(KB),0x0086),     // 134          86           Keypad Equal Sign29
    KC_INTERNATIONAL_1       = HID_UT_CODE(UT(KB),0x0087),     // 135          87           Keyboard International115,28 56
    KC_INTERNATIONAL_2       = HID_UT_CODE(UT(KB),0x0088),     // 136          88           Keyboard International216
    KC_INTERNATIONAL_3       = HID_UT_CODE(UT(KB),0x0089),     // 137          89           Keyboard International317
    KC_INTERNATIONAL_4       = HID_UT_CODE(UT(KB),0x008A),     // 138          8A           Keyboard International418
    KC_INTERNATIONAL_5       = HID_UT_CODE(UT(KB),0x008B),     // 139          8B           Keyboard International519
    KC_INTERNATIONAL_6       = HID_UT_CODE(UT(KB),0x008C),     // 140          8C           Keyboard International620
    KC_INTERNATIONAL_7       = HID_UT_CODE(UT(KB),0x008D),     // 141          8D           Keyboard International721
    KC_INTERNATIONAL_8       = HID_UT_CODE(UT(KB),0x008E),     // 142          8E           Keyboard International822
    KC_INTERNATIONAL_9       = HID_UT_CODE(UT(KB),0x008F),     // 143          8F           Keyboard International922
    KC_LANGUAGE_1            = HID_UT_CODE(UT(KB),0x0090),     // 144          90           Keyboard LANG125
    KC_LANGUAGE_2            = HID_UT_CODE(UT(KB),0x0091),     // 145          91           Keyboard LANG226
    KC_LANGUAGE_3            = HID_UT_CODE(UT(KB),0x0092),     // 146          92           Keyboard LANG330
    KC_LANGUAGE_4            = HID_UT_CODE(UT(KB),0x0093),     // 147          93           Keyboard LANG431
    KC_LANGUAGE_5            = HID_UT_CODE(UT(KB),0x0094),     // 148          94           Keyboard LANG532
    KC_LANGUAGE_6            = HID_UT_CODE(UT(KB),0x0095),     // 149          95           Keyboard LANG68
    KC_LANGUAGE_7            = HID_UT_CODE(UT(KB),0x0096),     // 150          96           Keyboard LANG78
    KC_LANGUAGE_8            = HID_UT_CODE(UT(KB),0x0097),     // 151          97           Keyboard LANG88
    KC_LANGUAGE_9            = HID_UT_CODE(UT(KB),0x0098),     // 152          98           Keyboard LANG98
    KC_ALTERNATE_ERASE       = HID_UT_CODE(UT(KB),0x0099),     // 153          99           Keyboard Alternate Erase7
    KC_SYSTEM_REQUEST        = HID_UT_CODE(UT(KB),0x009A),     // 154          9A           Keyboard SysReq/Attention1
    KC_CANCEL                = HID_UT_CODE(UT(KB),0x009B),     // 155          9B           Keyboard Cancel
    KC_CLEAR                 = HID_UT_CODE(UT(KB),0x009C),     // 156          9C           Keyboard Clear
    KC_PRIOR                 = HID_UT_CODE(UT(KB),0x009D),     // 157          9D           Keyboard Prior
    KC_RETURN                = HID_UT_CODE(UT(KB),0x009E),     // 158          9E           Keyboard Return
    KC_SEPARATOR             = HID_UT_CODE(UT(KB),0x009F),     // 159          9F           Keyboard Separator
    KC_OUT                   = HID_UT_CODE(UT(KB),0x00A0),     // 160          A0           Keyboard Out
    KC_OPER                  = HID_UT_CODE(UT(KB),0x00A1),     // 161          A1           Keyboard Oper
    KC_CLEAR_AGAIN           = HID_UT_CODE(UT(KB),0x00A2),     // 162          A2           Keyboard Clear/Again
    KC_CRSEL                 = HID_UT_CODE(UT(KB),0x00A3),     // 163          A3           Keyboard CrSel/Props
    KC_EXSEL                 = HID_UT_CODE(UT(KB),0x00A4),     // 164          A4           Keyboard ExSel
    KC_A5                    = HID_UT_CODE(UT(KB),0x00A5),     // 165-175      A5-CF        Reserved
    KC_B0                    = HID_UT_CODE(UT(KB),0x00B0),     // 176          B0           Keypad 00
    KC_B1                    = HID_UT_CODE(UT(KB),0x00B1),     // 177          B1           Keypad 000
    KC_B2                    = HID_UT_CODE(UT(KB),0x00B2),     // 178          B2           Thousands Separator 33
    KC_B3                    = HID_UT_CODE(UT(KB),0x00B3),     // 179          B3           Decimal Separator 33
    KC_B4                    = HID_UT_CODE(UT(KB),0x00B4),     // 180          B4           Currency Unit 34
    KC_B5                    = HID_UT_CODE(UT(KB),0x00B5),     // 181          B5           Currency Sub-unit 34
    KC_B6                    = HID_UT_CODE(UT(KB),0x00B6),     // 182          B6           Keypad (
    KC_B7                    = HID_UT_CODE(UT(KB),0x00B7),     // 183          B7           Keypad )
    KC_B8                    = HID_UT_CODE(UT(KB),0x00B8),     // 184          B8           Keypad {
    KC_B9                    = HID_UT_CODE(UT(KB),0x00B9),     // 185          B9           Keypad }
    KC_BA                    = HID_UT_CODE(UT(KB),0x00BA),     // 186          BA           Keypad Tab
    KC_BB                    = HID_UT_CODE(UT(KB),0x00BB),     // 187          BB           Keypad Backspace
    KC_BC                    = HID_UT_CODE(UT(KB),0x00BB),     // 188          BC           Keypad A
    KC_BD                    = HID_UT_CODE(UT(KB),0x00BD),     // 189          BD           Keypad B
    KC_BE                    = HID_UT_CODE(UT(KB),0x00BE),     // 190          BE           Keypad C
    KC_BF                    = HID_UT_CODE(UT(KB),0x00BF),     // 191          BF           Keypad D
    KC_C0                    = HID_UT_CODE(UT(KB),0x00C0),     // 192          C0           Keypad E
    KC_C1                    = HID_UT_CODE(UT(KB),0x00C1),     // 193          C1           Keypad F
    KC_C2                    = HID_UT_CODE(UT(KB),0x00C2),     // 194          C2           Keypad XOR
    KC_C3                    = HID_UT_CODE(UT(KB),0x00C3),     // 195          C3           Keypad ^
    KC_C4                    = HID_UT_CODE(UT(KB),0x00C4),     // 196          C4           Keypad %
    KC_C5                    = HID_UT_CODE(UT(KB),0x00C5),     // 197          C5           Keypad <
    KC_C6                    = HID_UT_CODE(UT(KB),0x00C6),     // 198          C6           Keypad >
    KC_C7                    = HID_UT_CODE(UT(KB),0x00C7),     // 199          C7           Keypad &
    KC_C8                    = HID_UT_CODE(UT(KB),0x00C8),     // 200          C8           Keypad &&
    KC_C9                    = HID_UT_CODE(UT(KB),0x00C9),     // 201          C9           Keypad |
    KC_CA                    = HID_UT_CODE(UT(KB),0x00CA),     // 202          CA           Keypad ||
    KC_CB                    = HID_UT_CODE(UT(KB),0x00CB),     // 203          CB           Keypad :
    KC_CC                    = HID_UT_CODE(UT(KB),0x00CC),     // 204          CC           Keypad #
    KC_CD                    = HID_UT_CODE(UT(KB),0x00CD),     // 205          CD           Keypad Space
    KC_CE                    = HID_UT_CODE(UT(KB),0x00CE),     // 206          CE           Keypad @
    KC_CF                    = HID_UT_CODE(UT(KB),0x00CF),     // 207          CF           Keypad !
    KC_D0                    = HID_UT_CODE(UT(KB),0x00D0),     // 208          D0           Keypad Memory Store
    KC_D1                    = HID_UT_CODE(UT(KB),0x00D1),     // 209          D1           Keypad Memory Recall
    KC_D2                    = HID_UT_CODE(UT(KB),0x00D2),     // 210          D2           Keypad Memory Clear
    KC_D3                    = HID_UT_CODE(UT(KB),0x00D3),     // 211          D3           Keypad Memory Add
    KC_D4                    = HID_UT_CODE(UT(KB),0x00D4),     // 212          D4           Keypad Memory Subtract
    KC_D5                    = HID_UT_CODE(UT(KB),0x00D5),     // 213          D5           Keypad Memory Multiply
    KC_D6                    = HID_UT_CODE(UT(KB),0x00D6),     // 214          D6           Keypad Memory Divide
    KC_D7                    = HID_UT_CODE(UT(KB),0x00D7),     // 215          D7           Keypad +/-
    KC_D8                    = HID_UT_CODE(UT(KB),0x00D8),     // 216          D8           Keypad Clear
    KC_D9                    = HID_UT_CODE(UT(KB),0x00D9),     // 217          D9           Keypad Clear Entry
    KC_DA                    = HID_UT_CODE(UT(KB),0x00DA),     // 218          DA           Keypad Binary
    KC_DB                    = HID_UT_CODE(UT(KB),0x00DB),     // 219          DB           Keypad Octal
    KC_DC                    = HID_UT_CODE(UT(KB),0x00DC),     // 220          DC           Keypad Decimal
    KC_DD                    = HID_UT_CODE(UT(KB),0x00DD),     // 221          DD           Keypad Hexadecimal
    KC_DE                    = HID_UT_CODE(UT(KB),0x00DE),     // 222-223      DE-DF        Reserved
    KC_LCTL                  = HID_UT_CODE(UT(KB),0x00E0),     // 224          E0           Keyboard LeftControl 58 √ √ √ 4/101/104
    KC_LSFT                  = HID_UT_CODE(UT(KB),0x00E1),     // 225          E1           Keyboard LeftShift 44 √ √ √ 4/101/104
    KC_LALT                  = HID_UT_CODE(UT(KB),0x00E2),     // 226          E2           Keyboard LeftAlt 60 √ √ √ 4/101/104
    KC_LGUI                  = HID_UT_CODE(UT(KB),0x00E3),     // 227          E3           Keyboard Left GUI10;23 127 √ √ √ 104
    KC_RCTL                  = HID_UT_CODE(UT(KB),0x00E4),     // 228          E4           Keyboard RightControl 64 √ √ √ 101/104
    KC_RSFT                  = HID_UT_CODE(UT(KB),0x00E5),     // 229          E5           Keyboard RightShift 57 √ √ √ 4/101/104
    KC_RALT                  = HID_UT_CODE(UT(KB),0x00E6),     // 230          E6           Keyboard RightAlt 62 √ √ √ 101/104
    KC_RGUI                  = HID_UT_CODE(UT(KB),0x00E7),     // 231          E7           Keyboard Right GUI10;24 128 √ √ √ 104
    KC_E8                    = HID_UT_CODE(UT(KB),0x00E8),     // 232-65535 E8-FFFF          Reserved

/*************************** 10 Keyboard/Keypad Page (0x07) - Alias ********************************/
    KC_TRNS      = KC_TRANSPARENT,
    KC_ENT       = KC_ENTER,
    KC_ESC       = KC_ESCAPE,
    KC_BSPC      = KC_BACKSPACE,
    KC_SPC       = KC_SPACE,
    KC_MINS      = KC_MINUS,
    KC_EQL       = KC_EQUAL,
    KC_LBRC      = KC_LEFT_BRACKET,
    KC_RBRC      = KC_RIGHT_BRACKET,
    KC_BSLS      = KC_BACKSLASH,
    KC_NUHS      = KC_NONUS_HASH,
    KC_SCLN      = KC_SEMICOLON,
    KC_QUOT      = KC_QUOTE,
    KC_GRV       = KC_GRAVE,
    KC_COMM      = KC_COMMA,
    KC_SLSH      = KC_SLASH,
    KC_CAPS      = KC_CAPS_LOCK,
    KC_PSCR      = KC_PRINT_SCREEN,
    KC_SCRL      = KC_SCROLL_LOCK,
    KC_BRMD      = KC_SCROLL_LOCK,
    KC_PAUS      = KC_PAUSE,
    KC_BRK       = KC_PAUSE,
    KC_BRMU      = KC_PAUSE,
    KC_INS       = KC_INSERT,
    KC_PGUP      = KC_PAGE_UP,
    KC_DEL       = KC_DELETE,
    KC_PGDN      = KC_PAGE_DOWN,
    KC_RGHT      = KC_RIGHT,
    KC_NUM       = KC_NUM_LOCK,
    KC_PSLS      = KC_KP_SLASH,
    KC_PAST      = KC_KP_ASTERISK,
    KC_PMNS      = KC_KP_MINUS,
    KC_PPLS      = KC_KP_PLUS,
    KC_PENT      = KC_KP_ENTER,
    KC_P1        = KC_KP_1,
    KC_P2        = KC_KP_2,
    KC_P3        = KC_KP_3,
    KC_P4        = KC_KP_4,
    KC_P5        = KC_KP_5,
    KC_P6        = KC_KP_6,
    KC_P7        = KC_KP_7,
    KC_P8        = KC_KP_8,
    KC_P9        = KC_KP_9,
    KC_P0        = KC_KP_0,
    KC_PDOT      = KC_KP_DOT,
    KC_NUBS      = KC_NONUS_BACKSLASH,
    KC_APP       = KC_APPLICATION,
    KC_PEQL      = KC_KP_EQUAL,
    KC_EXEC      = KC_EXECUTE,
    KC_SLCT      = KC_SELECT,
    KC_AGIN      = KC_AGAIN,
    KC_PSTE      = KC_PASTE,
    KC_LCAP      = KC_LOCKING_CAPS_LOCK,
    KC_LNUM      = KC_LOCKING_NUM_LOCK,
    KC_LSCR      = KC_LOCKING_SCROLL_LOCK,
    KC_PCMM      = KC_KP_COMMA,
    KC_INT1      = KC_INTERNATIONAL_1,
    KC_INT2      = KC_INTERNATIONAL_2,
    KC_INT3      = KC_INTERNATIONAL_3,
    KC_INT4      = KC_INTERNATIONAL_4,
    KC_INT5      = KC_INTERNATIONAL_5,
    KC_INT6      = KC_INTERNATIONAL_6,
    KC_INT7      = KC_INTERNATIONAL_7,
    KC_INT8      = KC_INTERNATIONAL_8,
    KC_INT9      = KC_INTERNATIONAL_9,
    KC_LNG1      = KC_LANGUAGE_1,
    KC_LNG2      = KC_LANGUAGE_2,
    KC_LNG3      = KC_LANGUAGE_3,
    KC_LNG4      = KC_LANGUAGE_4,
    KC_LNG5      = KC_LANGUAGE_5,
    KC_LNG6      = KC_LANGUAGE_6,
    KC_LNG7      = KC_LANGUAGE_7,
    KC_LNG8      = KC_LANGUAGE_8,
    KC_LNG9      = KC_LANGUAGE_9,
    KC_ERAS      = KC_ALTERNATE_ERASE,
    KC_SYRQ      = KC_SYSTEM_REQUEST,
    KC_CNCL      = KC_CANCEL,
    KC_CLR       = KC_CLEAR,
    KC_PRIR      = KC_PRIOR,
    KC_RETN      = KC_RETURN,
    KC_SEPR      = KC_SEPARATOR,
    KC_CLAG      = KC_CLEAR_AGAIN,
    KC_CRSL      = KC_CRSEL,
    KC_EXSL      = KC_EXSEL,

/*************************** <HID Usage Tables> - 4 Generic Desktop Page (0x01) ********************************/
//                                                                                     // Usage ID     Usage Name                                  Usage Type    Section
    KC(DESKTOP,POWER_DOWN)                     = HID_UT_CODE(UT(DESK),0x0081),         // 81           System Power Down                           OSC           4.5       // 系统掉电
    KC(DESKTOP,SLEEP)                          = HID_UT_CODE(UT(DESK),0x0082),         // 82           System Sleep                                OSC           4.5.1     // 系统睡眠, 已测试
    KC(DESKTOP,WAKE_UP)                        = HID_UT_CODE(UT(DESK),0x0083),         // 83           System Wake Up                              OSC           4.5.1     // 系统唤醒
    KC(DESKTOP,MIC_MUTE)                       = HID_UT_CODE(UT(DESK),0x00A9),         // A9           System Microphone Mute [77]                 OOC           4.5       // 系统麦克风静音
/*************************** 4 Generic Desktop Page (0x01) - Alias ********************************/
    KC_POWER                                   = KC(DESKTOP,POWER_DOWN),
    KC_SLEEP                                   = KC(DESKTOP,SLEEP),
    KC_WAKE                                    = KC(DESKTOP,WAKE_UP),
    KC_MICM                                    = KC(DESKTOP,MIC_MUTE),

/*************************** <HID Usage Tables> - 6 VR Controls Page (0x03) ********************************/

/*************************** <HID Usage Tables> - 6 VR Controls Page (0x03) - Alias ********************************/

/*************************** <HID Usage Tables> - Sport Controls Page (0x04) ********************************/

/*************************** <HID Usage Tables> - Sport Controls Page (0x04) - Alias ********************************/

/*************************** <HID Usage Tables> - 8 Game Controls Page (0x05) ********************************/

/*************************** <HID Usage Tables> - 8 Game Controls Page (0x05) - Alias ********************************/

/*************************** <HID Usage Tables> - 15 Consumer Page (0x0C) ********************************
*All controls on the Consumer page are application-specific. That is, they affect a specific device, not the system as a whole.
----------------------------------------------------------------------------------------------------------
Wootility 键盘特殊字符数据抓包
下一首：                   Consumer page, 0x00B5
上一首：                   Consumer page, 0x00B6
播放暂停：                 Consumer page, 0x00CD
停止播放：                 Consumer page, 0x00B7
静音：                     Consumer page, 0x00E2
音量增加：                 Consumer page, 0x00E9
音量减少：                 Consumer page, 0x00EA
打开多媒体播放器：         Consumer page, 0x0183
打开电子邮件客户端：       Consumer page, 0x018A
打开计算器：               Consumer page, 0x0192
打开文件浏览器：           Consumer page, 0x0194
搜索：                     Consumer page, 0x0221
打开浏览器：               Consumer page, 0x0223
上一页(浏览器)：           Consumer page, 0x0224
下一页(浏览器)：           Consumer page, 0x0225
刷新(浏览器)：             Consumer page, 0x0227
书签(浏览器)：             Consumer page, 0x022A
关机：                     =====================
睡眠：                     =====================
MacOS任务控制中心：        Consumer page, 0x029F
MacOS启动台：              Consumer page, 0x02A2
降低屏幕亮度：             Consumer page, 0x0070
提高屏幕亮度：             Consumer page, 0x006F
鼠标左键：                 03 01 00
鼠标右键：                 03 02 00
鼠标中键：                 03 04 00
鼠标侧面返回键：           03 08 00
鼠标侧面前进键：           03 10 00
**********************************************************************************************************/
//                                                                                      // Usage ID     Usage Name                                  Usage Type    Section
    KC(CONSUMER,MICROPHONE)                     = HID_UT_CODE(UT(CONSUMER),0x0004),     // 04           Microphone                                  CA            15.1   // 麦克风
    KC(CONSUMER,NEXT_TRACK)                     = HID_UT_CODE(UT(CONSUMER),0x00B5),     // B5           Scan Next Track                             OSC           15.7   // 下一曲
    KC(CONSUMER,PREVIOUS_TRACK)                 = HID_UT_CODE(UT(CONSUMER),0x00B6),     // B6           Scan Previous Track                         OSC           15.7   // 上一曲
    KC(CONSUMER,STOP)                           = HID_UT_CODE(UT(CONSUMER),0x00B7),     // B7           Stop                                        OSC           15.7   // 停止
    KC(CONSUMER,PLAY_PAUSE)                     = HID_UT_CODE(UT(CONSUMER),0x00CD),     // CD           Play/Pause                                  OSC           15.7   // 播放/暂停
    KC(CONSUMER,MUTE)                           = HID_UT_CODE(UT(CONSUMER),0x00E2),     // E2           Mute                                        OOC           15.9.1 // 静音
    KC(CONSUMER,VOLUME_INCREMENT)               = HID_UT_CODE(UT(CONSUMER),0x00E9),     // E9           Volume Increment                            RTC           15.9.1 // 音量加
    KC(CONSUMER,VOLUME_DECREMENT)               = HID_UT_CODE(UT(CONSUMER),0x00EA),     // EA           Volume Decrement                            RTC           15.9.1 // 音量减
    KC(CONSUMER,CONTROL_CFG)                    = HID_UT_CODE(UT(CONSUMER),0x0183),     // 183          AL Consumer Control Configuration           Sel           15.15  // 多媒体播放器
    KC(CONSUMER,EMAIL_READER)                   = HID_UT_CODE(UT(CONSUMER),0x018A),     // 18A          AL Email Reader                             Sel           15.15  // 邮件
    KC(CONSUMER,CALCULATOR)                     = HID_UT_CODE(UT(CONSUMER),0x0192),     // 192          AL Calculator                               Sel           15.15  // 计算器
    KC(CONSUMER,MACHINE_BROWSER)                = HID_UT_CODE(UT(CONSUMER),0x0194),     // 194          AL Local Machine Browser                    Sel           15.15  // 我的电脑
    KC(CONSUMER,WWW_SEARCH)                     = HID_UT_CODE(UT(CONSUMER),0x0221),     // 221          AC Search                                   Sel           15.16  // 搜索
    KC(CONSUMER,WWW_HOME)                       = HID_UT_CODE(UT(CONSUMER),0x0223),     // 223          AC Home                                     Sel           15.16  // 浏览器
    KC(CONSUMER,BACK)                           = HID_UT_CODE(UT(CONSUMER),0x0224),     // 224          AC Back                                     Sel           15.16  // 上一页(浏览器)
    KC(CONSUMER,FORWARD)                        = HID_UT_CODE(UT(CONSUMER),0x0225),     // 225          AC Forward                                  Sel           15.16  // 下一页(浏览器)
    KC(CONSUMER,AC_STOP)                        = HID_UT_CODE(UT(CONSUMER),0x0226),     // 226          AC Stop                                     Sel           15.16  // 停止(浏览器)
    KC(CONSUMER,REFRESH)                        = HID_UT_CODE(UT(CONSUMER),0x0227),     // 227          AC Refresh                                  Sel           15.16  // 刷新(浏览器)
    KC(CONSUMER,BOOKMARKS)                      = HID_UT_CODE(UT(CONSUMER),0x022A),     // 22A          AC Bookmarks                                Sel           15.16  // 书签(浏览器)
    KC(CONSUMER,SCREEN_INCREMENT)               = HID_UT_CODE(UT(CONSUMER),0x006F),     // 06F                                                                           // 屏幕亮度增加
    KC(CONSUMER,SCREEN_DECREMENT)               = HID_UT_CODE(UT(CONSUMER),0x0070),     // 070                                                                           // 屏幕亮度降低
    KC(CONSUMER,MACOS_TASK)                     = HID_UT_CODE(UT(CONSUMER),0x029F),     // 029F                                                                          // MacOS任务控制中心
    KC(CONSUMER,MACOS_START)                    = HID_UT_CODE(UT(CONSUMER),0x02A2),     // 02A2                                                                          // MacOS启动台
/*************************** 15 Consumer Page (0x0C) - Alias ********************************/
    KC_PLAY                                     = KC_CONSUMER_PLAY_PAUSE,
    KC_NTRACK                                   = KC(CONSUMER,NEXT_TRACK),
    KC_PTRACK                                   = KC(CONSUMER,PREVIOUS_TRACK),
    KC_CSTOP                                    = KC(CONSUMER,STOP),
    KC_MUTE                                     = KC(CONSUMER,MUTE),
    KC_VINC                                     = KC(CONSUMER,VOLUME_INCREMENT),
    KC_VDEC                                     = KC(CONSUMER,VOLUME_DECREMENT),
    KC_MEDIA                                    = KC(CONSUMER,CONTROL_CFG),
    KC_EMAIL                                    = KC(CONSUMER,EMAIL_READER),
    KC_CALC                                     = KC(CONSUMER,CALCULATOR),
    KC_PC                                       = KC(CONSUMER,MACHINE_BROWSER),
    KC_WWW                                      = KC(CONSUMER,WWW_HOME),
    KC_SEARCH                                   = KC(CONSUMER,WWW_SEARCH),
    KC_AC_BACK                                  = KC(CONSUMER,BACK),
    KC_FORWARD                                  = KC(CONSUMER,FORWARD),
    KC_MARKS                                    = KC(CONSUMER,BOOKMARKS),
    KC_AC_STOP                                  = KC(CONSUMER,AC_STOP),
    KC_REFRESH                                  = KC(CONSUMER,REFRESH),
    KC_SCREENI                                  = KC(CONSUMER,SCREEN_INCREMENT),
    KC_SCREEND                                  = KC(CONSUMER,SCREEN_DECREMENT),
    KC_MACT                                     = KC(CONSUMER,MACOS_TASK),
    KC_MACS                                     = KC(CONSUMER,MACOS_START),
    KC_MIC                                      = KC(CONSUMER,MICROPHONE),

/*************************** Multi keys - switch layout (0x0<<8) ********************************/
#define MOUSE_ID(id)           HID_UT_PREFIX(MOUSE_ID_,id)    // Mouse id
    HID_UT_PREFIX(MOUSE_ID_,FREE)               = (0x0<<8),                 // Mouse button release
    HID_UT_PREFIX(MOUSE_ID_,LEFT)               = (0x1<<8),                 // Mouse button1 Left
    HID_UT_PREFIX(MOUSE_ID_,RIGHT)              = (0x2<<8),                 // Mouse button2 Right
    HID_UT_PREFIX(MOUSE_ID_,MIDDLE)             = (0x3<<8),                 // Mouse button3 Middle
    HID_UT_PREFIX(MOUSE_ID_,FORWARD)            = (0x4<<8),                 // Mouse button4 Forward
    HID_UT_PREFIX(MOUSE_ID_,BACK)               = (0x5<<8),                 // Mouse button5 Back
    HID_UT_PREFIX(MOUSE_ID_,WHEELV)             = (0xE<<8),                 // Mouse Wheel V
    HID_UT_PREFIX(MOUSE_ID_,WHEELH)             = (0xF<<8),                 // Mouse Wheel H

    KC(MOUSE,FREE)                              = MOUSE_CODE(MOUSE_ID(FREE),    0),       // 释放
    KC(MOUSE,LEFT)                              = MOUSE_CODE(MOUSE_ID(LEFT),    0),       // Mouse button1 Left
    KC(MOUSE,RIGHT)                             = MOUSE_CODE(MOUSE_ID(RIGHT),   0),       // Mouse button2 Right
    KC(MOUSE,MIDDLE)                            = MOUSE_CODE(MOUSE_ID(MIDDLE),  0),       // Mouse button3 Middle
    KC(MOUSE,FORWARD)                           = MOUSE_CODE(MOUSE_ID(FORWARD), 0),       // Mouse button4 Forward
    KC(MOUSE,BACK)                              = MOUSE_CODE(MOUSE_ID(BACK),    0),       // Mouse button5 Back
    // 测试项
    KC(MOUSE,ZHR)                               = MOUSE_CODE(MOUSE_ID(WHEELH),  10),      // Wheel ZH
    KC(MOUSE,ZHL)                               = MOUSE_CODE(MOUSE_ID(WHEELH),  (246)),   // Wheel ZH
    KC(MOUSE,ZVR)                               = MOUSE_CODE(MOUSE_ID(WHEELV),  10),      // Wheel ZH
    KC(MOUSE,ZVL)                               = MOUSE_CODE(MOUSE_ID(WHEELV),  (246)),   // Wheel ZH

/*************************** function keys - switch layout (0x0<<8) - Alias ********************************/
    MOUSE_FREE                                  = KC(MOUSE,FREE),
    MOUSE_LEFT                                  = KC(MOUSE,LEFT),
    MOUSE_RIGHT                                 = KC(MOUSE,RIGHT),
    MOUSE_MID                                   = KC(MOUSE,MIDDLE),
    MOUSE_FORWARD                               = KC(MOUSE,FORWARD),
    MOUSE_BACK                                  = KC(MOUSE,BACK),
    MOUSE_ZHR                                   = KC(MOUSE,ZHR),
    MOUSE_ZHL                                   = KC(MOUSE,ZHL),
    MOUSE_ZVR                                   = KC(MOUSE,ZVR),
    MOUSE_ZVL                                   = KC(MOUSE,ZVL),

/*************************** 功能键 ******************************************
* 功能键扩展 ID bit[8-11]
******************************************************************************/

/*************************** function keys - switch layout (0x0<<8) ********************************/
    KC(FK,FN0)                                  = FK_CODE(FK_ID(FN),0 ),   // layout 0 , default
    KC(FK,FN1)                                  = FK_CODE(FK_ID(FN),1 ),   // layout 1
    KC(FK,FN2)                                  = FK_CODE(FK_ID(FN),2 ),   // layout 2
    KC(FK,FN3)                                  = FK_CODE(FK_ID(FN),3 ),   // layout 3
    KC(FK,FN4)                                  = FK_CODE(FK_ID(FN),4 ),   // layout 4
    KC(FK,FN5)                                  = FK_CODE(FK_ID(FN),5 ),   // layout 5
    KC(FK,FN6)                                  = FK_CODE(FK_ID(FN),6 ),   // layout 6
    KC(FK,FN7)                                  = FK_CODE(FK_ID(FN),7 ),   // layout 7
    KC(FK,FN8)                                  = FK_CODE(FK_ID(FN),8 ),   // layout 8
    KC(FK,FN9)                                  = FK_CODE(FK_ID(FN),9 ),   // layout 9
    KC(FK,FN10)                                 = FK_CODE(FK_ID(FN),10),   // layout 10
    KC(FK,FN11)                                 = FK_CODE(FK_ID(FN),11),   // layout 11
    KC(FK,FN12)                                 = FK_CODE(FK_ID(FN),12),   // layout 12
    KC(FK,FN13)                                 = FK_CODE(FK_ID(FN),13),   // layout 13
    KC(FK,FN14)                                 = FK_CODE(FK_ID(FN),14),   // layout 14
    KC(FK,FN15)                                 = FK_CODE(FK_ID(FN),15),   // layout 15
    KC(FK,FN16)                                 = FK_CODE(FK_ID(FN),16),   // layout 16

/*************************** function keys - switch layout (0x0<<8) - Alias ********************************/
    FK_FN1                                      = KC_FK_FN1,
    FK_FN2                                      = KC(FK,FN2),
    FK_FN3                                      = KC(FK,FN3),
    FK_FN4                                      = KC(FK,FN4),
    FK_FN5                                      = KC(FK,FN5),
    FK_FN6                                      = KC(FK,FN6),
    FK_FN7                                      = KC(FK,FN7),
    FK_FN8                                      = KC(FK,FN8),
    FK_FN9                                      = KC(FK,FN9),
    FK_FN10                                     = KC(FK,FN10),
    FK_FN11                                     = KC(FK,FN11),
    FK_FN12                                     = KC(FK,FN12),
    FK_FN13                                     = KC(FK,FN13),
    FK_FN14                                     = KC(FK,FN14),
    FK_FN15                                     = KC(FK,FN15),
    FK_FN16                                     = KC(FK,FN16),

/*************************** function keys - system (0x1<<8) ********************************/
    KC(FK,SYS_FACTORY)                          = FK_CODE(FK_ID(SYS), 0x00),   // Factory Reset
    KC(FK,SYS_LOCK_WIN)                         = FK_CODE(FK_ID(SYS), 0x01),   // Lock Win Key
    KC(FK,SYS_UNLOCK_WIN)                       = FK_CODE(FK_ID(SYS), 0x02),   // unLock Win Key
    KC(FK,SYS_JUMP_BOOT)                        = FK_CODE(FK_ID(SYS), 0x03),   // 让 BLE 模块进入 BOOT 模式
    KC(FK,SYS_JUMP_BOOT_DG)                     = FK_CODE(FK_ID(SYS), 0x04),   // 让 Dongle 进入 BOOT 模式
    KC(FK,SYS_TESTING)                          = FK_CODE(FK_ID(SYS), 0x05),   // 压力测试
    KC(FK,SYS_USB)                              = FK_CODE(FK_ID(SYS), 0x06),   // switch USB mode
    KC(FK,SYS_PAIR_RF)                          = FK_CODE(FK_ID(SYS), 0x07),   // switch RF  pair
    KC(FK,SYS_PAIR_BLE1)                        = FK_CODE(FK_ID(SYS), 0x08),   // switch BLE1  pair
    KC(FK,SYS_PAIR_BLE2)                        = FK_CODE(FK_ID(SYS), 0x09),   // switch BLE2  pair
    KC(FK,SYS_PAIR_BLE3)                        = FK_CODE(FK_ID(SYS), 0x0A),   // switch BLE3  pair
    KC(FK,SYS_PAIR_BLE4)                        = FK_CODE(FK_ID(SYS), 0x0B),   // switch BLE4  pair
    KC(FK,SYS_J2BOOT)                           = FK_CODE(FK_ID(SYS), 0x0C),   // Jump to boot
    KC(FK,SYS_WIN_C_ALT)                        = FK_CODE(FK_ID(SYS), 0x0D),   // Win Convert Alt
    KC(FK,SYS_BATT_QUERY)                       = FK_CODE(FK_ID(SYS), 0x0E),   // 查询电池电量
    KC(FK,SYS_RATE_TICK)                        = FK_CODE(FK_ID(SYS), 0x0F),   // 更改回报率
    KC(FK,SYS_TABLE1)                           = FK_CODE(FK_ID(SYS), 0x10),   // 切换参数组1
    KC(FK,SYS_TABLE2)                           = FK_CODE(FK_ID(SYS), 0x11),   // 切换参数组2
    KC(FK,SYS_TABLE3)                           = FK_CODE(FK_ID(SYS), 0x12),   // 切换参数组3
    KC(FK,SYS_TABLE4)                           = FK_CODE(FK_ID(SYS), 0x13),   // 切换参数组4
    KC(FK,SYS_RF_KEY_TEST)                      = FK_CODE(FK_ID(SYS), 0x14),   // RF 测试
    KC(FK,QUICK_SCAN)                           = FK_CODE(FK_ID(SYS), 0x15),   // 快速扫描模式
    KC(FK,CNV_WIN)                              = FK_CODE(FK_ID(SYS), 0x16),   // 切换到 Win 模式
    KC(FK,CNV_MAC)                              = FK_CODE(FK_ID(SYS), 0x17),   // 切换到 MAC 模
    KC(FK,SYS_LOCK_WIN_SHORT)                   = FK_CODE(FK_ID(SYS), 0x18),   // Lock Win Key, short, 锁 WIN键短按
    KC(FK,SYS_SAVE)                             = FK_CODE(FK_ID(SYS), 0x19),   // 组合键保存参数

/*************************** function keys - system (0x1<<8) - Alias ********************************/
    FK_RESET                                    = KC_FK_SYS_FACTORY,
    FK_FACTORY                                  = KC_FK_SYS_FACTORY,
    FK_B2BOOT                                   = KC(FK,SYS_JUMP_BOOT),
    FK_D2BOOT                                   = KC(FK,SYS_JUMP_BOOT_DG),
    FK_TEST                                     = KC(FK,SYS_TESTING),
    FK_LWIN                                     = KC(FK,SYS_LOCK_WIN),
    FK_LWINS                                    = KC(FK,SYS_LOCK_WIN_SHORT),   // Lock Win Key, short, 锁 WIN键短按
    FK_UWIN                                     = KC(FK,SYS_UNLOCK_WIN),
    FK_USB                                      = KC_FK_SYS_USB,
    FK_PRF                                      = KC(FK,SYS_PAIR_RF),
    FK_PBLE1                                    = KC(FK,SYS_PAIR_BLE1),
    FK_PBLE2                                    = KC(FK,SYS_PAIR_BLE2),
    FK_PBLE3                                    = KC(FK,SYS_PAIR_BLE3),
    FK_PBLE4                                    = KC(FK,SYS_PAIR_BLE4),
    FK_J2BOOT                                   = KC(FK,SYS_J2BOOT),
    FK_WCA                                      = KC(FK,SYS_WIN_C_ALT),
    FK_BATT                                     = KC(FK,SYS_BATT_QUERY),
    FK_RATE                                     = KC(FK,SYS_RATE_TICK),   // 更改回报率
    FK_TABLE1                                   = KC(FK,SYS_TABLE1),      // 切换参数组1
    FK_TABLE2                                   = KC(FK,SYS_TABLE2),      // 切换参数组2
    FK_TABLE3                                   = KC(FK,SYS_TABLE3),      // 切换参数组3
    FK_TABLE4                                   = KC(FK,SYS_TABLE4),      // 切换参数组4
    FK_RF_KEY_TEST                              = KC(FK,SYS_RF_KEY_TEST), // RF 测试
    FK_QUICK_SCAN                               = KC(FK,QUICK_SCAN),      // 快速扫描模式
    FK_CNV_WIN                                  = KC(FK,CNV_WIN)  ,       // 切换到 Win 模式
    FK_CNV_MAC                                  = KC(FK,CNV_MAC)  ,       // 切换到 MAC 模
    FK_SYS_SAVE                                 = KC(FK,SYS_SAVE) ,       // 组合键保存参数

/*************************** function keys - RGB (0x2<<8) ********************************/
    KC(FK,RGB)                                  = FK_CODE(FK_ID(RGB), 0x00),    // switch RGB MODE
    KC(FK,RGB_COLOR)                            = FK_CODE(FK_ID(RGB), 0x01),    // switch RGB color
    KC(FK,RGB_LIGHT_INC)                        = FK_CODE(FK_ID(RGB), 0x02),    // increase RGB brightness
    KC(FK,RGB_LIGHT_DEC)                        = FK_CODE(FK_ID(RGB), 0x03),    // decrease RGB brightness
    KC(FK,RGB_SPEED_INC)                        = FK_CODE(FK_ID(RGB), 0x04),    // increase RGB speed
    KC(FK,RGB_SPEED_DEC)                        = FK_CODE(FK_ID(RGB), 0x05),    // decrease RGB speed
    KC(FK,RGB_PREVIEW)                          = FK_CODE(FK_ID(RGB), 0x06),    // RGB preview/prevue
    KC(FK,RGB_TURN_ON)                          = FK_CODE(FK_ID(RGB), 0x07),    // RGB On or off
    KC(FK,RGB_UDEF1)                            = FK_CODE(FK_ID(RGB), 0x08),    // RGB user define 1
    KC(FK,RGB_UDEF2)                            = FK_CODE(FK_ID(RGB), 0x09),    // RGB user define 2
    KC(FK,RGB_UDEF3)                            = FK_CODE(FK_ID(RGB), 0x0A),    // RGB user define 3
    KC(FK,RGB_UDEF4)                            = FK_CODE(FK_ID(RGB), 0x0B),    // RGB user define 4
    KC(FK,RGB_UDEF5)                            = FK_CODE(FK_ID(RGB), 0x0C),    // RGB user define 5
    KC(FK,RGB_RECORD)                           = FK_CODE(FK_ID(RGB), 0x0D),    // RGB user define Record
    KC(FK,RGB_LIGHT)                            = FK_CODE(FK_ID(RGB), 0x0F),    // RGB brightness adj
    KC(FK,RGB_SUB_MODE)                         = FK_CODE(FK_ID(RGB), 0x10),    // Sub RGB mode
    KC(FK,RGB_SUB_LIGHT_INC)                    = FK_CODE(FK_ID(RGB), 0x11),    // increase Sub RGB brightness
    KC(FK,RGB_SUB_LIGHT_DEC)                    = FK_CODE(FK_ID(RGB), 0x12),    // decrease Sub RGB brightness
    KC(FK,RGB_MCOLOR)                           = FK_CODE(FK_ID(RGB), 0x13),    // switch Main RGB color
    KC(FK,RGB_WHITE)                            = FK_CODE(FK_ID(RGB), 0x14),    // show white light
    KC(FK,RGB_LED_STRIP_MODE)                   = FK_CODE(FK_ID(RGB), 0x15),    // LED Strip mode
    KC(FK,RGB_PREV)                             = FK_CODE(FK_ID(RGB), 0x16),    // switch Previous RGB MODE
    KC(FK,CNV_WASD)                             = FK_CODE(FK_ID(RGB), 0x17),    // switch WASD
    KC(FK,RGB_CHECK)                            = FK_CODE(FK_ID(RGB), 0x18),    // 灯光自检
/*************************** function keys - RGB (0x2<<8) - Alias ********************************/
    FK_RGB                                      = KC(FK,RGB),
    FK_COLOR                                    = KC(FK,RGB_COLOR),
    FK_MCOLOR                                   = KC(FK,RGB_MCOLOR),
    FK_LIGHTH                                   = KC(FK,RGB_LIGHT_INC),
    FK_LIGHTL                                   = KC(FK,RGB_LIGHT_DEC),
    FK_SPEEDH                                   = KC(FK,RGB_SPEED_INC),
    FK_SPEEDL                                   = KC(FK,RGB_SPEED_DEC),
    FK_PREVUR                                   = KC(FK,RGB_PREVIEW),
    FK_RGB_ON                                   = KC(FK,RGB_TURN_ON),
    FK_RGB_UDEF1                                = KC(FK,RGB_UDEF1),
    FK_RGB_UDEF2                                = KC(FK,RGB_UDEF2),
    FK_RGB_UDEF3                                = KC(FK,RGB_UDEF3),
    FK_RGB_UDEF4                                = KC(FK,RGB_UDEF4),
    FK_RGB_UDEF5                                = KC(FK,RGB_UDEF5),
    FK_RGB_RECORD                               = KC(FK,RGB_RECORD),
    FK_LIGHT                                    = KC(FK,RGB_LIGHT),
    FK_SUB_MODE                                 = KC(FK,RGB_SUB_MODE),
    FK_SLIGHTH                                  = KC(FK,RGB_SUB_LIGHT_INC),
    FK_SLIGHTL                                  = KC(FK,RGB_SUB_LIGHT_DEC),
    FK_WHITE                                    = KC(FK,RGB_WHITE),
    FK_STRIP_MODE                               = KC(FK,RGB_LED_STRIP_MODE),
    FK_RGB_PREV                                 = KC(FK,RGB_PREV),
    FK_WASD                                     = KC(FK,CNV_WASD),    // switch WASD
    FK_RGB_CHECK                                = KC(FK,RGB_CHECK) ,    // 灯光自检

/*************************** function keys - Multi keys (0x3<<8) ********************************/
    KC(FK,MK_TASK)                              = FK_CODE(FK_ID(MK),0x00),   // Win+Tab

/*************************** function keys - Multi keys (0x3<<8) - Alias ********************************/
    MK_TASK                                     = KC(FK,MK_TASK),

/*************************** function keys - Encoder (0x4<<8) ********************************/
    KC(EC,MEDIA)                                = FK_CODE(FK_ID(EC),0x00),   // 编码器, 多媒体
    KC(EC,RGB_ML)                               = FK_CODE(FK_ID(EC),0x01),   // 编码器, 调灯光模式和颜色

/*************************** function keys - Encoder (0x4<<8) - Alias ********************************/
    EC_MEDIA                                    = KC(EC,MEDIA),
    EC_RGB_ML                                   = KC(EC,RGB_ML),


};

/*************************** Keyboard/Keypad Page code converting ********************************/
//#define KCL(Short)  (HID_UT_CODE(UT(KB),Short))  // Usage Tables Long
//#define KCS(Long)   (Long&0xFF)                  // Usage Tables Short
#define KCL(Short)  (Short)                 // Usage Tables Long
#define KCS(Long)   (Long)                  // Usage Tables Short

#ifdef __cplusplus
}
#endif

#endif  // _HID_USAGE_TABLES_H_
