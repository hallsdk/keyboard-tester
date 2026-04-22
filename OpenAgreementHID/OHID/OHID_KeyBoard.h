/******************** (C) COPYRIGHT 2018 merafour ********************
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.0.0
* Date               : 01/01/2024
* Description        : OpenAgreementHID.
* Description        : 开源键盘通信协议 API 接口.
* Description        : 这些接口对数据包进行封装,可以让对协议的操作更简单
* Description        : 注：
* Description        : OHIDM_* : 主机(电脑上位机)发送给从机的数据包
* Description        : OHIDS_* : 从机(键盘)发送给主机的数据包
********************************************************************************
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#ifndef _OHID_KEYBOARD_H_
#define _OHID_KEYBOARD_H_

#include <stdint.h>
#include <OHID/OHID_Port.h>
#include "OHID.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OHID_PREFIX(prefix,field)        prefix##field
#define OHID_PREFIX3(ohid,prefix,field)  ohid##prefix##_##field//
//#define OHID(prefix,field)               OHID_##prefix##_##field//
#define OHID_MODE(field)                 (field)

/******************************************************************************
* MIX, 用于支持一些杂项, 为 BASE_MIX 命令的子命令
*******************************************************************************/
enum OHID_MIX_ORDER {
    OHID(MIX_ORDER,USB_DEBUG)               = 0xFE,   //  arg1(uint8_t) = 开启/关闭 USB调试;
    OHID(MIX_ORDER,RATE_TEST)               = 0xFD,   //  arg1(uint8_t) = 开启/关闭 回报率测试功能, arg2(uint16_t) = 测试时长, 0表示一直发, 单位 ms;
    OHID(MIX_ORDER,RGB_TEST)                = 0xFC,   //  arg1(uint8_t) = 1开启/0关闭 RGB测试功能(需先切换到直驱模式), arg2(uint32_t) = 测试的 RGB 颜色(高8 位固定位 0xFF);
    OHID(MIX_ORDER,KEY_TEST)                = 0xFB,   //  arg1(uint8_t) = 1开启/0关闭 按键测试功能, arg2(uint16_t) = 按键按下时延迟发送时间(10ms), arg3(uint8_t) = 按键发送间隔(ms);
    OHID(MIX_ORDER,ADJ_STATUS)              = 0xFA,   //  only read, 获取按键的校准状态, 返回值为 Bottom(6*uint32_t)/按下 和 Top(6*uint32_t)值/按下之后松开, 均为 bitmap 数据;
    OHID(MIX_ORDER,QUERY_TICK)              = 0xF9,   //  arg1(uint32_t)= 系统时间(ms), 用于获取键盘的系统计时器;
    OHID(MIX_ORDER,SLAVE_STOP)              = 0xF8,   //  arg1(uint8_t) = 停止与从机的通讯, 在用 MCU 做数据转发的时候会用到;
    OHID(MIX_ORDER,SLAVE_TRUNING)           = 0xF7,   //  arg1(uint8_t) = 通过从机调参, 在用 2.4G 对键盘进行调参的时候会用到;
    OHID(MIX_ORDER,ADJ_DEAD_ZONE)           = 0xF6,   //  arg1(uint8_t) = 死区,单位同全局触发 0.02mm ;
    // <Data> : 无参数.
    OHID(MIX_ORDER,MANUFACTURER)            = 0x00,   // only read, Get Manufacturer
    // <Data> : | Date | 编译信息.
    OHID(MIX_ORDER,BUILD)                   = 0x01,   // only write, 获取固件编译信息
    OHID(MIX_ORDER,SAVE)                    = 0x02,   // 手动保存参数,only write
    OHID(MIX_ORDER,RELOAD)                  = 0x03,   // 重新加载参数,only write
    OHID(MIX_ORDER,RESET)                   = 0x04,   // 参数重置,only write
    // <Data> : NONE(默认不带参数),恢复所有参数; [0-3],恢复对应参数组, 0xFF(255),恢复所有参数; [4-254],保留(使用保留参数将恢复所有参数).
    OHID(MIX_ORDER,FACTORY)                 = 0x05,   // 恢复出厂设置,等待响应超时应 > 200ms
    // <Data> : | GRAY1 | GRAY2 | GRAY3 | GRAY4 |, GRAYx 为 RGB 灰度, 0xFF 无效.
    OHID(MIX_ORDER,GRAY)                    = 0x06,   //  write/read, 写入 RGB 灰度 , arg = 0xFF 为无效
    // <Data> : | 0x01 | 0x00 | 0x05 |. 3Byte (V1.0.05)
    OHID(MIX_ORDER,QUERY_VER_HEX)           = 0x07,   // only read, Get Firmware Version(V1.0.05): 0x01,0x00,0x05
    // <Data> : | "V1.0.05" |, .
    OHID(MIX_ORDER,QUERY_VER_STR)           = 0x08,   // only read, Get Firmware Version(V1.0.05): "V1.0.05"
    // <Data> : | "0123456789ABCDEF" |, GRAYx 为 RGB 灰度, 0xFF 无效.
    OHID(MIX_ORDER,QUERY_SN)                = 0x09,   // only read, Get Serial number(通过芯片 ID计算得到的 16Byte 序列号,eg:"0123456789ABCDEF")
    // <Data> : | SN.0 | SN.1 | ... | SN.15 |, SN 为厂商自定义序列号,长度为 16byte.
    OHID(MIX_ORDER,PRODUCT_SN)              = 0x0A,   // write/read, 厂商义序列号(16byte)
    // <Data> : | <arg> |.
    OHID(MIX_ORDER,QUICK)                   = 0x10,   // write/read, 快速模式
    OHID(MIX_ORDER,EP_TICKS)                = 0x11,   // write/read, 回报率
    OHID(MIX_ORDER,RGB_ON)                  = 0x12,   // write/read, RGB开关
    OHID(MIX_ORDER,RGB_BACK)                = 0x13,   // write/read, 背光
    OHID(MIX_ORDER,RGB_COLOR)               = 0x14,   // write/read, 调色板
    OHID(MIX_ORDER,RGB_REVERSE)             = 0x15,   // write/read, 反向
    OHID(MIX_ORDER,RGB_SUB_GRAY)            = 0x16,   // arg(uint8_t) = 侧灯亮度, 0-100;
    OHID(MIX_ORDER,RGB_SUB_MODE)            = 0x17,   // arg(uint8_t) = 侧灯模式, 0:OFF, 1:ON, 2:呼吸;
    OHID(MIX_ORDER,RGB_SUB)                 = 0x18,   // arg1(uint8_t) = 侧灯亮度, 0-100;, arg2(uint8_t) = 侧灯模式, 0:OFF, 1:ON, 2:呼吸;
    OHID(MIX_ORDER,RGB_IDX)                 = 0x19,   // arg1(uint8_t) = 调色板索引[0-7];
    OHID(MIX_ORDER,RGB_MODE)                = 0x1A,   // arg1(uint8_t) = 主键区灯光模式;
    // 生产线(Production line)模式,仅用于生产测试,将关闭键盘功能,并进入特定灯光模式
    // arg: 1 开启, 0关闭
    OHID(MIX_ORDER,PROD_LINE)               = 0x1B,
    // 快速扫描模式, arg: 1 开启, 0关闭
    OHID(MIX_ORDER,QUICK_SCAN)              = 0x1C,
    // 开启或关闭自适应模式(死区,dead_zone), arg: 1 开启, 0关闭
    OHID(MIX_ORDER,DEAD_ZONE)               = 0x1D,
    // query/switch 指令返回 arg参数为 1表示查询为真,即所查询的内容被确认为真,其它值为否,0xFF忽略
    // query/switch 写操作为切换到指定的状态
    OHID(MIX_ORDER,QS_LOOKWIN)              = 0x20,   // query/switch, 查询/切换到 锁WIN键
    OHID(MIX_ORDER,QS_WIN)                  = 0x21,   // query/switch, 查询/切换到 是否为 Win 模式
    OHID(MIX_ORDER,QS_MAC)                  = 0x22,   // query/switch, 查询/切换到 是否为 Mac 模式
    OHID(MIX_ORDER,QS_BASE)                 = 0x23,   // query/switch, 查询/切换到 是否为 标准 模式
    OHID(MIX_ORDER,QS_LINEAR)               = 0x24,   // query/switch, 查询/切换到 是否为 可调行程 模式
    // query, arg1(uint8_t) = 工作模式, 由 enum ohid_kb_mode_t 定义, 只读, 即不可由驱动切换 USB, RF, BLE 模式
    OHID(MIX_ORDER,QS_RF)                   = 0x25,   // query/switch, 查询键盘的 USB, 2.4G, BLE 工作模式
    // 查询波动开关档位
    OHID(MIX_ORDER,QS_SW)                   = 0x26,   // query, arg1(uint8_t) = 拨动开关档位
    // reserved
    OHID(MIX_ORDER,AUTO_RP_HH)              = 0x30,   // arg1(uint8_t) = 开启/关闭上报, arg2(uint8_t) = 上报延时;
    OHID(MIX_ORDER,TRIGGER)                 = 0x31,   // arg1(uint8_t) = MM[1B], arg2(uint8_t) = Reset[1B];
    OHID(MIX_ORDER,TABLE)                   = 0x32,   // arg1(uint8_t) = Group[1,2,3,4], 切换参数组;
    // 可读可写, 智能校准功能默认打开,关闭时将一直使用手动校准的数据,使用过程中不会实时更新
    OHID(MIX_ORDER,ADJ_AUTO)                = 0x33,   // arg1(uint8_t) = 0/1, 关闭/开启智能校准;
    OHID(MIX_ORDER,ADJ_HAND)                = 0x34,   // arg1(uint8_t) = 0/1, 结束(保存)/开启/取消 手动校准, ;
    // reserved
    /*************************** Alias ********************************/
    OHID_MIX_USB_DEBUG               = OHID(MIX_ORDER,USB_DEBUG)     ,   //  arg1(uint8_t) = 开启/关闭 USB调试;
    OHID_MIX_RATE_TEST               = OHID(MIX_ORDER,RATE_TEST)     ,   //  arg1(uint8_t) = 开启/关闭 , arg2(uint8_t) = 发送间隔的 tick 数, 8K模式下 1 表示 125us;
    OHID_MIX_RGB_TEST                = OHID(MIX_ORDER,RGB_TEST)      ,   //  arg1(uint8_t) = 1开启/0关闭 RGB测试功能(需先切换到直驱模式), arg2(uint32_t) = 测试的 RGB 颜色(高8 位固定位 0xFF);
    OHID_MIX_KEY_TEST                = OHID(MIX_ORDER,KEY_TEST)      ,   //  arg1(uint8_t) = 1开启/0关闭 按键测试功能, arg2(uint16_t) = 按键按下时延迟发送时间(ms), arg3(uint8_t) = 按键发送间隔(ms);
    OHID_MIX_ADJ_STATUS              = OHID(MIX_ORDER,ADJ_STATUS)    ,   //  only read, 获取按键的校准状态, 返回值为 Bottom(6*uint32_t)/按下 和 Top(6*uint32_t)值/按下之后松开, 均为 bitmap 数据;
    OHID_MIX_QUERY_TICK              = OHID(MIX_ORDER,QUERY_TICK)    ,   //  arg1(uint32_t)= 系统时间(ms), 用于获取键盘的系统计时器;
    OHID_MIX_SLAVE_STOP              = OHID(MIX_ORDER,SLAVE_STOP)    ,   //  arg1(uint8_t) = 停止与从机的通讯, 在用 MCU 做数据转发的时候会用到;
    OHID_MIX_SLAVE_TRUNING           = OHID(MIX_ORDER,SLAVE_TRUNING) ,   //  arg1(uint8_t) = 通过从机调参, 在用 2.4G 对键盘进行调参的时候会用到;
    OHID_MIX_ADJ_DEAD_ZONE           = OHID(MIX_ORDER,ADJ_DEAD_ZONE) ,   //  arg1(uint8_t) = 死区,单位同全局触发 0.02mm ;
    OHID_MIX_MANUFACTURER            = OHID(MIX_ORDER,MANUFACTURER)  ,   // only read, Get Manufacturer
    OHID_MIX_BUILD                   = OHID(MIX_ORDER,BUILD)         ,   // only write, 获取固件编译信息
    OHID_MIX_SAVE                    = OHID(MIX_ORDER,SAVE)          ,   // 手动保存参数,only write
    OHID_MIX_RELOAD                  = OHID(MIX_ORDER,RELOAD)        ,   // 重新加载参数,only write
    OHID_MIX_RESET                   = OHID(MIX_ORDER,RESET)         ,   // 参数重置,only write
    OHID_MIX_FACTORY                 = OHID(MIX_ORDER,FACTORY)       ,   // 恢复出厂设置,等待响应超时应 > 200ms
    OHID_MIX_GRAY                    = OHID(MIX_ORDER,GRAY)          ,   //  write/read, 写入 RGB 灰度 , arg = 0xFF 为无效
    OHID_MIX_VER_HEX                 = OHID(MIX_ORDER,QUERY_VER_HEX) ,   //  only read, Get Firmware Version(V1.0.05): 0x01,0x00,0x05
    OHID_MIX_VER_STR                 = OHID(MIX_ORDER,QUERY_VER_STR) ,   //  only read, Get Firmware Version(V1.0.05): "V1.0.05"
    OHID_MIX_SN                      = OHID(MIX_ORDER,QUERY_SN)      ,   //  only read, Get Serial number(通过芯片 ID计算得到的 16Byte 序列号,eg:"0123456789ABCDEF")
    OHID_MIX_PRODUCT_SN              = OHID(MIX_ORDER,PRODUCT_SN)    ,   //  write/read, 厂商义序列号(16byte)
    OHID_MIX_QUICK                   = OHID(MIX_ORDER,QUICK)         ,   // write/read, 快速模式
    OHID_MIX_EP_TICKS                = OHID(MIX_ORDER,EP_TICKS)      ,   // write/read, arg(uint8_t) = 回报率, 0:默认, 1:8K, 2:4K, 3:2K, 4:1K, 5:500Hz, 6:250Hz, 7:125Hz,
    OHID_MIX_RGB_ON                  = OHID(MIX_ORDER,RGB_ON)        ,   // write/read, RGB开关
    OHID_MIX_RGB_BACK                = OHID(MIX_ORDER,RGB_BACK)      ,   // write/read, 背光
    OHID_MIX_RGB_COLOR               = OHID(MIX_ORDER,RGB_COLOR)     ,   // write/read, 调色板
    OHID_MIX_RGB_REVERSE             = OHID(MIX_ORDER,RGB_REVERSE)   ,   // write/read, 反向
    OHID_MIX_RGB_SUB_GRAY            = OHID(MIX_ORDER,RGB_SUB_GRAY)  ,   // arg(uint8_t) = 侧灯亮度, 0-100;
    OHID_MIX_RGB_SUB_MODE            = OHID(MIX_ORDER,RGB_SUB_MODE)  ,   // arg(uint8_t) = 侧灯模式, 0:OFF, 1:ON, 2:呼吸;
    OHID_MIX_PROD_LINE               = OHID(MIX_ORDER,PROD_LINE)     ,
    OHID_MIX_QUICK_SCAN              = OHID(MIX_ORDER,QUICK_SCAN)    ,
    OHID_MIX_DEAD_ZONE               = OHID(MIX_ORDER,DEAD_ZONE)     ,   // 开启或关闭自适应模式(死区,dead_zone), arg: 1 开启, 0关闭
    OHID_MIX_RGB_SUB                 = OHID(MIX_ORDER,RGB_SUB)       ,   // arg1(uint8_t) = 侧灯亮度, 0-100;, arg2(uint8_t) = 侧灯模式, 0:OFF, 1:ON, 2:呼吸;
    OHID_MIX_RGB_IDX                 = OHID(MIX_ORDER,RGB_IDX)       ,   // arg1(uint8_t) = 调色板索引[0-7];
    OHID_MIX_RGB_MODE                = OHID(MIX_ORDER,RGB_MODE)      ,   // arg1(uint8_t) = 主键区灯光模式;
    OHID_MIX_QS_LOOKWIN              = OHID(MIX_ORDER,QS_LOOKWIN)    ,   // query/switch, 查询/切换到 锁WIN键
    OHID_MIX_QS_WIN                  = OHID(MIX_ORDER,QS_WIN)        ,   // query/switch, 查询/切换到 是否为 Win 模式
    OHID_MIX_QS_MAC                  = OHID(MIX_ORDER,QS_MAC)        ,   // query/switch, 查询/切换到 是否为 Mac 模式
    OHID_MIX_QS_BASE                 = OHID(MIX_ORDER,QS_BASE)       ,   // query/switch, 查询/切换到 是否为 标准 模式
    OHID_MIX_QS_LINEAR               = OHID(MIX_ORDER,QS_LINEAR)     ,   // query/switch, 查询/切换到 是否为 linear 模式
    OHID_MIX_QS_SW                   = OHID(MIX_ORDER,QS_SW)         ,   // query, arg1(uint8_t) = 拨动开关档位
    OHID_MIX_AUTO_RP_HH              = OHID(MIX_ORDER,AUTO_RP_HH)    ,   // arg1(uint8_t) = 开启/关闭上报, arg2(uint8_t) = 上报延时;
    OHID_MIX_TRIGGER                 = OHID(MIX_ORDER,TRIGGER)       ,   // arg1(uint8_t) = MM[1B], arg2(uint8_t) = Reset[1B];
    OHID_MIX_TABLE                   = OHID(MIX_ORDER,TABLE)         ,   // arg1(uint8_t) = Group[1,2,3,4], 切换参数组;
    // 可读可写, 智能校准功能默认打开,关闭时将一直使用手动校准的数据,使用过程中不会实时更新
    OHID_MIX_ADJ_AUTO                = OHID(MIX_ORDER,ADJ_AUTO),   // arg1(uint8_t) = 0/1, 关闭/开启智能校准;
    OHID_MIX_ADJ_HAND                = OHID(MIX_ORDER,ADJ_HAND),   // arg1(uint8_t) = 0/1, 结束(保存)/开启/取消 手动校准, ;
};

/******************************************************************************
* 参数页, 键盘的参数比较多, 其中能够组成矩阵(如配列信息为 6*21 矩阵)的参数定义为 PAGE,
* 以下为不同 PAGE 的代号(索引号), 注:根据数据内容的不同,矩阵的基本数据类型可能不同
*******************************************************************************/
enum OHID_PARAM_PAGE {
    OHID(PARAM_PAGE,FN0)    = 0x00,          // 16bit, 按键层 0, 主层
    OHID(PARAM_PAGE,FN1)    = 0x01,          // 16bit, 按键层 1
    OHID(PARAM_PAGE,FN2)    = 0x02,          // 16bit, 按键层 2
    OHID(PARAM_PAGE,FN3)    = 0x03,          // 16bit, 按键层 3
    OHID(PARAM_PAGE,MODE)   = 0x04,          // 16bit, 按键工作模式
    OHID(PARAM_PAGE,MM)     = 0x05,          //  8bit, 按键触发行程, 主触发行程
    OHID(PARAM_PAGE,MM1)    = 0x06,          //  8bit, 按键触发行程1
    OHID(PARAM_PAGE,MM2)    = 0x07,          //  8bit, 按键触发行程2
    OHID(PARAM_PAGE,MM3)    = 0x08,          //  8bit, 按键触发行程3
    OHID(PARAM_PAGE,KEY1)   = 0x09,          // 16bit, 按键1
    OHID(PARAM_PAGE,KEY2)   = 0x0A,          // 16bit, 按键2
    OHID(PARAM_PAGE,KEY3)   = 0x0B,          // 16bit, 按键3
    OHID(PARAM_PAGE,KEY4)   = 0x0C,          // 16bit, 按键4
    OHID(PARAM_PAGE,TRPS1)  = 0x0D,          //  8bit, 触发点1
    OHID(PARAM_PAGE,TRPS2)  = 0x0E,          //  8bit, 触发点2
    OHID(PARAM_PAGE,TRPS3)  = 0x0F,          //  8bit, 触发点3
    OHID(PARAM_PAGE,TRPS4)  = 0x10,          //  8bit, 触发点4
    OHID(PARAM_PAGE,MACROA) = 0x11,          // 16bit, 宏地址
    OHID(PARAM_PAGE,MACROL) = 0x12,          //  8bit, 宏长度
    OHID(PARAM_PAGE,DELAY)  = 0x13,          // 8bit, MT/TGL 延时参数
    OHID(PARAM_PAGE,PHY)    = 0x14,          // 暂不使用
    OHID(PARAM_PAGE,MAX)    = 0x15,          // 参数个数
	/*************************** Alias ********************************/
    OHID_PAGE_FN0           = OHID(PARAM_PAGE,FN0)   ,          // /
    OHID_PAGE_FN1           = OHID(PARAM_PAGE,FN1)   ,          // /
    OHID_PAGE_FN2           = OHID(PARAM_PAGE,FN2)   ,          // /
    OHID_PAGE_FN3           = OHID(PARAM_PAGE,FN3)   ,          // /
    OHID_PAGE_MODE          = OHID(PARAM_PAGE,MODE)  ,          // /
    OHID_PAGE_MM            = OHID(PARAM_PAGE,MM)    ,          // /
    OHID_PAGE_MM1           = OHID(PARAM_PAGE,MM1)   ,          // /
    OHID_PAGE_MM2           = OHID(PARAM_PAGE,MM2)   ,          // /
    OHID_PAGE_MM3           = OHID(PARAM_PAGE,MM3)   ,          // /
    OHID_PAGE_KEY1          = OHID(PARAM_PAGE,KEY1)  ,          // /
    OHID_PAGE_KEY2          = OHID(PARAM_PAGE,KEY2)  ,          // /
    OHID_PAGE_KEY3          = OHID(PARAM_PAGE,KEY3)  ,          // /
    OHID_PAGE_KEY4          = OHID(PARAM_PAGE,KEY4)  ,          // /
    OHID_PAGE_TRPS1         = OHID(PARAM_PAGE,TRPS1) ,          // /
    OHID_PAGE_TRPS2         = OHID(PARAM_PAGE,TRPS2) ,          // /
    OHID_PAGE_TRPS3         = OHID(PARAM_PAGE,TRPS3) ,          // /
    OHID_PAGE_TRPS4         = OHID(PARAM_PAGE,TRPS4) ,          // /
    OHID_PAGE_MACROA        = OHID(PARAM_PAGE,MACROA),          // /
    OHID_PAGE_MACROL        = OHID(PARAM_PAGE,MACROL),          // /
    OHID_PAGE_DELAY         = OHID(PARAM_PAGE,DELAY) ,          // /
    OHID_PAGE_PHY           = OHID(PARAM_PAGE,PHY)   ,          // /
    OHID_PAGE_MAX           = OHID(PARAM_PAGE,MAX)   ,          // /
};
/******************************************************************************
* 按键工作模式 "OHID(PARAM_PAGE,MODE)" 定义
*******************************************************************************/
enum OHID_KB_MODE {
    OHID(KB_MODE,MASK)                  = 0x3F,           // mask
    // 0x0X
    OHID(KB_MODE,GMM)                   = 0x30,           // normal, 普通触发
    OHID(KB_MODE,NORMAL)                = 0x00,           // normal, 普通触发
    OHID(KB_MODE,MT)                    = 0x01,           // MT,
    OHID(KB_MODE,TGL)                   = 0x02,           // TGL,
    OHID(KB_MODE,MACRO)                 = 0x03,           // 宏
    OHID(KB_MODE,APPEND)                = 0x04,           // 追加
    OHID(KB_MODE,MOUSEL)                = 0x05,           // 鼠标左键
    OHID(KB_MODE,MOUSEM)                = 0x06,           // 鼠标右键
    OHID(KB_MODE,MOUSER)                = 0x07,           // 鼠标中键
    OHID(KB_MODE,MOUSE4)                = 0x08,           // 鼠标前进键
    OHID(KB_MODE,MOUSE5)                = 0x09,           // 鼠标后退键
	OHID(KB_MODE,ENCODER)               = 0x0A,           // 编码器
	// Quick 模式只上报最后一个按键
	OHID(KB_MODE,QUICK1)                = 0x0B,           // Rappy Snappy && Snap Tap && Quick
	OHID(KB_MODE,QUICK2)                = 0x0C,           // Rappy Snappy && Snap Tap && Quick
	OHID(KB_MODE,QUICK3)                = 0x0D,           // Rappy Snappy && Snap Tap && Quick
	OHID(KB_MODE,QUICK4)                = 0x0E,           // Rappy Snappy && Snap Tap && Quick
    // 0x1X
    OHID(KB_MODE,SINGLE)                = 0x10,           // Single, 单键触发
    OHID(KB_MODE,RT)                    = 0x11,           // RT,     快速触发
    OHID(KB_MODE,DT)                    = 0x12,           // DELta,  增程触发
	OHID(KB_MODE,RT_TRPS)               = 0x13,           // RT TRPS, RT 功能处于触发状态
    // 0x2X
    OHID(KB_MODE,DKS)                   = 0x20,           // 数字键控系统
    OHID(KB_MODE,AKS3)                  = 0x21,           // 模拟键控系统
	OHID(KB_MODE,TAP_STEPS1)            = 0x22,           // Tap Steps 1
	OHID(KB_MODE,TAP_STEPS2)            = 0x23,           // Tap Steps 2
	OHID(KB_MODE,TAP_STEPS3)            = 0x24,           // Tap Steps 3
	/*
	 * SOCD（Simultaneous Opposing Cardinal Directions）是指同时按下相反方向的按键时，
	 * 系统如何处理这种情况的功能。‌ 
	 * SOCD的定义和作用,SOCD是一种功能
	 * 用于处理用户同时按下相反方向的按键时的情况。例如，在《街头霸王》等格斗游戏中，
	 * 玩家可能会同时按下“上”和“下”键来执行特殊技能。SOCD机制会检测这些按键的输入顺序和深度，
	 * 并激活最深的按键，从而避免同时执行多个命令导致的混乱‌
	*/
	// SOCD 模式只上报按下最深的按键
	OHID(KB_MODE,SOCD1)                = 0x25,           // Rappy Snappy && Snap Tap && Quick
	OHID(KB_MODE,SOCD2)                = 0x26,           // Rappy Snappy && Snap Tap && Quick
	OHID(KB_MODE,SOCD3)                = 0x27,           // Rappy Snappy && Snap Tap && Quick
	OHID(KB_MODE,SOCD4)                = 0x28,           // Rappy Snappy && Snap Tap && Quick
    OHID(KB_MODE,ACTIVE_LONG)          = 0x80,           // active flag
	/*************************** Alias ********************************/
    KB_MASK                             = OHID(KB_MODE,MASK),
	KB_NORMAL                           = OHID(KB_MODE,NORMAL),
    KB_SINGLE                           = OHID(KB_MODE,SINGLE),
    KB_RT                               = OHID(KB_MODE,RT),
	KB_RT_TRPS                          = OHID(KB_MODE,RT_TRPS),
    KB_DT                               = OHID(KB_MODE,DT),
    KB_MT                               = OHID(KB_MODE,MT),
    KB_TGL                              = OHID(KB_MODE,TGL),
    KB_MACRO                            = OHID(KB_MODE,MACRO),
    KB_APPEND                           = OHID(KB_MODE,APPEND),
    KB_DKS                              = OHID(KB_MODE,DKS),
    KB_AKS3                             = OHID(KB_MODE,AKS3),
	KB_ENCODER                          = OHID(KB_MODE,ENCODER),
	KB_QUICK1                           = OHID(KB_MODE,QUICK1),
	KB_QUICK2                           = OHID(KB_MODE,QUICK2),
	KB_QUICK3                           = OHID(KB_MODE,QUICK3),
	KB_QUICK4                           = OHID(KB_MODE,QUICK4),
	KB_SOCD1                            = OHID(KB_MODE,SOCD1),
	KB_SOCD2                            = OHID(KB_MODE,SOCD2),
	KB_SOCD3                            = OHID(KB_MODE,SOCD3),
	KB_SOCD4                            = OHID(KB_MODE,SOCD4),
	KB_TAP_STEPS1                       = OHID(KB_MODE,TAP_STEPS1),           // Tap Steps 1
	KB_TAP_STEPS2                       = OHID(KB_MODE,TAP_STEPS2),           // Tap Steps 2
	KB_TAP_STEPS3                       = OHID(KB_MODE,TAP_STEPS3),           // Tap Steps 3
};
/******************************************************************************
* 按键模式宏处理指令
*******************************************************************************/
#define   TKB_MODE(mode)                        ((mode)&KB_MASK)//((mode)&0x3F)
//#define   OHID_MODE_IS_DKS(mode)                ((mode)&0x20)
//#define   OHID_MODE_IS_AKS(mode)                ((mode)&0x20)
// 0x0X
#define   OHID_MODE_IS_GMM(mode)                (OHID(KB_MODE,NORMAL)==(TKB_MODE(mode)&OHID(KB_MODE,GMM)))
#define   OHID_MODE_IS_NORMAL(mode)             (OHID(KB_MODE,NORMAL)==TKB_MODE(mode))
#define   OHID_MODE_IS_QUICK1(mode)             (KB_QUICK1==TKB_MODE(mode))
#define   OHID_MODE_IS_QUICK2(mode)             (KB_QUICK2==TKB_MODE(mode))
#define   OHID_MODE_IS_QUICK3(mode)             (KB_QUICK3==TKB_MODE(mode))
#define   OHID_MODE_IS_QUICK4(mode)             (KB_QUICK4==TKB_MODE(mode))
#define   OHID_MODE_IS_SOCD1(mode)              (KB_SOCD1 ==TKB_MODE(mode))
#define   OHID_MODE_IS_SOCD2(mode)              (KB_SOCD2 ==TKB_MODE(mode))
#define   OHID_MODE_IS_SOCD3(mode)              (KB_SOCD3 ==TKB_MODE(mode))
#define   OHID_MODE_IS_SOCD4(mode)              (KB_SOCD4 ==TKB_MODE(mode))
#define   OHID_MODE_IS_TAP_STEPS1(mode)         (KB_TAP_STEPS1==TKB_MODE(mode))
#define   OHID_MODE_IS_TAP_STEPS2(mode)         (KB_TAP_STEPS2==TKB_MODE(mode))
#define   OHID_MODE_IS_TAP_STEPS3(mode)         (KB_TAP_STEPS3==TKB_MODE(mode))
#define   OHID_MODE_IS_MT(mode)                 (OHID(KB_MODE,MT)==TKB_MODE(mode))
#define   OHID_MODE_IS_TGL(mode)                (OHID(KB_MODE,TGL)==TKB_MODE(mode))
#define   OHID_MODE_IS_MACRO(mode)              (OHID(KB_MODE,MACRO)==TKB_MODE(mode))
#define   OHID_MODE_IS_APPEND(mode)             (OHID(KB_MODE,APPEND)==TKB_MODE(mode))
#define   OHID_MODE_IS_MOUSEL(mode)             (OHID(KB_MODE,MOUSEL)==TKB_MODE(mode))
#define   OHID_MODE_IS_MOUSEM(mode)             (OHID(KB_MODE,MOUSEM)==TKB_MODE(mode))
#define   OHID_MODE_IS_MOUSER(mode)             (OHID(KB_MODE,MOUSER)==TKB_MODE(mode))
#define   OHID_MODE_IS_MOUSE4(mode)             (OHID(KB_MODE,MOUSE4)==TKB_MODE(mode))
#define   OHID_MODE_IS_MOUSE5(mode)             (OHID(KB_MODE,MOUSE5)==TKB_MODE(mode))
// 0x1X
#define   OHID_MODE_IS_SINGLE(mode)             (OHID(KB_MODE,SINGLE)==TKB_MODE(mode))
#define   OHID_MODE_IS_RT(mode)                 (KB_RT==TKB_MODE(mode))
#define   OHID_MODE_IS_RT_TRPS(mode)            (KB_RT_TRPS==TKB_MODE(mode))
#define   OHID_MODE_IS_DT(mode)                 (OHID(KB_MODE,DT)==TKB_MODE(mode))
// 0x2X
//#define   OHID_MODE_IS_DKS(mode)                ((mode)&0x20)
//#define   OHID_MODE_IS_AKS(mode)                ((mode)&0x20)
//#define   OHID_MODE_IS_DKS(mode)                ((mode)&KB_DKS)
//#define   OHID_MODE_IS_AKS(mode)                ((mode)&KB_AKS3)
#define   OHID_MODE_IS_DKS(mode)                (KB_DKS==TKB_MODE(mode))
#define   OHID_MODE_IS_AKS(mode)                (KB_AKS3==TKB_MODE(mode))
#define   OHID_MODE_GET_ACTIVE_LONG(mode)       ((mode)&OHID(KB_MODE,ACTIVE_LONG) )
#define   OHID_MODE_SET_ACTIVE_LONG(mode)       ((mode) |= OHID(KB_MODE,ACTIVE_LONG) )
#define   OHID_MODE_CLR_ACTIVE_LONG(mode)       ((mode) &= (~OHID(KB_MODE,ACTIVE_LONG) ))
/******************************************************************************
* 按键 DKS 模式触发状态定义
*******************************************************************************/
enum OHID_KB_DKS {
    OHID(DKS,PRESS)                     = 0x01, // 按下
    OHID(DKS,PRESS_HOLD)                = 0x02, // 按下保持
    OHID(DKS,PRESS_BOTTON)              = 0x04, // 触底
    OHID(DKS,BOTTON_HOLD)               = 0x18, // 触底保持
    OHID(DKS,UP)                        = 0x20, // 抬手
    OHID(DKS,UP_HOLD)                   = 0x40, // 抬手保持
    OHID(DKS,FREE)                      = 0x80, // 完全释放
	/*************************** Alias ********************************/
    DKS_PRESS                           = OHID(DKS,PRESS)        , // 
    DKS_PHOLD                           = OHID(DKS,PRESS_HOLD)   , // 
    DKS_PBOTTON                         = OHID(DKS,PRESS_BOTTON) , // 
    DKS_HOLD                            = OHID(DKS,BOTTON_HOLD)  , // 
    DKS_UP                              = OHID(DKS,UP)           , // 
    DKS_UHOLD                           = OHID(DKS,UP_HOLD)      , // 
    DKS_FREE                            = OHID(DKS,FREE)         , // 
};
/******************************************************************************
* 按键 DKS 处理宏定义
*******************************************************************************/
#define   OHID_ADD_DKS(status,dks)              ((status) |= dks)     //
#define   OHID_IS_DKS_PRESS(status)             (OHID_DKS_PRESS        == ((status)&OHID_DKS_PRESS))
#define   OHID_IS_DKS_PRESS_HOLD(status)        (OHID_DKS_PRESS_HOLD   == ((status)&OHID_DKS_PRESS_HOLD))
#define   OHID_IS_DKS_PRESS_BOTTON(status)      (OHID_DKS_PRESS_BOTTON == ((status)&OHID_DKS_PRESS_BOTTON))
#define   OHID_IS_DKS_BOTTON_HOLD(status)       (OHID_DKS_BOTTON_HOLD  == ((status)&OHID_DKS_BOTTON_HOLD))
#define   OHID_IS_DKS_UP(status)                (OHID_DKS_UP           == ((status)&OHID_DKS_UP))
#define   OHID_IS_DKS_UP_HOLD(status)           (OHID_DKS_UP_HOLD      == ((status)&OHID_DKS_UP_HOLD))
#define   OHID_IS_DKS_FREE(status)              (OHID_DKS_FREE         == ((status)&OHID_DKS_FREE))
///******************************************************************************
//* 宏指令编码定义, 其中 TrPs 的取值为 0x01(按下)/0x00(释放)
//*******************************************************************************/
//#define   OHID_MACRO_CMD(key,TrPs,delay)        ( (((TrPs)<<28)&0xF0000000) | (((delay)<<16)&0x0FFF0000) | ((key)&0xFFFF) )
//#define   OHID_MACRO_TRIG(cmd)                  ((cmd>>28)&0x0F)
//#define   OHID_MACRO_KEY(cmd)                   (cmd&0xFFFF)
//#define   OHID_MACRO_DELAY(cmd)                 ((cmd>>16)&0x0FFF)
/******************************************************************************
** 宏指令由 3部分构成, 分别是: 按键, 延时, 事件(触发/释放)
** 按键: 16bit 编码
** 延时: 14bit 编码, 最小值1(0+1) 最大值 16384(16383+1),单位 ms
** 事件: 2bit  编码, 取 enum OHID_TKB_TRPS 中定义的值，但实际只使用 0x1(触发) 和 0x0(释放) 两个值
** 使用键值 KC_NO 将认为宏序列结束
*******************************************************************************/
#define   OHID_MACRO_CMD(key,Event,delay)        ( ((((uint32_t)(Event))<<30)&0xC0000000) | ((((uint32_t)(delay))<<16)&0x3FFF0000) | ((key)&0xFFFF) )
//#define   OHID_MACRO_EVENT(cmd)                  ((cmd>>30)&0x03)
#define   OHID_MACRO_EVENT(cmd)                  ((cmd>>30)&0x03) //( (((uint32_t)cmd)>>30)&0x03)
#define   OHID_MACRO_KEY(cmd)                    ((cmd)&0xFFFF)
#define   OHID_MACRO_DELAY(cmd)                  ( (cmd>>16)&0x3FFF) //( (( ((uint32_t)(cmd))>>16)&0x3FFF) )


/******************************************************************************
** RGB 模式, 其中 [0x00-0x14] 为固定模式, 0x15 开始为扩展模式,不同需求的产品具体定义可能会不同
*******************************************************************************/
enum ohid_rgb_modes_t{
	//OHID(RGB_MODE,INVALID)        = 0xFF,   // 无效
	OHID(RGB_MODE,NONE)           = 0xFF,   // 空
	OHID(RGB_MODE,NEXT_GROUP)     = 0xFE,   // 下一组
    OHID(RGB_MODE,MASK)           = 0x3F,
	OHID(RGB_MODE,DIRECT)         = 0x00,   // 直驱模式,eg:OpenRGB+Artemis
	OHID(RGB_MODE,WHITE)          = 0x01,   // 自检白光
	OHID(RGB_MODE,RECORD)         = 0x02,   // 录制 (not using)
	OHID(RGB_MODE,UDEF1)          = 0x03,   // 用户定义1
	OHID(RGB_MODE,UDEF2)          = 0x04,   // 用户定义2
	OHID(RGB_MODE,UDEF3)          = 0x05,   // 用户定义3
	OHID(RGB_MODE,UDEF4)          = 0x06,   // 用户定义4
	OHID(RGB_MODE,UDEF5)          = 0x07,   // 用户定义5 
	OHID(RGB_MODE,CLICK)          = 0x08,   // click, 单点
	OHID(RGB_MODE,LIGHT)          = 0x09,   // light, 恒亮
	OHID(RGB_MODE,TRACELESS)      = 0x0A,   // traceless, 踏雪无痕
	OHID(RGB_MODE,BREATHE)        = 0x0B,   // Breathing, 呼吸
	OHID(RGB_MODE,RIPPLE)         = 0x0C,   // ripple, 涟漪
	OHID(RGB_MODE,WAVE)           = 0x0D,   // wave, 波浪	
	OHID(RGB_MODE,RAIN)           = 0x0E,   // Rain, 雨滴
	OHID(RGB_MODE,RACE)           = 0x0F,   // horse race lamp, 跑马
	OHID(RGB_MODE,SIN)            = 0x10,   // Sinusoid, 弦曲曲线
	OHID(RGB_MODE,SPIRAL)         = 0x11,   // Spiral, 螺旋
	OHID(RGB_MODE,FILL)           = 0x12,   // Fill, 充满
	OHID(RGB_MODE,STACK)          = 0x13,   // Stacks, 堆
	OHID(RGB_MODE,SWAP)           = 0x14,   // Swap, 交换
	/*************************** 侧灯 ********************************/
	OHID(RGB_SIDE,OFF)            = 0x0,
	OHID(RGB_SIDE,ON)             = 0x1,
	OHID(RGB_SIDE,BEART)          = 0x2,
	/*************************** redefine ********************************/
	OHID(RGB_MODE,MODE1)          = 0x15,   //
	OHID(RGB_MODE,MODE2)          = 0x16,   //
	OHID(RGB_MODE,MODE3)          = 0x17,   //
	OHID(RGB_MODE,MODE4)          = 0x18,   // 
	OHID(RGB_MODE,MODE5)          = 0x19,   // 
	OHID(RGB_MODE,MODE6)          = 0x1A,   //
	OHID(RGB_MODE,MODE7)          = 0x1B,   //
	OHID(RGB_MODE,MODE8)          = 0x1C,   //
	OHID(RGB_MODE,MODE9)          = 0x1D,   //
	OHID(RGB_MODE,MODE10)         = 0x1E,   //
	OHID(RGB_MODE,MODE11)         = 0x1F,   //
	OHID(RGB_MODE,MODE12)         = 0x20,   //
	OHID(RGB_MODE,MODE13)         = 0x21,   //
	OHID(RGB_MODE,MODE14)         = 0x22,   //
	OHID(RGB_MODE,MODE15)         = 0x23,   //
	OHID(RGB_MODE,MODE16)         = 0x24,   //
	OHID(RGB_MODE,MODE17)         = 0x25,   //
	OHID(RGB_MODE,MODE18)         = 0x26,   //
	OHID(RGB_MODE,MODE19)         = 0x27,   //
	OHID(RGB_MODE,MODE20)         = 0x28,   //
	OHID(RGB_MODE,MODE21)         = 0x29,   //
	OHID(RGB_MODE,MODE22)         = 0x2A,   //
	OHID(RGB_MODE,MODE23)         = 0x2B,   //
	OHID(RGB_MODE,MODE24)         = 0x2C,   //
	OHID(RGB_MODE,MODE25)         = 0x2D,   //
	OHID(RGB_MODE,MODE26)         = 0x2E,   //
	OHID(RGB_MODE,MODE27)         = 0x2F,   //
	OHID(RGB_MODE,MODE28)         = 0x30,   //
	OHID(RGB_MODE,MODE29)         = 0x31,   //
	OHID(RGB_MODE,MODE30)         = 0x32,   //
	OHID(RGB_MODE,MODE31)         = 0x33,   //
	OHID(RGB_MODE,MODE32)         = 0x34,   //
	OHID(RGB_MODE,MODE33)         = 0x35,   //
	OHID(RGB_MODE,MODE34)         = 0x36,   //
	OHID(RGB_MODE,MODE35)         = 0x37,   //
	OHID(RGB_MODE,MODE36)         = 0x38,   //
	OHID(RGB_MODE,MODE37)         = 0x39,   //
	OHID(RGB_MODE,MODE38)         = 0x3A,   //
	OHID(RGB_MODE,MODE39)         = 0x3B,   //
	OHID(RGB_MODE,MODE40)         = 0x3C,   //
	OHID(RGB_MODE,MODE41)         = 0x3D,   //
	OHID(RGB_MODE,MODE42)         = 0x3E,   //
	OHID(RGB_MODE,MODE43)         = 0x3F,   //
	
	/*************************** Alias ********************************/
	//RGB_MODE_INVALID        = OHID(RGB_MODE,INVALID)        ,   // 无效
	RGB_MODE_NONE           = OHID(RGB_MODE,NONE)           ,
	RGB_MODE_NEXT_GROUP     = OHID(RGB_MODE,NEXT_GROUP)     ,
	RGB_MODE_PREVIRE        = OHID(RGB_MODE,MASK)           ,   // preview, 预览
	RGB_MODE_MASK           = OHID(RGB_MODE,MASK)           ,
	RGB_MODE_DIRECT         = OHID(RGB_MODE,DIRECT)         ,   // 直驱模式,eg:OpenRGB+Artemis
//	RGB_MODE_WHITE          = OHID(RGB_MODE,WHITE)          ,   // 自检白光
	RGB_MODE_RECORD         = OHID(RGB_MODE,RECORD)         ,   // 录制 (not using)
	RGB_MODE_UDEF1          = OHID(RGB_MODE,UDEF1)          ,   // 用户定义1
	RGB_MODE_UDEF2          = OHID(RGB_MODE,UDEF2)          ,   // 用户定义2
	RGB_MODE_UDEF3          = OHID(RGB_MODE,UDEF3)          ,   // 用户定义3
	RGB_MODE_UDEF4          = OHID(RGB_MODE,UDEF4)          ,   // 用户定义4
	RGB_MODE_UDEF5          = OHID(RGB_MODE,UDEF5)          ,   // 用户定义5 
	RGB_MODE_CLICK          = OHID(RGB_MODE,CLICK)          ,   // click, 单点
	RGB_MODE_LIGHT          = OHID(RGB_MODE,LIGHT)          ,   // light, 恒亮
	RGB_MODE_TRACELESS      = OHID(RGB_MODE,TRACELESS)      ,   // traceless, 踏雪无痕
	RGB_MODE_BREATHE        = OHID(RGB_MODE,BREATHE)        ,   // Breathing, 呼吸
	RGB_MODE_RIPPLE         = OHID(RGB_MODE,RIPPLE)         ,   // ripple, 涟漪
	RGB_MODE_WAVE           = OHID(RGB_MODE,WAVE)           ,   // wave, 波浪	
	RGB_MODE_RAIN           = OHID(RGB_MODE,RAIN)           ,   // Rain, 雨滴
	RGB_MODE_RACE           = OHID(RGB_MODE,RACE)           ,   // horse race lamp, 跑马
    RGB_MODE_SIN            = OHID(RGB_MODE,SIN)            ,   // Sinusoid, 正弦曲曲线
	RGB_MODE_SPIRAL         = OHID(RGB_MODE,SPIRAL)         ,   // Spiral, 螺旋
	RGB_MODE_FILL           = OHID(RGB_MODE,FILL)           ,   // Fill, 充满
	RGB_MODE_STACK          = OHID(RGB_MODE,STACK)          ,   // Stacks, 堆
	RGB_MODE_SWAP           = OHID(RGB_MODE,SWAP)           ,   // Swap, 交换
	/*************************** redefine ********************************/
	RGB_MODE1               = OHID(RGB_MODE,MODE1)          ,   //
	RGB_MODE2               = OHID(RGB_MODE,MODE2)          ,   //
	RGB_MODE3               = OHID(RGB_MODE,MODE3)          ,   //
	RGB_MODE4               = OHID(RGB_MODE,MODE4)          ,   // 
	RGB_MODE5               = OHID(RGB_MODE,MODE5)          ,   // 
	RGB_MODE6               = OHID(RGB_MODE,MODE6)          ,   //
	RGB_MODE7               = OHID(RGB_MODE,MODE7)          ,   //
	RGB_MODE8               = OHID(RGB_MODE,MODE8)          ,   //
	RGB_MODE9               = OHID(RGB_MODE,MODE9)          ,   //
	RGB_MODE10              = OHID(RGB_MODE,MODE10)         ,   //
	RGB_MODE11              = OHID(RGB_MODE,MODE11)         ,   //
	RGB_MODE12              = OHID(RGB_MODE,MODE12)         ,   //
	RGB_MODE13              = OHID(RGB_MODE,MODE13)         ,   //
	RGB_MODE14              = OHID(RGB_MODE,MODE14)         ,   //
	RGB_MODE15              = OHID(RGB_MODE,MODE15)         ,   //
	RGB_MODE16              = OHID(RGB_MODE,MODE16)         ,   //
	RGB_MODE17              = OHID(RGB_MODE,MODE17)         ,   //
	RGB_MODE18              = OHID(RGB_MODE,MODE18)         ,   //
	RGB_MODE19              = OHID(RGB_MODE,MODE19)         ,   //
	RGB_MODE20              = OHID(RGB_MODE,MODE20)         ,   //
	RGB_MODE21              = OHID(RGB_MODE,MODE21)         ,   //
	RGB_MODE22              = OHID(RGB_MODE,MODE22)         ,   //
	RGB_MODE23              = OHID(RGB_MODE,MODE23)         ,   //
	RGB_MODE24              = OHID(RGB_MODE,MODE24)         ,   //
	RGB_MODE25              = OHID(RGB_MODE,MODE25)         ,   //
	RGB_MODE26              = OHID(RGB_MODE,MODE26)         ,   //
	RGB_MODE27              = OHID(RGB_MODE,MODE27)         ,   //
	RGB_MODE28              = OHID(RGB_MODE,MODE28)         ,   //
	RGB_MODE29              = OHID(RGB_MODE,MODE29)         ,   //
	RGB_MODE30              = OHID(RGB_MODE,MODE30)         ,   //
	RGB_MODE31              = OHID(RGB_MODE,MODE31)         ,   //
	RGB_MODE32              = OHID(RGB_MODE,MODE32)         ,   //
	RGB_MODE33              = OHID(RGB_MODE,MODE33)         ,   //
	RGB_MODE34              = OHID(RGB_MODE,MODE34)         ,   //
	RGB_MODE35              = OHID(RGB_MODE,MODE35)         ,   //
	RGB_MODE36              = OHID(RGB_MODE,MODE36)         ,   //
	RGB_MODE37              = OHID(RGB_MODE,MODE37)         ,   //
	RGB_MODE38              = OHID(RGB_MODE,MODE38)         ,   //
	RGB_MODE39              = OHID(RGB_MODE,MODE39)         ,   //
	RGB_MODE40              = OHID(RGB_MODE,MODE40)         ,   //
	RGB_MODE41              = OHID(RGB_MODE,MODE41)         ,   //
	RGB_MODE42              = OHID(RGB_MODE,MODE42)         ,   //
	RGB_MODE43              = OHID(RGB_MODE,MODE43)         ,   //
	/*************************** 侧灯 ********************************/
	RGB_SUB_OFF             = OHID(RGB_SIDE,OFF)            ,
	RGB_SUB_ON              = OHID(RGB_SIDE,ON)             ,
	RGB_SUB_BEART           = OHID(RGB_SIDE,BEART)          ,
	RGB_SIDE_OFF            = OHID(RGB_SIDE,OFF)            ,
	RGB_SIDE_ON             = OHID(RGB_SIDE,ON)             ,
	RGB_SIDE_BEART          = OHID(RGB_SIDE,BEART)          ,
	/*************************** LED Strip ********************************/
	RGB_LEDSTRIP_LIGHT      = 0x00                          ,   // light, 恒亮
	RGB_LEDSTRIP_BREATHE    = 0x01                          ,   // Breathing, 呼吸
	RGB_LEDSTRIP_RIPPLE     = 0x02                          ,   // ripple, 涟漪
	RGB_LEDSTRIP_WAVE       = 0x03                          ,   // wave, 波浪	
	RGB_LEDSTRIP_RAIN       = 0x04                          ,   // Rain, 雨滴
	RGB_LEDSTRIP_RACE       = 0x05                          ,   // horse race lamp, 跑马
    RGB_LEDSTRIP_SIN        = 0x06                          ,   // Sinusoid, 正弦曲曲线
	RGB_LEDSTRIP_SPIRAL     = 0x07                          ,   // Spiral, 螺旋
	RGB_LEDSTRIP_FILL       = 0x08                          ,   // Fill, 充满
	RGB_LEDSTRIP_STACK      = 0x09                          ,   // Stacks, 堆
	RGB_LEDSTRIP_SWAP       = 0x0A                          ,   // Swap, 交换
	RGB_LEDSTRIP_ROC        = 0x0B                          ,   // Roc, 大鹏展翅
	RGB_LEDSTRIP_MAX        = 0x0B                          ,   // MAX
	
	/*************************** Mode ********************************/
	RGB_MODE_ROC            = RGB_MODE1                     ,   // Roc, 大鹏展翅
	/*************************** 扩展效果 ********************************/
	RGB_MODE_BOBALL         = RGB_MODE2                     ,   // BoBall(BouncingBall), 弹力球
	RGB_MODE_BUBBLES        = RGB_MODE3                     ,   // Bubbles, --气泡	
	RGB_MODE_SINUSOID       = RGB_MODE4                     ,   // Sinusoid, 正弦曲线
	RGB_MODE_WAVW_GRAD      = RGB_MODE5                     ,   // GradientWave, 梯度波
	RGB_MODE_MOVE_PANES     = RGB_MODE6                     ,   // MovingPanes, 移动窗格
	RGB_MODE_SMOOTH_BLINK   = RGB_MODE7                     ,   // SmoothBlink, 平滑闪烁
	RGB_MODE_WINNOWER       = RGB_MODE8                     ,   // Winnower, 风车效果
	RGB_MODE_STARRY_NIGHT   = RGB_MODE9                     ,   // StarryNight, 星空
	RGB_MODE_SWIRL_CIRCLES  = RGB_MODE10                    ,   // SwirlCircles, 漩涡圈
	RGB_MODE_WAVY           = RGB_MODE11                    ,   // Wavy, 波形的
	RGB_MODE_BEAMS          = RGB_MODE12                    ,   // Beams, 交叉梁
	RGB_MODE_BEAM_ROTATION  = RGB_MODE13                    ,   // RotatingBeam, 旋转射束
	// Rainbow
	RGB_MODE_COLOR_WHEEL    = RGB_MODE14                    ,   // ColorWheel, 色轮
	RGB_MODE_DR_BOW         = RGB_MODE15                    ,   // DoubleRotatingRainbow, 双旋转彩虹
	RGB_MODE_HYPN           = RGB_MODE16                    ,   // Hypnotoad
	RGB_MODE_NOISE_MAP      = RGB_MODE17                    ,   // NoiseMap, 噪声地图
	RGB_MODE_BOW_WAVE       = RGB_MODE18                    ,   // RainbowWave, 彩虹波
	RGB_MODE_BOW_RADIAL     = RGB_MODE19                    ,   // RadialRainbow, 径向彩虹
	RGB_MODE_BOW_ROTATION   = RGB_MODE20                    ,   // RotatingRainbow, 旋转的彩虹
	RGB_MODE_SPECTRUM       = RGB_MODE21                    ,   // SpectrumCycling, 光谱骑自行车/循环
	// Random
	RGB_MODE_BLOOM          = RGB_MODE22                    ,   // Bloom, 花朵
	RGB_MODE_LIGHTNING      = RGB_MODE23                    ,   // Lightning, 闪电
	// Simple
	RGB_MODE_BREATHE_CIRCLE = RGB_MODE24                    ,   // BreathingCircle, 呼吸循环
	RGB_MODE_COMET          = RGB_MODE25                    ,   // Comet, 彗星
	RGB_MODE_MARQUEE        = RGB_MODE26                    ,   // Marquee, 大天幕
	RGB_MODE_MOSAIC         = RGB_MODE27                    ,   // Mosaic, 马赛克	
	RGB_MODE_MOTION_POINT   = RGB_MODE28                    ,   // MotionPoint, 运动点
	RGB_MODE_MOTION_POINTS  = RGB_MODE29                    ,   // MotionPoints, 运动点
	RGB_MODE_VISOR          = RGB_MODE30                    ,   // Visor, 遮阳板
	RGB_MODE_GAME           = RGB_MODE31                    ,   // Game, 游戏模式
};
/******************************************************************************
* 自动上报类型定义定义
*******************************************************************************/
enum OHID_RP_TYPE {
    OHID(RP_TYPE,KEYS)                = (0x00<<2)&0xFC, // 按键行程
    OHID(RP_TYPE,KEYSH)               = (0x01<<2)&0xFC, // 按键行程
	/*************************** Alias ********************************/
    RP_KEYS                           = OHID(RP_TYPE,KEYS)        , // 
    RP_KEYSH                          = OHID(RP_TYPE,KEYSH)       , //
    /*************************** List ********************************/
    RP_KEYS0                          = RP_KEYS | 0x0             , //
    RP_KEYS1                          = RP_KEYS | 0x1             , //
    RP_KEYSH0                         = RP_KEYSH | 0x0            , //
    RP_KEYSH1                         = RP_KEYSH | 0x1            , //
    RP_KEYSH2                         = RP_KEYSH | 0x2            , //
    RP_KEYSH3                         = RP_KEYSH | 0x3            , //
};
#define   RP_TYPE(item,idx)           ((enum OHID_RP_TYPE)((item) | (idx&0x03)))

/******************************************************************************
* 轴体类型定义
*******************************************************************************/
enum OHID_AXES_TYPE {
    OHID(AXES_TYPE,TIANKONG)          = 0x00, // 天空轴      ,行程 4.0mm
    OHID(AXES_TYPE,CIYU)              = 0x01, // 磁玉轴      ,行程 3.5mm
	OHID(AXES_TYPE,TTC)               = 0x02, // TTC万磁王轴 ,行程 3.5mm
	OHID(AXES_TYPE,DASHI)             = 0x03, // 大师轴      ,行程 3.6mm
	/*************************** Alias ********************************/
    OHID_AXES_TIANKONG                = OHID(AXES_TYPE,TIANKONG)   , //  天空轴      ,行程 4.0mm
    OHID_AXES_CIYU                    = OHID(AXES_TYPE,CIYU)       , //  磁玉轴      ,行程 3.5mm
	OHID_AXES_TTC                     = OHID(AXES_TYPE,TTC)        , //  TTC万磁王轴 ,行程 3.5mm
	OHID_AXES_DASHI                   = OHID(AXES_TYPE,DASHI)      , //  大师轴      ,行程 3.6mm
};

/******************************************************************************
** 按键数据定义
*******************************************************************************/
union OHIDM_Driver_Key_t{
    //uint16_t MKEY;
    uint16_t Half[7];  // 对齐
    uint16_t Key;
    struct {
        uint8_t Gmm;
        uint8_t RSTmm;
    }Signal;
    struct {
        uint8_t Gmm;
        uint8_t RTmm;
        uint8_t RSTmm;
    }RT;
    struct {
        uint8_t Gmm;
        uint8_t RSTmm;
    }DT;
    struct {
        uint16_t Addr;
        uint16_t Long;
    }Macro;
    struct {
        uint16_t NewKEY;
    }Append;
    struct {
        uint16_t KEY1;
        uint16_t KEY2;
        uint16_t KEY3;
        uint16_t KEY4;
        uint8_t TrPs1;
        uint8_t TrPs2;
        uint8_t TrPs3;
        uint8_t TrPs4;
        uint8_t mm1;
    }DKS;
    struct {
        uint16_t KEY1;
        uint16_t KEY2;
        uint16_t KEY3;
        uint8_t mm1;
        uint8_t mm2;
        uint8_t mm3;
    }AKS;
    struct {
        uint16_t KEY1;
        uint16_t KEY2;
        uint16_t delay;
    }MT;
    struct {
        uint16_t KEY1;
        uint16_t delay;
    }TGL;
    struct {
        uint16_t KEY1;
        uint16_t Mode;
    }Quick;
};

struct Macro_PT_t {
	uint16_t MacroID;
	uint16_t Addr;
	uint16_t Long;
};

/******************************************************************************
** API
*******************************************************************************/
extern void OHIDM_Base_mix(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order);
extern void OHIDM_Base_mix_bin(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t bin[59], uint8_t len);
extern void OHIDM_Base_mix_word(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint32_t word[14], uint8_t len);
extern void OHIDM_Base_mix_rgb_test(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t srart, const uint32_t color);
extern void OHIDM_Base_mix_key_test(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t srart, const uint16_t delay, const uint8_t tick);
extern void OHIDM_Base_mix_arg(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t arg);
extern void OHIDM_Base_mix_arg2(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t arg1, const uint8_t arg2);
extern void OHIDM_Base_mix_arg2B(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t arg1, const uint16_t arg2);
extern void OHIDM_Base_mix_product_sn(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t SN[4*6], const uint8_t SIGN[16]);
extern void OHIDS_Base_mix_product_sn(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t SN[4*6], const uint8_t SIGN[16]);
extern void OHIDS_Base_mix(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order);
extern void OHIDS_Base_mix_bin(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t bin[59], uint8_t len);
extern void OHIDS_Base_mix_word(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint32_t word[14], uint8_t len);
extern void OHIDS_Base_mix_rgb_test(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t srart, const uint32_t color);
extern void OHIDS_Base_mix_key_test(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t srart, const uint16_t delay, const uint8_t tick);
extern void OHIDS_Base_mix_arg(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t arg);
extern void OHIDS_Base_mix_arg2(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t arg1, const uint8_t arg2);
extern void OHIDS_Base_mix_arg3(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t arg1, const uint8_t arg2, const uint8_t arg3);
extern uint8_t OHID_Base_mix_get_byte(union OpenAgreementHID_t* const Pack, const uint8_t index);
extern uint16_t OHID_Base_mix_get_half(union OpenAgreementHID_t* const Pack, const uint8_t index);
extern uint32_t OHID_Base_mix_get_word(union OpenAgreementHID_t* const Pack, const uint8_t index);

/******************************* 驱动指令 ***********************************/
#define   GROUP_LEN_PARAM         (12)
#define   GROUP_LEN_MT            (7)
#define   GROUP_LEN_TGL           (10)
#define   GROUP_LEN_DKS           (4)
#define   GROUP_LEN_AKS3          (5)
#define   GROUP_LEN_KRBG          (12)
#define   GROUP_LEN_IRBG          (15)
#define   GROUP_LEN_RP            (59)
#define   GROUP_LEN_RP_HALF       (29)
#define   GROUP_LEN_CURVE         (28)
#define   GROUP_LEN_KEY           (4)
#define   GROUP_LEN_MACROLIST     (14)
// Param,[4B]+[5*12 B] 
extern void OHIDM_Driver_Param(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_PARAM], const enum OHID_PARAM_PAGE Item[GROUP_LEN_PARAM], const uint16_t Value[GROUP_LEN_PARAM]);
extern void OHIDS_Driver_Param(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_PARAM], const enum OHID_PARAM_PAGE Item[GROUP_LEN_PARAM], const uint16_t Value[GROUP_LEN_PARAM]);
// MT, Reference Wooting, [4B]+[8*7 B]
extern void OHIDM_Driver_MT(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_MT], const uint16_t KEY1[GROUP_LEN_MT], const uint16_t KEY2[GROUP_LEN_MT], const uint16_t delay[GROUP_LEN_MT]);
extern void OHIDS_Driver_MT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_MT], const uint16_t KEY1[GROUP_LEN_MT], const uint16_t KEY2[GROUP_LEN_MT], const uint16_t delay[GROUP_LEN_MT]);
// TGL, Reference Wooting,[4B]+[6*10 B]
extern void OHIDM_Driver_TGL(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_TGL], const uint16_t KEY1[GROUP_LEN_TGL], const uint16_t delay[GROUP_LEN_TGL]);
extern void OHIDS_Driver_TGL(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_TGL], const uint16_t KEY1[GROUP_LEN_TGL], const uint16_t delay[GROUP_LEN_TGL]);
// DKS, Reference Wooting, [4B]+[15*4 B]
extern void OHIDM_Driver_DKS(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_DKS], const uint16_t KEY1[GROUP_LEN_DKS], const uint16_t KEY2[GROUP_LEN_DKS], const uint16_t KEY3[GROUP_LEN_DKS], const uint16_t KEY4[GROUP_LEN_DKS], const uint8_t TrPs1[GROUP_LEN_DKS], const uint8_t TrPs2[GROUP_LEN_DKS], const uint8_t TrPs3[GROUP_LEN_DKS], const uint8_t TrPs4[GROUP_LEN_DKS], const uint8_t mm1[GROUP_LEN_DKS]);
extern void OHIDS_Driver_DKS(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_DKS], const uint16_t KEY1[GROUP_LEN_DKS], const uint16_t KEY2[GROUP_LEN_DKS], const uint16_t KEY3[GROUP_LEN_DKS], const uint16_t KEY4[GROUP_LEN_DKS], const uint8_t TrPs1[GROUP_LEN_DKS], const uint8_t TrPs2[GROUP_LEN_DKS], const uint8_t TrPs3[GROUP_LEN_DKS], const uint8_t TrPs4[GROUP_LEN_DKS], const uint8_t mm1[GROUP_LEN_DKS]);
// AKS, Reference 海盗船, [4B]+[11*5 B]
extern void OHIDM_Driver_AKS3(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_AKS3], const uint16_t KEY1[GROUP_LEN_AKS3], const uint16_t KEY2[GROUP_LEN_AKS3], const uint16_t KEY3[GROUP_LEN_AKS3], const uint8_t mm1[GROUP_LEN_AKS3], const uint8_t mm2[GROUP_LEN_AKS3], const uint8_t mm3[GROUP_LEN_AKS3]);
extern void OHIDS_Driver_AKS3(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_AKS3], const uint16_t KEY1[GROUP_LEN_AKS3], const uint16_t KEY2[GROUP_LEN_AKS3], const uint16_t KEY3[GROUP_LEN_AKS3], const uint8_t mm1[GROUP_LEN_AKS3], const uint8_t mm2[GROUP_LEN_AKS3], const uint8_t mm3[GROUP_LEN_AKS3]);
extern void OHIDM_Driver_Key(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_KEY], const union OHIDM_Driver_Key_t Keys[GROUP_LEN_KEY], const enum OHID_KB_MODE Mode[GROUP_LEN_KEY]);
extern void OHIDS_Driver_Key(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_KEY], const union OHIDM_Driver_Key_t Keys[GROUP_LEN_KEY], const enum OHID_KB_MODE Mode[GROUP_LEN_KEY]);
// Trigger
//extern void OHIDM_Driver_Trigger(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint8_t MM, const uint8_t Reset, const uint8_t RTmm, const uint8_t RTreset, const uint8_t mm1, const uint8_t mm2, const uint8_t mm3);
//extern void OHIDS_Driver_Trigger(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t MM, const uint8_t Reset, const uint8_t RTmm, const uint8_t RTreset, const uint8_t mm1, const uint8_t mm2, const uint8_t mm3);
// RGB_PARAM
extern void OHIDM_Driver_RGB_PARAM(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint32_t back, const uint32_t Palettes[8], const uint8_t gray, const uint8_t mode, const uint8_t speed, const uint8_t sleep, const uint8_t on, const uint8_t on_sleep, const uint8_t reverse);
extern void OHIDS_Driver_RGB_PARAM(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t back, const uint32_t Palettes[8],  const uint8_t gray, const uint8_t mode, const uint8_t speed, const uint8_t sleep, const uint8_t on, const uint8_t on_sleep, const uint8_t reverse);
// KRGB, 提供以下两种接口, [4B]+[5*12 B]
extern void OHIDM_Driver_krgb2(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_KRBG], const uint8_t  RGBR[GROUP_LEN_KRBG], const uint8_t RGBG[GROUP_LEN_KRBG], const uint8_t RGBB[GROUP_LEN_KRBG]);
extern void OHIDS_Driver_krgb2(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_KRBG], const uint8_t  RGBR[GROUP_LEN_KRBG], const uint8_t RGBG[GROUP_LEN_KRBG], const uint8_t RGBB[GROUP_LEN_KRBG]);
extern void OHIDM_Driver_krgb3(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_KRBG], const uint32_t RGB[GROUP_LEN_KRBG]);
extern void OHIDS_Driver_krgb3(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_KRBG], const uint32_t RGB[GROUP_LEN_KRBG]);
// IRGB
extern void OHIDM_Driver_IRGB1(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint32_t irgb[GROUP_LEN_IRBG]);
extern void OHIDS_Driver_IRGB1(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t irgb[GROUP_LEN_IRBG]);
extern void OHIDM_Driver_IRGB2(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint8_t idx[GROUP_LEN_IRBG], const uint32_t RGB[GROUP_LEN_IRBG]);
extern void OHIDS_Driver_IRGB2(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t idx[GROUP_LEN_IRBG], const uint32_t RGB[GROUP_LEN_IRBG]);
// Macro
extern void OHIDM_Driver_MacroList(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint32_t macro[GROUP_LEN_MACROLIST], uint8_t len, const uint16_t offset);
extern void OHIDS_Driver_MacroList(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t macro[GROUP_LEN_MACROLIST], uint8_t len, const uint16_t offset);
// RP
//extern void OHIDM_Driver_RP(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_RP_TYPE type, const uint16_t delay);
extern void OHIDS_Driver_RP(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_RP_TYPE type, const uint8_t bin[GROUP_LEN_RP]);
extern void OHIDS_Driver_RP_Half(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_RP_TYPE type, const uint16_t half[GROUP_LEN_RP_HALF]);
// curve
extern void OHIDM_Driver_Curve(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint8_t idx, uint8_t len, const uint16_t offset, const uint16_t curve[GROUP_LEN_CURVE]);
extern void OHIDS_Driver_Curve(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t idx, uint8_t len, const uint16_t offset, const uint16_t curve[GROUP_LEN_CURVE]);

#define GROUP_LEN_NORMAL     (30)
#define GROUP_LEN_QUICK      (20)
#define GROUP_LEN_SOCD       (12)
#define GROUP_LEN_SINGLE     (15)
#define GROUP_LEN_RT         (12)
#define GROUP_LEN_DT         (15)
#define GROUP_LEN_MACRO      (10)
#define GROUP_LEN_MACRO_PT   ( 9)
#define GROUP_LEN_APPEND     (15)
// NORMAL
extern void OHIDM_Driver_Normal(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_NORMAL]);
extern void OHIDS_Driver_Normal(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_NORMAL]);
// Quick
extern void OHIDM_Driver_Quick(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_QUICK], const uint8_t Mode[GROUP_LEN_QUICK]);
extern void OHIDS_Driver_Quick(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_QUICK], const uint8_t Mode[GROUP_LEN_QUICK]);
// SOCD
extern void OHIDM_Driver_SOCD(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_SOCD], const uint8_t Mode[GROUP_LEN_SOCD], const uint8_t MM[GROUP_LEN_SOCD], const uint8_t RST[GROUP_LEN_SOCD]);                  
extern void OHIDS_Driver_SOCD(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_SOCD], const uint8_t Mode[GROUP_LEN_SOCD], const uint8_t MM[GROUP_LEN_SOCD], const uint8_t RST[GROUP_LEN_SOCD]);
// SINGLE
extern void OHIDM_Driver_SIGNAL(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_SINGLE], const uint8_t MM[GROUP_LEN_SINGLE], const uint8_t Reset[GROUP_LEN_SINGLE]);
extern void OHIDS_Driver_SIGNAL(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_SINGLE], const uint8_t MM[GROUP_LEN_SINGLE], const uint8_t Reset[GROUP_LEN_SINGLE]);
// RT
extern void OHIDM_Driver_RT(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_RT], const uint8_t MM[GROUP_LEN_RT], const uint8_t RTmm[GROUP_LEN_RT], const uint8_t Reset[GROUP_LEN_RT]);
extern void OHIDS_Driver_RT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_RT], const uint8_t MM[GROUP_LEN_RT], const uint8_t RTmm[GROUP_LEN_RT], const uint8_t Reset[GROUP_LEN_RT]);
// DT
extern void OHIDM_Driver_DT(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_DT], const uint8_t DTmm[GROUP_LEN_DT], const uint8_t DTreset[GROUP_LEN_DT]);
extern void OHIDS_Driver_DT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_DT], const uint8_t DTmm[GROUP_LEN_DT], const uint8_t DTreset[GROUP_LEN_DT]);
// MACRO
extern void OHIDM_Driver_Macro_Key(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_MACRO], const uint16_t Addr[GROUP_LEN_MACRO], const uint16_t Long[GROUP_LEN_MACRO]);
extern void OHIDS_Driver_Macro_Key(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_MACRO], const uint16_t Addr[GROUP_LEN_MACRO], const uint16_t Long[GROUP_LEN_MACRO]);
extern void OHIDM_Driver_Macro_PT(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t Page, const struct Macro_PT_t MPT[GROUP_LEN_MACRO_PT]);
extern void OHIDS_Driver_Macro_PT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t Page, const struct Macro_PT_t MPT[GROUP_LEN_MACRO_PT]);
extern void OHIDS_Driver_Get_MPT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, struct Macro_PT_t MPT[GROUP_LEN_MACRO_PT]);
// FREE
extern void OHIDM_Driver_Append(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_APPEND], const uint16_t NewKEY[GROUP_LEN_APPEND]);
extern void OHIDS_Driver_Append(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_APPEND], const uint16_t NewKEY[GROUP_LEN_APPEND]);

#ifdef __cplusplus
}
#endif

#endif // _OHID_KEYBOARD_H_

/************************** (C) COPYRIGHT Merafour **************************/
