/******************** (C) COPYRIGHT 2018 merafour ********************
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.1.0
* Date               : 2024.04.07
* Description        : OpenAgreementHID.
* Description        : 注：
* Description        : OHIDM_* : 主机(电脑上位机)发送给从机的数据包
* Description        : OHIDS_* : 从机(键盘)发送给主机的数据包
********************************************************************************
* 数据包格式:
* | S/M    | Pack Len   | Des |        Head[4B]        | <Data> |
* | Master | [4B]+[ nB] | CMD | 0xAA | len | cmd | crc | [ nB]  |
* | Slave  | [4B]+[ nB] | ACK | 0xA5 | len | cmd | crc | [ nB]  |
* head : 协议头,注: head 用于区分 Master 和 Slave 角色;
* len  : data长度;
* CMD  : 命令编码, 其中 bit7为 读/写 位(0xFF除外);
* CRC  : 累加和校验, 计算方式为:crc=head+len+cmd+<data.end>+1+2+3+…+(len+3), 具体参考 OHID_Checksum 函数;
* data : 数据域, 最大长度为 64*4-4 B , 当数据类型为 half word 或 word 时使用小端格式传输;
* Pack Len : 应为 64*n B(HID 全速设备单个数据包为 64字节)
********************************************************************************
* 用于实现协议的基础操作
********************************************************************************
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#ifndef _OHID_PORT_H_
#define _OHID_PORT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define OHID(prefix,field)          OHID_##prefix##_##field//
#ifndef OHID
#define OHID(prefix,field)          OHID_##prefix##_##field//
#endif
#define OHID_PACK_HEAD_MASTRT       (0xAA&0xFF)
#define OHID_PACK_HEAD_SLAVE        (0xA5&0xFF)
#define OHID_CMD_WRITE(cmd)         ((enum OHID_CMD)(0x80|(cmd&0x7F)))
#define OHID_CMD_GET(cmd)           (((uint8_t)cmd)&0x7F)
#define OHID_CMD_IS_WRITE(cmd)      (((uint8_t)cmd)&0x80)

/******************************************************************************
 * 错误码定义
*******************************************************************************/
enum OHID_ERR_CODE {
    OHID(ERR_CODE,INVALID)       = 0xFF,    // 'invalid' response for bad commands
    OHID(ERR_CODE,OVER_ADDR)     = 0x01,    // flash addr over
    OHID(ERR_CODE,OVER_SIZE)     = 0x02,    // flash size over
    OHID(ERR_CODE,OVER_BUFF)     = 0x03,    // buffer over
    OHID(ERR_CODE,ARG)           = 0x04,    // 参数错误
    OHID(ERR_CODE,SIGN_INVALID)  = 0x05,    // 签名失效
    OHID(ERR_CODE,SIGN)          = 0x06,    // 签名错误
    OHID(ERR_CODE,SIGN_DATE)     = 0x07,    // 签名数据错误
    OHID(ERR_CODE,SIGN_SHAKE1)   = 0x08,    // handshake1
    OHID(ERR_CODE,SIGN_SHAKE2)   = 0x09,    // handshake2
    OHID(ERR_CODE,BUSY)          = 0x0A,    // 设备忙
    OHID(ERR_CODE,PROGRAM)       = 0x0B,    // 编程错误
    OHID(ERR_CODE,STATUS)        = 0x0C,    // 状态错误
    OHID(ERR_CODE,CRC)           = 0x0D,    // CRC错误
    OHID(ERR_CODE,PACK)          = 0x0E,    // pack错误
    OHID(ERR_CODE,NONE)          = 0x00,    // 无错误
    /*************************** Alias ********************************/
    OHID_ERR_INVALID             = OHID(ERR_CODE,INVALID)       ,    // 'invalid' response for bad commands
    OHID_ERR_OVER_ADDR           = OHID(ERR_CODE,OVER_ADDR)     ,    // flash addr over
    OHID_ERR_OVER_SIZE           = OHID(ERR_CODE,OVER_SIZE)     ,    // flash size over
    OHID_ERR_OVER_BUFF           = OHID(ERR_CODE,OVER_BUFF)     ,    // buffer over
    OHID_ERR_ARG                 = OHID(ERR_CODE,ARG)           ,    // 参数错误
    OHID_ERR_SIGN_INVALID        = OHID(ERR_CODE,SIGN_INVALID)  ,    // 签名失效
    OHID_ERR_SIGN                = OHID(ERR_CODE,SIGN)          ,    // 签名错误
    OHID_ERR_SIGN_DATE           = OHID(ERR_CODE,SIGN_DATE)     ,    // 签名数据错误
    OHID_ERR_SIGN_SHAKE1         = OHID(ERR_CODE,SIGN_SHAKE1)   ,    // handshake1
    OHID_ERR_SIGN_SHAKE2         = OHID(ERR_CODE,SIGN_SHAKE2)   ,    // handshake2
    OHID_ERR_BUSY                = OHID(ERR_CODE,BUSY)          ,    // 设备忙
    OHID_ERR_PROGRAM             = OHID(ERR_CODE,PROGRAM)       ,    // 编程错误
    OHID_ERR_STATUS              = OHID(ERR_CODE,STATUS)        ,    // 状态错误
    OHID_ERR_CRC                 = OHID(ERR_CODE,CRC)           ,    // CRC错误
    OHID_ERR_PACK                = OHID(ERR_CODE,PACK)          ,    // pack错误
    OHID_ERR_NONE                = OHID(ERR_CODE,NONE)          ,    // 无错误
};

/******************************************************************************
 * 路由节点定义, 当主 MCU 外围仅连接了一个 MCU 时，可忽略该参数
*******************************************************************************/
enum OHID_ROUTING {
    OHID(ENDPOINT,NONE)    = 0x00,    // default
    OHID(ENDPOINT,BLE)     = 0x01,    // BLE
    OHID(ENDPOINT,MCU)     = 0x02,    // MCU
    /*************************** Alias ********************************/
    OHID_POINT_BLE         = OHID(ENDPOINT,BLE)     ,    // BLE
    OHID_POINT_MCU         = OHID(ENDPOINT,MCU)     ,    // MCU
    OHID_POINT_NONE        = OHID(ENDPOINT,NONE)    ,    // default
};

/******************************************************************************
 * 命令定义
*******************************************************************************/
enum OHID_CMD {
    /******************************* 基础指令 ***********************************/
    /*
     * 出错时返回 FAIL指令
     | S      | Pack Len    | Des  |        Head[4B]         | <Data>                             |
     | Slave  | [4B]+[2+nB] | FAIL | 0xAA | len | 0xFF | crc | Err Code[1B] | cmd[1B] | <arg>[1B] |
     */
    OHID_CMD_BASE_FAIL      = 0xFF,
    /*
     * 调试指令,用于替代串口查看设备的调试信息
     | S      | Pack Len    | Des   |        Head[4B]         | <Data>      |
     | Slave  | [4B]+[2+nB] | Debug | 0xAA | len | 0x7F | crc | <Data>[60B] |
     */
    OHID_CMD_BASE_DEBUG     = 0x7F,
    /*
     * Routing, 路由指令,用于上位机连接与主 MCU连接的辅助 MCU
     | S/M    | Pack Len    | Des     |        Head[4B]         | <Data>                   |
     | Master | [4B]+[1+nB] | Routing | 0xAA | len | 0x7E | crc | endpoint[1B] | <arg>[nB] |
     | Slave  | [4B]+[1+nB] | ACK     | 0xA5 | len | 0x7E | crc | endpoint[1B] | <arg>[nB] |
    */
    OHID_CMD_BASE_ROUTING   = 0x7E,
    /*
     * Bridging, 桥接指令,用于上位机连接与主 MCU连接的辅助 MCU
     * 与 Routing 不同的是, Routing 把需要发送的数据放在 arg 字段，接收到该指令直接将整个数据包转发到 endpoint,
     * 而 Bridging 开启后是将所有的数据包都转发到 endpoint, 直到关闭 Bridging 功能(一旦开启将无法通过 OHID 指令关闭！！！),
     * 两者都是为了实现上位机连接与主 MCU 相连的辅助 MCU(可以是协处理器, BLE, 或其它设备),
     * Routing 可以同时保持与主 MCU 和辅助 MCU 的通信，而 Bridging 一旦开启必须关闭方可与主 MCU通信, 也就是透传,
     * 关闭 Bridging 有两种方式, 1.重新初始化,如重新上电或休眠后被唤醒; 2.进行特定的 HID 操作;
     * 注: Bridging 开启后主 MCU 将不再判断上位机发送过来的数据,全部进行转发,也就是可以用来转发任意数据,
     | S/M    | Pack Len  | Des     |        Head[4B]         | <Data>       |
     | Master | [4B]+[1B] | Bridging| 0xAA | len | 0x7D | crc | endpoint[1B] |
     | Slave  | [4B]+[1B] | ACK     | 0xA5 | len | 0x7D | crc | endpoint[1B] |
    */
    OHID_CMD_BASE_BRIDGING   = 0x7D,
    /*
     * 混杂指令,用于执行一些杂项操作
     | S/M    | Pack Len    | Des |        Head[4B]         | <Data>                |
     | Master | [4B]+[1+nB] | CMD | 0xAA | len | 0x00 | crc | order[1B] | <arg>[nB] |
     | Slave  | [4B]+[1+nB] | ACK | 0xA5 | len | 0x00 | crc | order[1B] | <arg>[nB] |
    */
    OHID_CMD_BASE_MIX       = 0x00,
    /*
     * 同步指令,用于获取设备的基础信息,通常为 Master 给 Slave 发送的第一个指令
     | S/M    | Pack Len   | Des  |        Head[4B]         | <Data>                                                             |
     | Master | [4B]+[ 0B] | SYNC | 0xAA | len | 0x01 | crc |                                                                    |
     | Slave  | [4B]+[41B] | ACK  | 0xA5 | len | 0x01 | crc | Board ID[4B] | fw_size[2B] | run_mode[1B] | SN[17B] | version[17B] |
    */
    OHID_CMD_BASE_SYNC      = 0x01,
    /*
     * 安全指令, 用于写入自定义序列号与加密信息, 该指令写操作需要解锁
     | S/M    | Pack Len   | Des  |        Head[4B]         | <Data>                  |
     | Master | [4B]+[52B] | SAFE | 0xAA | len | 0x02 | crc | SN[20] | Encryption[32] |
     | Slave  | [4B]+[52B] | ACK  | 0xA5 | len | 0x02 | crc | SN[20] | Encryption[32] |
    */
    OHID_CMD_BASE_SAFE      = 0x02,
    /*
     * 扩展指令, 用于添加自定义通讯
     | S/M    | Pack Len   | Des       |        Head[4B]         | <Data>    |
     | Master | [4B]+[ nB] | EXPANDING | 0xAA | len | 0x03 | crc | <arg>[nB] |
     | Slave  | [4B]+[ nB] | ACK       | 0xA5 | len | 0x03 | crc | <arg>[nB] |
    */
    OHID_CMD_BASE_EXPANDING = 0x03,
    /******************************* BLE 指令 ***********************************/
    /*
     * BLE 模组指令
     | S/M    | Pack Len    | Des  |        Head[4B]         | <Data>    |
     | Master | [4B]+[2+nB] | BLE  | 0xAA | len | 0x06 | crc | Bitmap[1B]| order[1B]| <arg>[nB]  |
     | Slave  | [4B]+[ 5B]  | ACK  | 0xA5 | len | 0x06 | crc | Bitmap[1B]| Leds[1B] | Pair[1B]   | Voltage[2B]|
    */
    OHID_CMD_BLE       = 0x06,
    /*
     * BLE上报指令
     | S/M    | Pack Len    | Des  |        Head[4B]         | <Data>    |
     | Master | [4B]+[17B]  | BRP  | 0xAA | len | 0x07 | crc | Bitmap[1B]| Key[16B] |
     | Slave  | [4B]+[ 5B]  | ACK  | 0xA5 | len | 0x06 | crc | Bitmap[1B]| Leds[1B] | Pair[1B]   | Voltage[2B]|
    */
    OHID_CMD_BRP       = 0x07,
    /******************************* IAP 指令 ***********************************/
    /*
     * signature, 签名,用于鉴别是否合法操作
     | S/M    | Pack Len   | Des  |        Head[4B]         | <Data>                               |
     | Master | [4B]+[33B] | SIGN | 0xAA | len | 0x08 | crc | UnLock[1B] | SIGN[16B]  | Datas[16B] |
     | Slave  | [4B]+[17B] | ACK  | 0xA5 | len | 0x08 | crc | UnLock[1B] | Datas[16B] |
    */
    OHID_CMD_IAP_SIGN   = 0x08,
    /*
     * 擦除
     | S/M    | Pack Len   | Des   |        Head[4B]         | <Data>                  |
     | Master | [4B]+[ 8B] | ERASE | 0xAA | len | 0x09 | crc | address[4B]  | Size[4B] |
     | Slave  | [4B]+[ 1B] | ACK   | 0xA5 | len | 0x09 | crc | progress[1B] |
    */
    OHID_CMD_IAP_ERASE  = 0x09,
    /*
     * 重启
     | S/M    | Pack Len   | Des    |        Head[4B]         | <Data>     |
     | Master | [4B]+[ 1B] | REBOOT | 0xAA | len | 0x0A | crc | <rand>[1B] |
     | Slave  | [4B]+[ 1B] | ACK    | 0xA5 | len | 0x0A | crc | <rand>[1B] |
    */
    OHID_CMD_IAP_REBOOT = 0x0A,
    /*
     * Jump app
     | S/M    | Pack Len   | Des  |        Head[4B]         | <Data>                            |
     | Master | [4B]+[10B] | JUMP | 0xAA | len | 0x0B | crc | address[4B] | Size[4B]  | CRC[4B] |
     | Slave  | [4B]+[10B] | ACK  | 0xA5 | len | 0x0B | crc | address[4B] | Size[4B]  | CRC[4B] |
    */
    OHID_CMD_IAP_JUMP   = 0x0B,
    /*
     * 编程指令,刷写固件或读取固件, wSize 需 4字节对齐
     | S/M    | Pack Len    | Des     |        Head[4B]         | <Data>                             |
     | Master | [4B]+[6+nB] | PROGRAM | 0xAA | len | 0x0C | crc | address[4B] | wSize[2B] | code[nB] |
     | Slave  | [4B]+[6+nB] | ACK     | 0xA5 | len | 0x0C | crc | address[4B] | wSize[2B] | code[nB] |
    */
    OHID_CMD_IAP_PROGRAM= 0x0C,
    /*
     * 获取 CRC
     | S/M    | Pack Len   | Des  |        Head[4B]         | <Data>                            |
     | Master | [4B]+[ 8B] | RCRC | 0xAA | len | 0x0D | crc | address[4B] | Size[4B]  |
     | Slave  | [4B]+[10B] | ACK  | 0xA5 | len | 0x0D | crc | address[4B] | Size[4B]  | CRC[4B] |
    */
    OHID_CMD_IAP_RCRC   = 0x0D,
    /*
     * 下载操作, 需要先设置下载的地址，用该指令更新固件相比 PROGRAM 效率提升约 15%，其地址是自增的
     | S/M    | Pack Len  | Des      |        Head[4B]         | <Data>   |
     | Master | [4B]+[nB] | DOWNLOAD | 0xAA | len | 0x0E | crc | code[nB] |
     | Slave  | [4B]+[nB] | ACK      | 0xA5 | len | 0x0E | crc | code[nB] |
    */
    OHID_CMD_IAP_DOWNLOAD= 0x0E,
    /******************************* 驱动指令 ***********************************/
    /*
     * Param
     * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
     | S/M    | Pack Len     | Des   |        Head[4B]         | <Data>                                    |
     | Master | [4B]+[12x5B] | PARAM | 0xAA | len | 0x10 | crc | MKEY[2B]  | Item[1B]  | Value [2B] | ...  | 0xFF 无效
     | Slave  | [4B]+[12x5B] | ACK   | 0xA5 | len | 0x10 | crc | MKEY[2B]  | Item[1B]  | Value [2B] | ...  | 0xFF 无效
    */
    OHID_CMD_DRIVER_PARAM       = 0x10,
    /*
     * MT 功能设置, Reference Wooting
     * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
     | S/M    | Pack Len    | Des  |        Head[4B]         | <Data>                                               |
     | Master | [4B]+[7x8B] | MT   | 0xAA | len | 0x11 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | delay[2B] | ...  | 0xFF 无效
     | Slave  | [4B]+[7x8B] | ACK  | 0xA5 | len | 0x11 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | delay[2B] | ...  | 0xFF 无效
    */
    OHID_CMD_DRIVER_MT        = 0x11,
    /*
     * TGL 功能设置, Reference Wooting
     * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
     | S/M    | Pack Len     | Des  |        Head[4B]         | <Data>                                   |
     | Master | [4B]+[10x6B] | TGL  | 0xAA | len | 0x12 | crc | MKEY[2B]  | KEY1[2B]  | delay[2B] | ...  | 0xFF 无效
     | Slave  | [4B]+[10x6B] | ACK  | 0xA5 | len | 0x12 | crc | MKEY[2B]  | KEY1[2B]  | delay[2B] | ...  | 0xFF 无效
    */
    OHID_CMD_DRIVER_TGL       = 0x12,
    /*
     * DKS(Digital keying system)功能设置, Reference Wooting
     * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
     | S/M    | Pack Len     | Des  |        Head[4B]         | <Data>                                                                                                                     |
     | Master | [4B]+[4x15B] | DKS  | 0xAA | len | 0x13 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | KEY3[2B]  | KEY4[2B]  | TrPs1[1B]  | TrPs2[1B]  | TrPs3[1B]  | TrPs4[1B]  | mm1 | ...  | 0xFF 无效
     | Slave  | [4B]+[4x15B] | ACK  | 0xA5 | len | 0x13 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | KEY3[2B]  | KEY4[2B]  | TrPs1[1B]  | TrPs2[1B]  | TrPs3[1B]  | TrPs4[1B]  | mm1 | ...  | 0xFF 无效
    */
    OHID_CMD_DRIVER_DKS       = 0x13,
    /*
     * AKS(Analog keying system)功能设置, Reference 海盗船
     * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
     | S/M    | Pack Len     | Des  |        Head[4B]         | <Data1>                                                                        |<Datan>|
     | Master | [4B]+[5x11B] | AKS3 | 0xAA | len | 0x14 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | KEY3[2B]  | mm1[1B]  | mm2[1B]  | mm3[1B]  | ...   | 0xFF 无效
     | Slave  | [4B]+[5x11B] | ACK  | 0xA5 | len | 0x14 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | KEY3[2B]  | mm1[1B]  | mm2[1B]  | mm3[1B]  | ...   | 0xFF 无效
    */
    OHID_CMD_DRIVER_AKS3      = 0x14,
    /*
     * KEY,按键设置通用指令,会自动根据按键的模式设置合适的数据格式,该指令最大的用处在于驱动启动时获取按键的数据会很简单，
     * 一般的方法是根据按键的模式参数调佣不同的指令获取按键的数据，但该指令提供了一种通用的方法；
     | S/M    | Pack Len     | Des |        Head[4B]         | <Data1>             | <Data2>             | <Data3>             | <Data4>             |
     | Master | [4B]+[4x15B] | KEY | 0xAA | len | 0x15 | crc | MKEY1[2B] | D1[13B] | MKEY2[2B] | D2[13B] | MKEY3[2B] | D3[13B] | MKEY4[2B] | D4[13B] |
     | Slave  | [4B]+[4x15B] | ACK | 0xA5 | len | 0x15 | crc | MKEY1[2B] | D1[13B] | MKEY2[2B] | D2[13B] | MKEY3[2B] | D3[13B] | MKEY4[2B] | D4[13B] |
    */
    OHID_CMD_DRIVER_KEY       = 0x15,
    /*
     * RGB_PARAM 设置
     | S/M    | Pack Len   | Des     |        Head[4B]         | <Data>                                                                                                         |
     | Master | [4B]+[43B] | PARAM   | 0xAA | len | 0x16 | crc | BACK[4B]  | Palettes[8*4B] | Gray[1B] | MODE[1B] | SPEED[1B] | SLEEP[1B] | ON[1B] | ON_SLEEP[1B] | REVERSE[1B] |
     | Slave  | [4B]+[43B] | ACK     | 0xA5 | len | 0x16 | crc | BACK[4B]  | Palettes[8*4B] | Gray[1B] | MODE[1B] | SPEED[1B] | SLEEP[1B] | ON[1B] | ON_SLEEP[1B] | REVERSE[1B] |
    */
    OHID_CMD_DRIVER_RGB_PARAM    = 0x16,
    /*
     * RGB 设置, 根据 RGB 对应的按键的键值识别, 此方法使用的时候更方便, 但如果一个 RGB 没有对应的按键将不可设置
     * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
     | S/M    | Pack Len     | Des  |        Head[4B]         | <Data1>                                          |<Datan>|
     | Master | [4B]+[12x5B] | KRGB | 0xAA | len | 0x17 | crc | MKEY[2B]  | RGB.R[1B]  | RGB.G[1B]  | RGB.B[1B]  | ...   | 0xFF 无效
     | Slave  | [4B]+[12x5B] | ACK  | 0xA5 | len | 0x17 | crc | MKEY[2B]  | RGB.R[1B]  | RGB.G[1B]  | RGB.B[1B]  | ...   | 0xFF 无效
    */
    OHID_CMD_DRIVER_KRGB      = 0x17,
    /*
     * RGB 设置, 根据 RGB 坐标识别, 此方法的通用性更高, IDX<128 为按键灯 (Matrix6*21) , IDX>=128 为侧灯
     | S/M    | Pack Len     | Des  |        Head[4B]         | <Data1>                                         |<Datan>|
     | Master | [4B]+[15x4B] | IRGB | 0xAA | len | 0x18 | crc | IDX[1B]  | RGB.R[1B]  | RGB.G[1B]  | RGB.B[1B]  | ...   | 0xFF 无效
     | Slave  | [4B]+[15x4B] | ACK  | 0xA5 | len | 0x18 | crc | IDX[1B]  | RGB.R[1B]  | RGB.G[1B]  | RGB.B[1B]  | ...   | 0xFF 无效
    */
    OHID_CMD_DRIVER_IRGB      = 0x18,
    /*
     * 宏序列设置
     | S/M    | Pack Len     | Des       |        Head[4B]         | <Data>                         |
     | Master | [4B]+[3+4nB] | MACROLIST | 0xAA | len | 0x19 | crc | offset[2B] | len[1B] | bin[nW] |
     | Slave  | [4B]+[3+4nB] | ACK       | 0xA5 | len | 0x19 | crc | offset[2B] | len[1B] | bin[nW] |
    */
    OHID_CMD_DRIVER_MACROLIST = 0x19,
    /*
     * 按键数据上报
     | S/M    | Pack Len     | Des   |        Head[4B]         | <Data>               |
   --| Master | [4B]+[3+4nB] | RP    | 0xAA | len | 0x1A | crc | ON  [1B] | Delay[2B] |
     | Slave  | [4B]+[1+59B] | ACK   | 0xA5 | len | 0x1A | crc | TYPE[1B] | bin [59B] |
    */
    OHID_CMD_DRIVER_RP        = 0x1A,
    /*
     * curve, 补偿曲线
     | S/M    | Pack Len     | Des   |        Head[4B]         | <Data>                                              |
     | Master | [4B]+[4+56B] | CURVE | 0xAA | len | 0x1B | crc | idx[1B] | len[1B] | offset[2B] | Point1[2B] | ... | Point29[2B] |
     | Slave  | [4B]+[4+56B] | ACK   | 0xA5 | len | 0x1B | crc | idx[1B] | len[1B] | offset[2B] | Point1[2B] | ... | Point29[2B] |
    */
    OHID_CMD_DRIVER_CURVE     = 0x1B,
    /*
     * NORMAL, 普通触发
     | S/M    | Pack Len   | Des    |        Head[4B]         | <Data>                       |
     | Master | [4B]+[2nB] | NORMAL | 0xAA | len | 0x1C | crc | MKEY1[2B] | ... | MKEY30[2B] |
     | Slave  | [4B]+[2nB] | ACK    | 0xA5 | len | 0x1C | crc | MKEY1[2B] | ... | MKEY30[2B] |
    */
    OHID_CMD_DRIVER_NORMAL    = 0x1C,
    /*
     * SINGLE, 单键触发
     | S/M    | Pack Len     | Des    |        Head[4B]         | <Data>                        |
     | Master | [4B]+[15x4B] | SINGLE | 0xAA | len | 0x1D | crc | MKEY[2B] | MM[1B] | Reset[1B] |
     | Slave  | [4B]+[15x4B] | ACK    | 0xA5 | len | 0x1D | crc | MKEY[2B] | MM[1B] | Reset[1B] |
    */
    OHID_CMD_DRIVER_SINGLE    = 0x1D,
    /*
     * RT, 快速触发
     | S/M    | Pack Len     | Des   |        Head[4B]         | <Data>                                   |
     | Master | [4B]+[12x5B] | RT    | 0xAA | len | 0x1E | crc | MKEY[2B] | MM[1B] | RTmm[1B] | Reset[1B] |
     | Slave  | [4B]+[12x5B] | ACK   | 0xA5 | len | 0x1E | crc | MKEY[2B] | MM[1B] | RTmm[1B] | Reset[1B] |
    */
    OHID_CMD_DRIVER_RT        = 0x1E,
    /*
     * DT, 增程触发
     | S/M    | Pack Len     | Des   |        Head[4B]         | <Data>                            |
     | Master | [4B]+[15x4B] | DT    | 0xAA | len | 0x1F | crc | MKEY[2B] | DTmm[1B] | DTreset[1B] |
     | Slave  | [4B]+[15x4B] | ACK   | 0xA5 | len | 0x1F | crc | MKEY[2B] | DTmm[1B] | DTreset[1B] |
    */
    OHID_CMD_DRIVER_DT        = 0x1F,
    /*
     * MACRO, 宏指令
     | S/M    | Pack Len     | Des   |        Head[4B]         | <Data>                         |
     | Master | [4B]+[10x6B] | MACRO | 0xAA | len | 0x20 | crc | MKEY[2B] | Addr[2B] | Long[2B] |
     | Slave  | [4B]+[10x6B] | ACK   | 0xA5 | len | 0x20 | crc | MKEY[2B] | Addr[2B] | Long[2B] |
    */
    OHID_CMD_DRIVER_MACRO     = 0x20,
    /*
     * APPEND, 追加
     | S/M    | Pack Len      | Des    |        Head[4B]         | <Data>                |
     | Master | [4B]+[15x4B]  | APPEND | 0xAA | len | 0x21 | crc | MKEY[2B] | NewKEY[2B] |
     | Slave  | [4B]+[15x4B]  | ACK    | 0xA5 | len | 0x21 | crc | MKEY[2B] | NewKEY[2B] |
    */
    OHID_CMD_DRIVER_APPEND    = 0x21,
    /*
     * MPT(Macro Partition Table), 宏分区指令, 该内容和 OHID_CMD_DRIVER_MACROLIST 是一个整体
     * MacroID: 0xFFFF, 无效
     | S/M    | Pack Len      | Des   |        Head[4B]         | <Data>     宏ID       宏地址      宏长度                                                          |
     | Master | [4B]+[2+9x6B] | MPT   | 0xAA | len | 0x22 | crc | Page[2B] | MID1[2B] | Addr1[2B] | Long1[2B] | MID2[2B] | Addr2[2B] | Long2[2B] |
     | Slave  | [4B]+[2+9x6B] | ACK   | 0xA5 | len | 0x22 | crc | Page[2B] | MID1[2B] | Addr1[2B] | Long1[2B] | MID2[2B] | Addr2[2B] | Long2[2B] |
    */
    OHID_CMD_DRIVER_MACRO_PT  = 0x22,
    /*
     * QUICK, 快速触发,快速触发模式下仅上报最后一次按下的按键
     | S/M    | Pack Len     | Des    |        Head[4B]         | <Data>                                                |
     | Master | [4B]+[20x3B] | QUICK  | 0xAA | len | 0x23 | crc | MKEY1[2B] | Mode1[1B] | ... | MKEY20[2B] | Mode20[1B] |
     | Slave  | [4B]+[20x3B] | ACK    | 0xA5 | len | 0x23 | crc | MKEY1[2B] | Mode1[1B] | ... | MKEY20[2B] | Mode20[1B] |
    */
    OHID_CMD_DRIVER_QUICK     = 0x23,
    /*
     * GETMODE(Get Mode), 获取按键的模式参数,只读,
     | S/M    | Pack Len      | Des     |        Head[4B]         | <Data>                |
     | Master | [4B]+[20x3B]  | GETMODE | 0xAA | len | 0x24 | crc | MKEY[2B] | Mode[1B] |
     | Slave  | [4B]+[20x3B]  | ACK     | 0xA5 | len | 0x24 | crc | MKEY[2B] | Mode[1B] |
    */
    //OHID_CMD_DRIVER_GETMODE   = 0x24,
    /*
     * SOCD, SOCD 模式只上报按下最深的按键
     | S/M    | Pack Len     | Des    |        Head[4B]         | <Data>                                                                                                |
     | Master | [4B]+[12x5B] | SOCD   | 0xAA | len | 0x25 | crc | MM1[1B] | Reset1[1B] | MKEY1[2B] | Mode1[1B] | ... | MM19[1B] | Reset19[1B] | MKEY19[2B] | Mode19[1B] |
     | Slave  | [4B]+[12x5B] | ACK    | 0xA5 | len | 0x25 | crc | MM1[1B] | Reset1[1B] | MKEY1[2B] | Mode1[1B] | ... | MM19[1B] | Reset19[1B] | MKEY19[2B] | Mode29[1B] |
    */
    OHID_CMD_DRIVER_SOCD     = 0x25,
};

// 单个数据包最大长度 64*4 B
#define   OHID_HEAD_SIZE    (4)
#define   OHID_PORT_SIZE    (64*4)
#define   OHID_DATA_SIZE    (OHID_PORT_SIZE-OHID_HEAD_SIZE)
union OpenAgreementHID_t {
    uint32_t word[OHID_PORT_SIZE/4];  // 使用 uint32_t 便于编译器进行 4 字节对齐
    uint8_t bin[OHID_PORT_SIZE];
    struct {
        uint8_t HEAD;
        uint8_t length;
        uint8_t cmd;
        uint8_t checksum;
        uint8_t data[OHID_DATA_SIZE];
    }ohid;
};
/******************************************************************************
** 数据结构定义, 使用联合体将数据处理为小端格式
*******************************************************************************/
union OHID_uint16_t {
    uint16_t data;
    struct {
        uint8_t Byte0;
        uint8_t Byte1;
    }ohid;
};
union OHID_uint32_t {
    uint32_t data;
    struct {
        uint8_t Byte0;
        uint8_t Byte1;
        uint8_t Byte2;
        uint8_t Byte3;
    }ohid;
};

/******************************************************************************
** 协议接口函数
*******************************************************************************/
#define OHID_pack_size(Pack)    (Pack.ohid.length+OHID_HEAD_SIZE)
#define OHID_ppack_size(Pack)   (Pack->ohid.length+OHID_HEAD_SIZE)
extern void OHIDM_init(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd);
extern void OHIDS_init(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd);
extern int OHID_copy(uint8_t bin[], const uint8_t src[], const uint16_t len);
extern uint8_t OHID_Checksum(const union OpenAgreementHID_t* const Pack);
extern int OHIDM_encode(union OpenAgreementHID_t* const Pack);
extern int OHIDS_encode(union OpenAgreementHID_t* const Pack);
extern int _OHIDM_Decode(union OpenAgreementHID_t* const Pack, const uint16_t bSize);
extern int _OHIDS_Decode(union OpenAgreementHID_t* const Pack, const uint16_t bSize);
extern int _OHIDM_decode(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint16_t bSize);
extern int _OHIDS_decode(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint16_t bSize);
extern int OHIDM_decode(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint16_t bSize);
extern int OHIDS_decode(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint16_t bSize);
/******************************************************************************
** 数据接口函数
*******************************************************************************/
// 添加一个 32bit无符号数
extern int OHID_add_32b(union OpenAgreementHID_t* const Pack, const uint32_t value);
extern int OHID_add_16b(union OpenAgreementHID_t* const Pack, const uint16_t value);
extern int OHID_add_8b(union OpenAgreementHID_t* const Pack, const uint8_t value);
// 添加一串二进制数据到指令结尾处
extern int OHID_add_bin(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint8_t blen);
// 获取一个 32bit无符号数
extern uint32_t OHID_get_32b(const union OpenAgreementHID_t* const Pack, const uint8_t pos);
extern uint16_t OHID_get_16b(const union OpenAgreementHID_t* const Pack, const uint8_t pos);
extern uint8_t  OHID_get_8b(const union OpenAgreementHID_t* const Pack, const uint8_t pos);
// 获取一串二进制数据
extern int OHID_get_bin(const union OpenAgreementHID_t* const Pack, const uint8_t pos, uint8_t bin[], const uint16_t bsize);

// 键盘接收到无效指令或者指令执行出错会返回 FAIL；
extern void OHIDS_fail(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd, const enum OHID_ERR_CODE code);
extern void OHIDS_fail_arg2(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd, const enum OHID_ERR_CODE code, const uint8_t arg1, const uint8_t arg2);
//static inline int OHID_get_fail(union OpenAgreementHID_t* const Pack, enum OHID_CMD* const cmd, enum OHID_ERR_CODE* const code)
//{
//    *cmd  = (enum OHID_CMD)OHID_get_8b(Pack, 0);
//    *code = (enum OHID_ERR_CODE)OHID_get_8b(Pack, 1);
//    return 0;
//}
extern void OHIDS_Debug(union OpenAgreementHID_t* const Pack, const uint8_t str[], uint8_t len);
#define OHID_get_fail_cmd(Pack)    ((enum OHID_CMD)OHID_get_8b(Pack, 0))
#define OHID_get_fail_code(Pack)   ((enum OHID_ERR_CODE)OHID_get_8b(Pack, 1))

#ifdef __cplusplus
}
#endif

#endif // _OHID_PORT_H_

/************************** (C) COPYRIGHT Merafour **************************/
