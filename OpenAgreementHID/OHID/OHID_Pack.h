/******************** (C) COPYRIGHT 2018 merafour ********************
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.1.0
* Date               : 2024.04.07
* Description        : OpenAgreementHID.
* Description        : 开源键盘通信协议 API 接口.
* Description        : 这些接口对数据包进行封装,可以让对协议的操作更简单
* Description        : 注：
* Description        : OHIDM_* : 主机(电脑上位机)发送给从机的数据包
* Description        : OHIDS_* : 从机(键盘)发送给主机的数据包
* Description        : 如无必要,勿增实体.
********************************************************************************
* SAFE, ERASE, REBOOT, BOOT, WRITE, READ, 6个关键指令均需要解锁相应的安全等级,以确保设备的安全性
* 解锁需要二次握手, 且每种解锁操作都对应一组签名验证：
* 第一次握手:
*   1. 解锁指令 SIGN， 发送 SN 号 (SYNC返回的SN号) 和签名, 设备验证通过将返回一串随机数 Datas 用于第二次握手;
* 第二次握手:
*   2. 根据键盘返回的随机数 Datas 进行签名, 需要解锁的操作不同，签名算法也不相同 ;
*   3. 解锁指令 ERASE(eg.)，发送 Datas和签名, 设备验证失败将返回错误码 ;
*   4. 设备解锁安全操作成功, 安全操作窗口期为 120s, 超时后关闭安全窗口, 需重新解锁方可操作, 否则将返回错误码 SIGN_INVALID;
********************************************************************************
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#ifndef _OHID_PACK_H_
#define _OHID_PACK_H_

#include <stdint.h>
#include <OHID/OHID_Port.h>
#include <OHID/OHID_Board.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* BOOT 指令解锁操作,解锁操作需要满足特定的解锁校验算法,算法校验不通过则解锁失败,BOOT指令的相关操作也将失败,
* 该设计用于禁止不被授权的刷机操作
*******************************************************************************/
enum OHID_UNLOCK {
    OHID(UNLOCK,NONE)          = 0x00,   // 无效
    OHID(UNLOCK,SIGN)          = 0x01,   // 签名
    OHID(UNLOCK,ERASE)         = 0x02,   // 解锁擦除
    OHID(UNLOCK,REBOOT)        = 0x03,   // 解锁重启
    OHID(UNLOCK,BOOT)          = 0x04,   // 解锁跳转
    OHID(UNLOCK,PROGRAM)       = 0x05,   // 解锁编程操作
    OHID(UNLOCK,SAFE)          = 0x06,   // 解锁SN(加密)写操作
	OHID(UNLOCK,PROD_LINE)     = 0x07,   // 解锁生产线模式
	OHID_UNLOCK_MAX            = 0x07,   // 最大值
	/*************************** Alias ********************************/
    UNLOCK_NONE                = OHID(UNLOCK,NONE)          ,   // 无效
    UNLOCK_SIGN                = OHID(UNLOCK,SIGN)          ,   // 签名
    UNLOCK_ERASE               = OHID(UNLOCK,ERASE)         ,   // 解锁擦除
    UNLOCK_REBOOT              = OHID(UNLOCK,REBOOT)        ,   // 解锁重启
    UNLOCK_BOOT                = OHID(UNLOCK,BOOT)          ,   // 解锁跳转
    UNLOCK_PROGRAM             = OHID(UNLOCK,PROGRAM)       ,   // 解锁编程操作
    UNLOCK_SAFE                = OHID(UNLOCK,SAFE)          ,   // 解锁SN(加密)写操作
	UNLOCK_PROD_LINE           = OHID(UNLOCK,PROD_LINE)     ,   // 解锁生产线模式
};
/******************************************************************************
* 设备工作模式, 用于区分 BOOT 和固件
*******************************************************************************/
enum OHID_RUN_MODE {
    OHID(RUN_MODE,BOOT)           = 0x00,    // BOOT mode
    OHID(RUN_MODE,FW)             = 0x01,    // FW   mode
	/*************************** Alias ********************************/
    OHID_RUN_BOOT                 = OHID(RUN_MODE,BOOT),    // BOOT mode
    OHID_RUN_FW                   = OHID(RUN_MODE,FW)  ,    // FW   mode
};

extern void OHIDM_None(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd);
extern void OHIDS_None(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd);
/*
 * Routing, 路由指令,用于上位机连接与主 MCU连接的辅助 MCU
 | S/M    | Pack Len    | Des     |        Head[4B]         | <Data>                   |
 | Master | [4B]+[1+nB] | Routing | 0xAA | len | 0x7E | crc | endpoint[1B] | <arg>[nB] |
 | Slave  | [4B]+[1+nB] | ACK     | 0xA5 | len | 0x7E | crc | endpoint[1B] | <arg>[nB] |
*/
extern void OHIDM_Routing(union OpenAgreementHID_t* const Pack, const enum OHID_ROUTING endpoint, const uint8_t arg[], const uint8_t len);
extern void OHIDS_Routing(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_ROUTING endpoint, const uint8_t arg[], const uint8_t len);
extern void OHIDM_Bridging(union OpenAgreementHID_t* const Pack, const enum OHID_ROUTING endpoint);
extern void OHIDS_Bridging(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_ROUTING endpoint);
/******************************* 基础指令 ***********************************/
extern void OHIDS_Base_sync(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_BOARDS_ID_LIST board_id, const uint16_t fw_size, const enum OHID_RUN_MODE run_mode, const char SN[16], const char version[16]);
extern void OHIDM_Base_SAFE(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint8_t SN[20], const uint8_t Encryption[32]);
extern void OHIDS_Base_SAFE(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t SN[20], const uint8_t Encryption[32]);

/******************************* IAP 指令 ***********************************/
// 秘钥
extern const uint8_t OHID_signature_key_main[16];
// 签名参数
extern const uint8_t OHID_signature_params1[16];
extern const uint8_t OHID_signature_params2[16];
extern uint32_t ohid_crc32(const uint8_t *src, const uint32_t len, uint32_t state);
extern void OHID_Signature(const uint8_t Datas[16], const uint8_t KEYS[16], const uint8_t params[16], const uint8_t unlock, uint8_t SIGN[16]);
// signature
extern void OHIDM_IAP_Sign(union OpenAgreementHID_t* const Pack, const enum OHID_UNLOCK unlock, const uint8_t SIGN[16], const uint8_t Datas[16]);
extern void OHIDS_IAP_Sign(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_UNLOCK unlock, const uint8_t Datas[16]);
extern void OHIDM_IAP_Erase(union OpenAgreementHID_t* const Pack, const uint32_t address, const uint32_t eSize);
extern void OHIDS_IAP_Erase(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t progress);
extern void OHIDM_IAP_Reboot(union OpenAgreementHID_t* const Pack, const uint8_t rand);
extern void OHIDS_IAP_Reboot(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t rand);
extern void OHIDM_IAP_Jump(union OpenAgreementHID_t* const Pack, const uint32_t address, const uint32_t Size, const uint32_t CRCin);
extern void OHIDS_IAP_Jump(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t address, const uint32_t Size, const uint32_t CRCin);
extern void OHIDM_IAP_Program(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint32_t address, const uint16_t Size, const uint8_t binary[]);
extern void OHIDS_IAP_Program(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t address, const uint16_t Size, const uint8_t binary[]);
extern void OHIDM_IAP_RCRC(union OpenAgreementHID_t* const Pack, const uint32_t address, const uint32_t Size);
extern void OHIDS_IAP_RCRC(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t address, const uint32_t Size, const uint32_t CRCin);

#ifdef __cplusplus
}
#endif

#endif // _OHID_PACK_H_

/************************** (C) COPYRIGHT Merafour **************************/
