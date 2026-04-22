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
#ifndef _OHID_BOARD_H_
#define _OHID_BOARD_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OHID
#define OHID(prefix,field)          OHID_##prefix##_##field//
#endif
/******************************************************************************
* 宏定义
*******************************************************************************/
#define OHID_TARGET_STR(des)                   #des
#define OHID_TARGET_PRE(prefix,name)           prefix##_##name
//#define _OHID_TARGET_BOARD(target)            (OHID_TARGET_STR(OHID_BOARD)##_##target)
//#define _OHID_TARGET_BOARD(target)            OHID_BOARD_##target
#define OHID_TARGET_BOARD(target)              OHID_TARGET_PRE(OHID_BOARD,target)
//#define OHID_TARGET_BOARD(target)              OHID_TARGET_NAME(OHID_TARGET_PRE(OHID_BOARD),target)//_OHID_TARGET_BOARD(target)
//#define _OHID_TARGET_API(target)              OHID_API_##target
#define OHID_TARGET_API(target)                OHID_TARGET_PRE(OHID_API,target)
#define OHID_TARGET_BOARD_DES(target)          OHID_TARGET_STR(target)
//#define __OHID_VERSION_STR(Version)            (#Version)
//#define _OHID_VERSION_STR(Master,Sub,Revise)   OHID_TARGET_STR(FW V##Master.Sub.Revise)
//#define OHID_VERSION_STR(Master,Sub,Revise)    _OHID_VERSION_STR(Master,Sub,Revise)
#define _OHID_VERSION_STR(prefix,Master,Sub,Revise)   OHID_TARGET_STR(prefix##V##Master.Sub.Revise)
#define OHID_VERSION_STR(prefix,Master,Sub,Revise)    _OHID_VERSION_STR(prefix,Master,Sub,Revise)
#define _OHID_VERSION_BCD(Master,Sub,Revise)   (0x##Master##Sub##Revise)
#define _OHID_VERSION_ZBCD(Master,Sub,Revise)  (0x##Master##Sub##0##Revise)
//#define OHID_BCD(BCD)                          (0x##BCD)
#define OHID_VERSION_BCD(Master,Sub,Revise)    _OHID_VERSION_BCD(Master,Sub,Revise)
#define OHID_VERSION_ZBCD(Master,Sub,Revise)    _OHID_VERSION_BCD(Master,Sub,Revise)

/*
 * 数据包格式:
 * | board_id[32bit]                   |
 * | DEVICE[8b] | SUB[16b] | INDEX[8b] |
 * DEVICE: 设备类
 * SUB  : 子类
 * INDEX    : 序号
 */
#define  OHID_BOARD_ID(DEVICE,SUB,INDEX)     ( (((DEVICE)<<24)&0xFF000000) | (((SUB)<<8)&0x00FFFF00) | ((INDEX)&0x00FF) )
#define  OHID_GET_DEVICE(board_id)           ((board_id>>24)&0xFF)
#define  OHID_GET_SUB(board_id)              ((board_id>>8)&0xFFFF)
#define  OHID_GET_ID(board_id)               ((board_id)&0x00FF)

enum OHID_BOARD_DEVICE {   // 8bit
    OHID(BOARD_DEVICE,UNDEFINED)         = 0x00000000U,   //  undefined, 未定义设备, 用于未明确定义的设备
    OHID(BOARD_DEVICE,KEYBOARD)          = 0x00000001U,   //  键盘
    OHID(BOARD_DEVICE,BLE)               = 0x00000002U,   //  BLE 模组
    OHID(BOARD_DEVICE,RGB)               = 0x00000003U,   //  RGB 模组
    OHID(BOARD_DEVICE,MOUSE)             = 0x00000004U,   //  Mouse
    OHID(BOARD_DEVICE,XBOX)              = 0x00000005U,   //  Xbox
#if 0
    /*************************** Alias ********************************/
    OHID_DEVICE_UNDEFINED                = OHID(BOARD_DEVICE,UNDEFINED),   //  undefined, 未定义设备, 用于未明确定义的设备
    OHID_DEVICE_KEYBOARD                 = OHID(BOARD_DEVICE,KEYBOARD) ,   //  键盘
    OHID_DEVICE_BLE                      = OHID(BOARD_DEVICE,BLE)      ,   //  BLE 模组
    OHID_DEVICE_RGB                      = OHID(BOARD_DEVICE,RGB)      ,   //  RGB 模组
    OHID_DEVICE_MOUSE                    = OHID(BOARD_DEVICE,MOUSE)    ,   //  Mouse
    OHID_DEVICE_XBOX                     = OHID(BOARD_DEVICE,XBOX)     ,   //  Xbox
#endif
};
/*************************** Alias ********************************/
#define     OHID_DEVICE_UNDEFINED        0x00000000U   //  undefined, 未定义设备, 用于未明确定义的设备
#define     OHID_DEVICE_KEYBOARD         0x00000001U   //  键盘
#define     OHID_DEVICE_BLE              0x00000002U   //  BLE 模组
#define     OHID_DEVICE_RGB              0x00000003U   //  RGB 模组
#define     OHID_DEVICE_MOUSE            0x00000004U   //  Mouse
#define     OHID_DEVICE_XBOX             0x00000005U   //  Xbox
/*
** 键盘的子类为键盘的配列
** 注:预处理无法识别枚举值, 故,这些值得使用宏定义
**/
enum OHID_BOARD_SUB_KEYBOARD {
    OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K104)            = 0x0000A400U,   // 104键位(默认)
    OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K99)             = 0x00009900U,   // 99键位
    OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K98)             = 0x00009800U,   // 98键位
    OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K87)             = 0x00008700U,   // 87键位
    OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K82)             = 0x00008200U,   // 82键位
    OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K80)             = 0x00008000U,   // 80键位
    OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K75)             = 0x00007500U,   // 75键位
    OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K61)             = 0x00006100U,   // 61键位
    OHID(BOARD_SUB_KEYBOARD,OPTICAL_K104)             = 0x0000A410U,   // 104键位(默认)
    OHID(BOARD_SUB_KEYBOARD,OPTICAL_K99)              = 0x00009910U,   // 99键位
    OHID(BOARD_SUB_KEYBOARD,OPTICAL_K98)              = 0x00009810U,   // 98键位
    OHID(BOARD_SUB_KEYBOARD,OPTICAL_K87)              = 0x00008710U,   // 87键位
    OHID(BOARD_SUB_KEYBOARD,OPTICAL_K82)              = 0x00008210U,   // 82键位
    OHID(BOARD_SUB_KEYBOARD,OPTICAL_K80)              = 0x00008010U,   // 80键位
    OHID(BOARD_SUB_KEYBOARD,OPTICAL_K75)              = 0x00007510U,   // 75键位
    OHID(BOARD_SUB_KEYBOARD,OPTICAL_K61)              = 0x00006110U,   // 61键位
    OHID(BOARD_SUB_KEYBOARD,MACHINE_K104)             = 0x0000A420U,   // 104键位(默认)
    OHID(BOARD_SUB_KEYBOARD,MACHINE_K99)              = 0x00009920U,   // 99键位
    OHID(BOARD_SUB_KEYBOARD,MACHINE_K98)              = 0x00009820U,   // 98键位
    OHID(BOARD_SUB_KEYBOARD,MACHINE_K87)              = 0x00008720U,   // 87键位
    OHID(BOARD_SUB_KEYBOARD,MACHINE_K82)              = 0x00008220U,   // 82键位
    OHID(BOARD_SUB_KEYBOARD,MACHINE_K80)              = 0x00008020U,   // 80键位
    OHID(BOARD_SUB_KEYBOARD,MACHINE_K75)              = 0x00007520U,   // 75键位
    OHID(BOARD_SUB_KEYBOARD,MACHINE_K61)              = 0x00006120U,   // 61键位
#if 0
    /*************************** Alias ********************************/
    OHID_KB_MAGNETIC_K104                             = OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K104) ,   // 104键位(默认)
    OHID_KB_MAGNETIC_K99                              = OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K99)  ,   // 99键位
    OHID_KB_MAGNETIC_K98                              = OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K98)  ,   // 98键位
    OHID_KB_MAGNETIC_K87                              = OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K87)  ,   // 87键位
    OHID_KB_MAGNETIC_K82                              = OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K82)  ,   // 82键位
    OHID_KB_MAGNETIC_K80                              = OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K80)  ,   // 80键位
    OHID_KB_MAGNETIC_K75                              = OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K75)  ,   // 75键位
    OHID_KB_MAGNETIC_K61                              = OHID(BOARD_SUB_KEYBOARD,MAGNETIC_K61)  ,   // 61键位
    OHID_KB_OPTICAL_K104                              = OHID(BOARD_SUB_KEYBOARD,OPTICAL_K104)  ,   // 104键位(默认)
    OHID_KB_OPTICAL_K99                               = OHID(BOARD_SUB_KEYBOARD,OPTICAL_K99)   ,   // 99键位
    OHID_KB_OPTICAL_K98                               = OHID(BOARD_SUB_KEYBOARD,OPTICAL_K98)   ,   // 98键位
    OHID_KB_OPTICAL_K87                               = OHID(BOARD_SUB_KEYBOARD,OPTICAL_K87)   ,   // 87键位
    OHID_KB_OPTICAL_K82                               = OHID(BOARD_SUB_KEYBOARD,OPTICAL_K82)   ,   // 82键位
    OHID_KB_OPTICAL_K80                               = OHID(BOARD_SUB_KEYBOARD,OPTICAL_K80)   ,   // 80键位
    OHID_KB_OPTICAL_K75                               = OHID(BOARD_SUB_KEYBOARD,OPTICAL_K75)   ,   // 75键位
    OHID_KB_OPTICAL_K61                               = OHID(BOARD_SUB_KEYBOARD,OPTICAL_K61)   ,   // 61键位
    OHID_KB_MACHINE_K104                              = OHID(BOARD_SUB_KEYBOARD,MACHINE_K104)  ,   // 104键位(默认)
    OHID_KB_MACHINE_K99                               = OHID(BOARD_SUB_KEYBOARD,MACHINE_K99)   ,   // 99键位
    OHID_KB_MACHINE_K98                               = OHID(BOARD_SUB_KEYBOARD,MACHINE_K98)   ,   // 98键位
    OHID_KB_MACHINE_K87                               = OHID(BOARD_SUB_KEYBOARD,MACHINE_K87)   ,   // 87键位
    OHID_KB_MACHINE_K82                               = OHID(BOARD_SUB_KEYBOARD,MACHINE_K82)   ,   // 82键位
    OHID_KB_MACHINE_K80                               = OHID(BOARD_SUB_KEYBOARD,MACHINE_K80)   ,   // 80键位
    OHID_KB_MACHINE_K75                               = OHID(BOARD_SUB_KEYBOARD,MACHINE_K75)   ,   // 75键位
    OHID_KB_MACHINE_K61                               = OHID(BOARD_SUB_KEYBOARD,MACHINE_K61)   ,   // 61键位
#endif
};
/*************************** Alias ********************************/
#define       OHID_KB_MAGNETIC_K108                  0x0000A800U    // 108键位
#define       OHID_KB_MAGNETIC_K104                  0x0000A400U    // 104键位(默认)
#define       OHID_KB_MAGNETIC_K99                   0x00009900U    // 99键位
#define       OHID_KB_MAGNETIC_K98                   0x00009800U    // 98键位
#define       OHID_KB_MAGNETIC_K87                   0x00008700U    // 87键位
#define       OHID_KB_MAGNETIC_K82                   0x00008200U    // 82键位
#define       OHID_KB_MAGNETIC_K80                   0x00008000U    // 80键位
#define       OHID_KB_MAGNETIC_K75                   0x00007500U    // 75键位
#define       OHID_KB_MAGNETIC_K68                   0x00006800U    // 68键位
#define       OHID_KB_MAGNETIC_K64                   0x00006400U    // 64键位
#define       OHID_KB_MAGNETIC_K61                   0x00006100U    // 61键位
#define       OHID_KB_MAGNETIC_K16                   0x00001600U    // 16键位
#define       OHID_KB_OPTICAL_K104                   0x0000A410U    // 104键位(默认)
#define       OHID_KB_OPTICAL_K99                    0x00009910U    // 99键位
#define       OHID_KB_OPTICAL_K98                    0x00009810U    // 98键位
#define       OHID_KB_OPTICAL_K87                    0x00008710U    // 87键位
#define       OHID_KB_OPTICAL_K82                    0x00008210U    // 82键位
#define       OHID_KB_OPTICAL_K80                    0x00008010U    // 80键位
#define       OHID_KB_OPTICAL_K75                    0x00007510U    // 75键位
#define       OHID_KB_OPTICAL_K64                    0x00006410U    // 64键位
#define       OHID_KB_OPTICAL_K61                    0x00006110U    // 61键位
#define       OHID_KB_MACHINE_K104                   0x0000A420U    // 104键位(默认)
#define       OHID_KB_MACHINE_K99                    0x00009920U    // 99键位
#define       OHID_KB_MACHINE_K98                    0x00009820U    // 98键位
#define       OHID_KB_MACHINE_K87                    0x00008720U    // 87键位
#define       OHID_KB_MACHINE_K82                    0x00008220U    // 82键位
#define       OHID_KB_MACHINE_K80                    0x00008020U    // 80键位
#define       OHID_KB_MACHINE_K75                    0x00007520U    // 75键位
#define       OHID_KB_MACHINE_K64                    0x00006420U    // 64键位
#define       OHID_KB_MACHINE_K61                    0x00006120U    // 61键位

/******************************************************************************
* 嵌套引用,防止错误地引用头文件, <OHID.h> 与 <OHID/OHID_Board.h> 必须同时被引用,
* 且必须先引用 <OHID.h> 后引用 <OHID/OHID_Board.h>
*******************************************************************************/
#ifndef _OHID_H_
#include <OHID.h>
#endif
/******************************************************************************
* Board ID 定义
*******************************************************************************/
#ifndef   OHID_BOARDS_ID_LIST_EXP
enum OHID_BOARDS_ID_LIST {
    OHID_BOARD_UNDEFINED              = OHID_BOARD_ID(OHID_DEVICE_UNDEFINED, 0x00,                   0x00),    // 未定义设备, 用于临时使用和扩展
    OHID_BOARD_MACHINE                = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MACHINE_K104,   0x00),    // 104 键机械轴
    OHID_BOARD_OPTICAL                = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_OPTICAL_K104,   0x00),    // 104 键光轴
    OHID_BOARD_MAGNETIC               = OHID_BOARD_ID(OHID_DEVICE_KEYBOARD,  OHID_KB_MAGNETIC_K104,  0x00),    // 104 键磁轴
};
#endif  // OHID_BOARD_ID_EXP

#ifdef __cplusplus
}
#endif

#endif // _OHID_BOARD_H_

/************************** (C) COPYRIGHT Merafour **************************/
