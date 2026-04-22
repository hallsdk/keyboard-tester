/******************** (C) COPYRIGHT 2018 merafour ********************
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.1.0
* Date               : 2024.04.07
* Description        : OpenAgreementHID.
* Description        : BOARD_ID 定义
* Description        : 注：
* Description        : OpenAgreementHID(OHID) 基于键盘与上位机通讯的需求而开发, 但也可用于非键盘设备, BOARD_ID 用于识别唯一一款设备
* Description        : BOARD_ID 分为三个字段 DEVICE, SUB, INDEX, 分别是 : 设备类, 子类, 序号
********************************************************************************
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#ifndef _OHID_H_
#define _OHID_H_

#include <stdint.h>

#define  OHID_BOARDS_ID_LIST_EXP     1
/******************************************************************************
* 嵌套引用,防止错误地引用头文件, <OHID.h> 与 <OHID/OHID_Board.h> 必须同时被引用,
* 且必须先引用 <OHID.h> 后引用 <OHID/OHID_Board.h>
*******************************************************************************/
#ifndef _OHID_BOARD_H_
#include <OHID/OHID_Board.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
* Board ID 定义
*******************************************************************************/
enum OHID_BOARDS_ID_LIST {
    OHID_BOARD_UNDEFINED              = OHID_BOARD_ID(OHID_DEVICE_UNDEFINED, 0x00,                   0x00),    // 未定义设备, 用于临时使用和扩展
    OHID_BOARD_MACHINE                = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MACHINE_K104,   0x00),    // 104 键机械轴
    OHID_BOARD_OPTICAL                = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_OPTICAL_K104,   0x00),    // 104 键光轴
    // OHID_BOARD_MAGNETIC               = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K104,  0x00),    // 104 键磁轴
	//OHID_BOARD_BK75H                  = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K82,   0x00),    //  82 键磁轴
	//OHID_BOARD_V98H                   = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x00),    //  98 键磁轴
	//OHID_BOARD_51GFH                  = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MACHINE_K80,    0x00),    //  80 键磁轴
    OHID_BOARD_MAGNETIC               = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,  0x00),    // 104 键磁轴
};

/************************ (C) COPYLEFT 2018 Merafour *************************
* 磁轴
******************************************************************************/
// 104
#define  OHID_BOARD_51N               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K104,  0x00)    //  104 键磁轴
#define  OHID_BOARD_51NSSS            OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K104,  0x01)    //  104 键磁轴
#define  OHID_BOARD_TK51QHS370        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K104,  0x02)    //  106 配列磁轴
#define  OHID_BOARD_GX922HS370        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K104,  0x03)    //  106 配列磁轴

// 98
#define  OHID_BOARD_V98H              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x00)    //  98 键磁轴
#define  OHID_BOARD_V98HS             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x01)    //  98 键磁轴, 高速版本
#define  OHID_BOARD_51WH              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x02)    //  98 配列磁轴
#define  OHID_BOARD_51WHOLED          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x03)    //  98 配列磁轴
//#define  OHID_BOARD_51Q               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x04)    //  98 配列磁轴
#define  OHID_BOARD_51WH370           OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x04)    //  98 配列磁轴
#define  OHID_BOARD_V98HS370          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x05)    //  98 配列磁轴
#define  OHID_BOARD_51Q               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x06)    //  98 配列磁轴
#define  OHID_BOARD_CK98UHS370        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x07)    //  98 配列磁轴
#define  OHID_BOARD_51N5C             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x08)    //  98 配列磁轴
#define  OHID_BOARD_51QFS370          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x09)    //  98 配列磁轴
#define  OHID_BOARD_LIUFS370          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x0A)    //  98 配列磁轴
#define  OHID_BOARD_M98               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x10)    //  98 配列磁轴
#define  OHID_BOARD_FS370_WP98S       OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x11)    //  98 配列磁轴
#define  OHID_BOARD_BIN_WP98_V2_5C    OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x0D)    //  98 配列磁轴
#define  OHID_BOARD_BIN_MP98_V2_5C    OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K98,   0x0E)    //  98 配列磁轴

#define  wp75_5c                        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x10)
// 87
#define  OHID_BOARD_52J               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K87,   0x00)    //  87 配列磁轴
#define  OHID_BOARD_TK52J370          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K87,   0x01)    //  87 配列磁轴, 2024.11.05
#define  OHID_BOARD_TK51CU370         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K87,   0x02)    //  87 配列磁轴, 2024.12.02
#define  OHID_BOARD_TK52U370          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K87,   0x03)    //  87 配列磁轴, 2024.12.02

// 82
#define  OHID_BOARD_BK75H             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K82,   0x00)    //  82 键磁轴

// 80
#define  OHID_BOARD_51GFH             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K80,   0x00)    //  80 键磁轴
#define  OHID_BOARD_52L               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K80,   0x01)    //  80 键磁轴

// 75
#define  OHID_BOARD_52G               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x00)    //  75 配列磁轴
#define  OHID_BOARD_EK75              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x01)    //  75 配列磁轴
#define  OHID_BOARD_EK75PLUS          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x02)    //  75 配列磁轴
#define  OHID_BOARD_52LF              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x03)    //  75 配列磁轴, 泰凌微三模
#define  OHID_BOARD_BK75H370          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x03)    //  75 配列磁轴
#define  OHID_BOARD_TK52L370          OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x04)    //  75 配列磁轴
#define  OHID_BOARD_SSS75             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x05)    //  75 配列磁轴,单总线方式
#define  OHID_BOARD_ZONEX75           OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x06)    //  75 配列磁轴, 370
#define  OHID_BOARD_CK8971X75         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x07)    //  75 配列磁轴, 370
#define  OHID_BOARD_TK52LU370_CY      OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x08)    //  75 配列磁轴
#define  OHID_BOARD_TK52LU370_DS      OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x09)    //  75 配列磁轴
#define  OHID_BOARD_NP75HS370         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x0A)    //  75 配列磁轴
#define  OHID_BOARD_ZONEX75HS         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x0B)    //  75 配列磁轴, 370
#define  OHID_BOARD_TK51DHS370        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x0C)    //  75 配列磁轴
#define  OHID_BOARD_FS370_WP75        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x16)    //  75 配列磁轴,网吧版本
#define  OHID_BOARD_FS370_XP75        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x17)    //  75 配列磁轴,网吧版本
// #define  OHID_BOARD_FS370_XP75        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K75,   0x17)    //  75 配列磁轴,网吧版本

// 68
#define  OHID_BOARD_AE68              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x00)    //  68 键磁轴
#define  OHID_BOARD_KG015_370         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x01)    //  68 键磁轴
#define  OHID_BOARD_TK52R_EK68        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x02)    //  68 键磁轴
#define  OHID_BOARD_TK52C_EK68        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x03)    //  68 键磁轴
#define  OHID_BOARD_TKG332            OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x04)    //  68 键磁轴
#define  OHID_BOARD_TK52CUFS370       OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x05)    //  68 键磁轴
#define  OHID_BOARD_KC650HS370        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x06)    //  68 键磁轴
#define  OHID_BOARD_TK52GHS370        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x07)    //  68 键磁轴
#define  OHID_BOARD_FS_GK6830         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x08)    //  68 键磁轴
#define  OHID_BOARD_FS_GK6830A        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x09)    //  68 键磁轴
#define  OHID_BOARD_TKG332_V6         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x0A)    //  68 键磁轴
#define  OHID_BOARD_ON_S2012PD        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x0B)    //  68 键磁轴
#define  OHID_BOARD_FS_GK6813         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x0C)    //  68 键磁轴
#define  OHID_BOARD_FS_GK8050         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x0D)    //  68 键磁轴
#define  OHID_BOARD_HS370_CK68        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x0E)    //  68 键磁轴
#define  OHID_BOARD_HS_GK8050         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x0F)    //  68 键磁轴
#define  OHID_BOARD_HS_GK6950         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x10)    //  68 键磁轴
#define  OHID_BOARD_FS_KD65_XU67      OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x11)    //  68 键磁轴
#define  OHID_BOARD_FS_GK6950         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K68,   0x12)    //  68 键磁轴

// 64
#define  OHID_BOARD_CZ64              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K64,   0x00)    //  64 键磁轴
#define  OHID_BOARD_KC65              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K64,   0x01)    //  65 键磁轴
#define  OHID_BOARD_TKG325            OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K64,   0x02)    //  65 键磁轴

// 60
#define  OHID_BOARD_H60               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x00)    //  61 键磁轴,寒梅
#define  OHID_BOARD_M60               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x01)    //  61 键磁轴,寒梅
#define  OHID_BOARD_Q60               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x02)    //  61 键磁轴,晴天
#define  OHID_BOARD_J60               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x03)    //  61 键磁轴,巨浪
#define  OHID_BOARD_JNS60             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x04)    //  61 键磁轴,剑廿三
#define  OHID_BOARD_MJH60             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x05)    //  61 键磁轴,满江红
#define  OHID_BOARD_M60B              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x06)    //  61 键磁轴,寒梅 标准版本
#define  OHID_BOARD_M60BHS            OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x07)    //  61 键磁轴,寒梅 标准版本HS
#define  OHID_BOARD_TK10_61KEY        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x08)    //  61 键磁轴
#define  OHID_BOARD_DEEP60HS370       OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K61,   0x09)    //  61 键磁轴

// 16
#define  OHID_BOARD_Y16               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K16,   0x00)    //  16 键磁轴,花妖
#define  OHID_BOARD_Y16               OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K16,   0x00)    //  16 键磁轴,花妖

/************************ (C) COPYLEFT 2018 Merafour *************************
* 机械轴
******************************************************************************/
#define  OHID_BOARD_QL108             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MACHINE_K104,   0x00)    //  148(104) 键机械轴
#define  OHID_BOARD_QingMei87         OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MACHINE_K87,    0x00)    //  87 键机械轴
#define  OHID_BOARD_SD60              OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MACHINE_K61,    0x00)    //  61       键机械轴,蜀道

/************************ (C) COPYLEFT 2018 Merafour *************************
* 光轴
******************************************************************************/
#define  OHID_BOARD_TK50T             OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_OPTICAL_K98,   0x00)    //  98 键光轴
#define  OHID_BOARD_TK50T_5020        OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_OPTICAL_K98,   0x01)    //  98 键光轴, 使用 5020 驱动

/******************************************************************************
* 以下内容需在 Target.h 文件中定义,并引用 OHID.h 文件
*******************************************************************************/
#if 0
#define OHID_VERM      1   // master , 主版本号(V1.0.01)
#define OHID_VERS      0   // sub    , 次版本号
#define OHID_VERR     01   // revise , 修订号

#define OHID_TARGET             BK75H
#define OHID_TARGET_ID          OHID_TARGET_BOARD(OHID_TARGET)
#define OHID_TARGET_DES         OHID_TARGET_BOARD_DES(OHID_TARGET)

#ifndef OHID_TARGET_ID
#error  "Please define OHID_TARGET_ID"
#endif
#ifndef OHID_TARGET
#error  "Please define OHID_TARGET"
#endif

//#define OHID_Version        "OHID BOOT V1.0.0"
#define TARGET_Version        OHID_VERSION_STR(OHID_VERM,OHID_VERS,OHID_VERR)
#define TARGET_Version_BCD    OHID_VERSION_BCD(OHID_VERM,OHID_VERS,OHID_VERR)
#endif

#ifdef __cplusplus
}
#endif

#endif // _OHID_H_

/************************** (C) COPYRIGHT Merafour **************************/
