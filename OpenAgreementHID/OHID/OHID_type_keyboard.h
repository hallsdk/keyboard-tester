/******************** (C) COPYRIGHT 2018 merafour ********************
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.0.0
* Date               : 01/01/2024
* Description        : OpenAgreementHID.
* Reference          : px4 bootloader
********************************************************************************
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#ifndef _OHID_TYPE_KEYBOARD_H_
#define _OHID_TYPE_KEYBOARD_H_

#include <stdint.h>
#include <string.h>
#include <OHID/OHID_Port.h>
#include "OHID/OHID_KeyBoard.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OHID
#define OHID(prefix,field)          OHID_##prefix##_##field//
#endif
//#define OHID_PREFIX(prefix,field)        prefix##field
//#define OHID_PREFIX3(ohid,prefix,field)  ohid##prefix##_##field//
//#define OHID(prefix,field)               OHID_##prefix##_##field//
//#define OHID_MODE(field)                 (field)

/******************************************************************************
 * 数据转换宏定义
 * OHID_mm(
 * OHID_mm_cnv(
*******************************************************************************/
#define   OHID_MM_MAX              (200U)                                           // (40*5)/(0.02mm)
#define   OHID_MM40_MAX            (200U)                                           // (40*5)/(0.02mm)
#define   OHID_MM38_MAX            (190U)                                           // (40*5)/(0.02mm)
#define   OHID_MM36_MAX            (180U)                                           // (36*5)/(0.02mm)
#define   OHID_MM35_MAX            (175U)                                           // (35*5)/(0.02mm)
#define   OHID_MM_MIN              (100U)                                           // (20*5)/(0.02mm)
#define     OHID_mm(mm)              (mm*5)                                           // adc[0-255]  / (0.02mm)
#define   OHID_mm_cnv(mm)          (mm/5)                                           // adc[0-255]  / (0.02mm)
#define   OHID_cnv12_adc(mm)       (((mm)*4095U+ (OHID_MM_MAX-1))/OHID_MM_MAX)      // mm [0-200]  / (0.02mm), 4.0mm
#define   OHID_cnv12HH(mm,max)     (((mm)*4095U+ ((max)-1))/(max))      // mm [0-200]  / (0.02mm)
#define   OHID_cnv12_mm(adc)       (((adc)*OHID_MM_MAX+2047)/4095U)                 // adc[0-4095] / (1/4095)
//#define   OHID_cnv10_adc(mm)       (((mm)*1023U+ (OHID_MM_MAX-1))/OHID_MM_MAX)      // mm [0-200]  / (0.02mm)
//#define   OHID_cnv10_mm(adc)       (((adc)*OHID_MM_MAX+ 511)/1023U)                 // adc[0-1023] / (1/1023)
//#define   OHID_cnv8_adc(mm)        (((mm)*255U+ (OHID_MM_MAX-1))/OHID_MM_MAX)       // mm [0-200]  / (0.02mm)
//#define   OHID_cnv8_mm(adc)        (((adc)*OHID_MM_MAX+ 127)/255U)                  // adc[0-255]  / (1/255)
/******************************************************************************
 * 常量(枚举)类型定义
*******************************************************************************/

// 按键工作模式定义
enum OHID_TKB_TRPS {
// 参考 Wooting和海盗船
//                                         S    //T1[RL] T2[RL] T3[RL] // S状态码, R当前状态, L上一刻状态
    OHID(TRPS,NULL)                     = 0x0,  //00     00     00     // 完全释放
    OHID(TRPS,TRIG1)                    = 0x1,  //10     00     00     // TrPs(触发点)1按下
    OHID(TRPS,HOLD1A)                   = 0x2,  //11     00     00     // TrPs(触发点)1按下保持 (TrPs1按下)
    OHID(TRPS,TRIG2)                    = 0x3,  //11     10     00     // TrPs(触发点)2按下     (TrPs1按下)
    OHID(TRPS,HOLD2)                    = 0x4,  //11     11     00     // TrPs(触发点)2按下保持 (TrPs1按下)
    OHID(TRPS,TRIG3)                    = 0x5,  //11     11     1x     // TrPs(触发点)3按下     (TrPs1按下,TrPs2按下)
    OHID(TRPS,FREE2)                    = 0x6,  //11     01     00     // TrPs(触发点)2释放     (TrPs1按下)
    OHID(TRPS,HOLD1B)                   = 0x7,  //11     00     00     // TrPs(触发点)1抬起保持 (TrPs1按下)
    OHID(TRPS,FREE1)                    = 0x8,  //01     00     00     // TrPs(触发点)1释放
	/*************************** Alias ********************************/
    TRPS_NULL                           = OHID(TRPS,NULL)           ,  //00     00     00     // 完全释放
    TRPS_TRIG1                          = OHID(TRPS,TRIG1)          ,  //10     00     00     // TrPs(触发点)1按下
    TRPS_HOLD1A                         = OHID(TRPS,HOLD1A)         ,  //11     00     00     // TrPs(触发点)1按下保持 (TrPs1按下)
    TRPS_TRIG2                          = OHID(TRPS,TRIG2)          ,  //11     10     00     // TrPs(触发点)2按下     (TrPs1按下)
    TRPS_HOLD2                          = OHID(TRPS,HOLD2)          ,  //11     11     00     // TrPs(触发点)2按下保持 (TrPs1按下)
    TRPS_TRIG3                          = OHID(TRPS,TRIG3)          ,  //11     11     1x     // TrPs(触发点)3按下     (TrPs1按下,TrPs2按下)
    TRPS_FREE2                          = OHID(TRPS,FREE2)          ,  //11     01     00     // TrPs(触发点)2释放     (TrPs1按下)
    TRPS_HOLD1B                         = OHID(TRPS,HOLD1B)         ,  //11     00     00     // TrPs(触发点)1抬起保持 (TrPs1按下)
    TRPS_FREE1                          = OHID(TRPS,FREE1)          ,  //01     00     00     // TrPs(触发点)1释放
};
//--
#define   OHID_GET_TRPS(status)                 (status&0xF)
///******************************************************************************
//** 宏指令由 3部分构成, 分别是: 按键, 延时, 事件(按下/抬起)
//** 按键: 16bit 编码
//** 延时: 12bit 编码, 最小值1(0+1) 最大值 1024(1023+1),单位 10ms
//** 事件: 4bit  编码, 取 enum OHID_TKB_TRPS 中定义的值，但实际只使用 TRPS_TRIG1(按下) 和 TRPS_FREE1(抬起) 两个值
//** 使用键值 KC_NO 或者事件 TRPS_NULL 都将认为宏序列结束
//*******************************************************************************/
//#define   OHID_MACRO_CMD(key,Event,delay)        ( (((Event)<<28)&0xF0000000) | ((((delay/10))<<16)&0x0FFF0000) | ((key)&0xFFFF) )
//#define   OHID_MACRO_EVENT(cmd)                  ((cmd>>28)&0x0F)
//#define   OHID_MACRO_KEY(cmd)                    (cmd&0xFFFF)
//#define   OHID_MACRO_DELAY(cmd)                  (((cmd>>16)&0x0FFF)*10)

/******************************************************************************
 * 数据结构定义
*******************************************************************************/

/***********************************************************************************************
* Bitmap[1B] : | hid[0-2] | sleep[3] | mode[4-7] |
***********************************************************************************************/
#define ohid_bitmap(hid,mode,sleep)           ( ((hid)&0x07) | (((sleep)<<3)&0x08) | (((mode)<<4)&0xF0) )
#define ohid_sleep_get(Bitmap)                ( (Bitmap>>3)&0x01)
#define ohid_mode_get(Bitmap)                 ( (Bitmap>>4)&0x0F)
#define ohid_hid_get(Bitmap)                  ( Bitmap&0x07 )

enum ohid_host_sys_t {
	OHID(HOST_SYS,MASK)        = 0x01,
	OHID(HOST_SYS,HAND)        = 0x02,
    OHID(HOST_SYS,WIN)         = 0x00,
    OHID(HOST_SYS,MAC)         = 0x01,
    OHID(HOST_SYS_HAND,WIN)    = 0x00 | OHID(HOST_SYS,HAND),
    OHID(HOST_SYS_HAND,MAC)    = 0x01 | OHID(HOST_SYS,HAND),
	HOST_MASK                  = OHID(HOST_SYS,MASK),
	HOST_HAND                  = OHID(HOST_SYS,HAND),
	HOST_WIN                   = OHID(HOST_SYS,WIN),
    HOST_MAC                   = OHID(HOST_SYS,MAC),
	HOST_HWIN                  = OHID(HOST_SYS_HAND,WIN),
    HOST_HMAC                  = OHID(HOST_SYS_HAND,MAC),
};
enum ohid_rgb_bright_t {
	OHID(RGB_BRIGHT,MASK)      = 0x07,   // mask
	OHID(RGB_BRIGHT,UPDATE)    = 0x80,   // update flag
    OHID(RGB_BRIGHT,GRADE0)    = 0x00,   // grade0
    OHID(RGB_BRIGHT,GRADE1)    = 0x01,   // grade1
	OHID(RGB_BRIGHT,GRADE2)    = 0x02,   // grade2
	OHID(RGB_BRIGHT,GRADE3)    = 0x03,   // grade3
	OHID(RGB_BRIGHT,GRADE4)    = 0x04,   // grade4
	OHID(RGB_BRIGHT,MAX)       = 0x04,   // grade max
	/*************************** Alias ********************************/
    RGB_MASK2                  = OHID(RGB_BRIGHT,MASK),
	RGB_UPDATE                 = OHID(RGB_BRIGHT,UPDATE),
	RGB_GRADE0                 = OHID(RGB_BRIGHT,GRADE0),
    RGB_GRADE1                 = OHID(RGB_BRIGHT,GRADE1),
	RGB_GRADE2                 = OHID(RGB_BRIGHT,GRADE2),
	RGB_GRADE3                 = OHID(RGB_BRIGHT,GRADE3),
	RGB_GRADE4                 = OHID(RGB_BRIGHT,GRADE4),
	RGB_MAX                    = OHID(RGB_BRIGHT,MAX),
};

enum ohid_rgb_color_t {
	OHID(RGB_COLOR,NONE)      = 0xFF,   // None
	OHID(RGB_COLOR,MASK)      = 0x07,   // mask
	OHID(RGB_COLOR,RED)       = 0x00,   // 红
    OHID(RGB_COLOR,GREEN)     = 0x01,   // 绿
    OHID(RGB_COLOR,YELLOW)    = 0x02,   // 黄
	OHID(RGB_COLOR,BLUE)      = 0x03,   // 蓝
	OHID(RGB_COLOR,PURPLE)    = 0x04,   // 紫
	OHID(RGB_COLOR,CYAN)      = 0x05,   // 青
	OHID(RGB_COLOR,WHITE)     = 0x06,   // 白
    OHID(RGB_COLOR,RAIN)      = 0x07,   // 多彩
	OHID(RGB_COLOR,MAX)       = 0x07,   // max
	/*************************** Alias ********************************/
	RGBC_NONE                 = OHID(RGB_COLOR,NONE),
	RGBC_MASK                 = OHID(RGB_COLOR,MASK),
	RGBC_RED                  = OHID(RGB_COLOR,RED),
	RGBC_GREEN                = OHID(RGB_COLOR,GREEN),
    RGBC_YELLOW               = OHID(RGB_COLOR,YELLOW),
	RGBC_BLUE                 = OHID(RGB_COLOR,BLUE),
	RGBC_PURPLE               = OHID(RGB_COLOR,PURPLE),
	RGBC_CYAN                 = OHID(RGB_COLOR,CYAN),
	RGBC_WHITE                = OHID(RGB_COLOR,WHITE),
	RGBC_RAIN                 = OHID(RGB_COLOR,RAIN),
	RGBC_MAX                  = OHID(RGB_COLOR,MAX),
};

enum ohid_bitmap_t {
    OHID(BITMAP,NONE)                 = 0x00,       //
    OHID(BITMAP,RGB_ON)               = (0x01<<0),  // RGB ON/OFF
    OHID(BITMAP,REVERSE)              = (0x01<<1),  // reverse(反向), 用于控制动态灯光效果的反向
    OHID(BITMAP,QUICK)                = (0x01<<2),  // 快速模式,开启将关闭 RGB 
	OHID(BITMAP,RGB_SLEEP)            = (0x01<<3),  // RGB Sleep
	OHID(BITMAP,BASE)                 = (0x01<<4),  // Base Mode
	OHID(BITMAP,LOCK_WIN)             = (0x01<<5),  // Lock Win
	//OHID(BITMAP,ADJ_AUTO)             = (0x01<<6),  // 智能校准
	// 可读可写, 智能校准功能默认打开,关闭时将一直使用手动校准的数据,使用过程中不会实时更新
	OHID(BITMAP,ADJ_STATIC)           = (0x01<<6),  // 使用静态校准
	/*************************** Alias ********************************/
    OHID_BIT_NONE                     = OHID(BITMAP,NONE),
    OHID_BIT_RGB_ON                   = OHID(BITMAP,RGB_ON),
    OHID_BIT_REVERSE                  = OHID(BITMAP,REVERSE),
    OHID_BIT_QUICK                    = OHID(BITMAP,QUICK),
	OHID_BIT_RGB_SLEEP                = OHID(BITMAP,RGB_SLEEP) ,
	OHID_BIT_BASE                     = OHID(BITMAP,BASE) ,
	OHID_BIT_LOCK_WIN                 = OHID(BITMAP,LOCK_WIN),  // Lock Win
	OHID_BIT_ADJ_STATIC               = OHID(BITMAP,ADJ_STATIC),  // 使用静态校准
};
#define OHID_BITMAP_GET(Flag,bitmap)                 ((bitmap) & (uint32_t)(Flag))
#define OHID_BITMAP_SET(Flag,bitmap)                 ((bitmap) |= (uint32_t)(Flag))
#define OHID_BITMAP_CLR(Flag,bitmap)                 ((bitmap) &= (~(uint32_t)(Flag)))

#ifdef __cplusplus
}
#endif

#endif // _OHID_TYPE_KEYBOARD_H_
