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
********************************************************************************
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#include <string.h>
#include "OHID/OHID_Pack.h"
#include "OHID/OHID_KeyBoard.h"

/*
 * 数据包格式:
 * | S/M    | Pack Len   | Des |        Head[4B]        | <Data> |
 * | Master | [4B]+[ nB] | CMD | 0xAA | len | cmd | crc | [ nB]  |
 * | Slave  | [4B]+[ nB] | ACK | 0xA5 | len | cmd | crc | [ nB]  |
 * head : 协议头;
 * len  : data长度;
 * CMD  : 命令编码;
 * CRC  : 累加和校验, 计算方式为:crc=head+len+cmd+<data.end>+1+2+3+…+(len+3), 具体参考 OHID_Checksum 函数;
 * data : 数据域, 最大长度为 64*4-4 B ;
 */
 void OHIDM_Base_mix(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
    OHID_add_8b(Pack, order);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_Base_mix_bin(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t bin[59], uint8_t len)
{
    uint8_t cnt;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
    OHID_add_8b(Pack, order);
    if(len>59) len=59;  // 64-4-1 = 59
    for(cnt=0; cnt<len; cnt++) OHID_add_8b(Pack, bin[cnt]);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_Base_mix_word(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint32_t word[], uint8_t len)
{
    uint8_t cnt;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
    OHID_add_8b(Pack, order);
    if(len>14) len=14;  // (64-4-1)/4 = 59/4 = 14
    for(cnt=0; cnt<len; cnt++) OHID_add_32b(Pack, word[cnt]);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_Base_mix_rgb_test(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t srart, const uint32_t color)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
	OHID_add_8b(Pack, order);
	OHID_add_8b(Pack, srart);
	OHID_add_32b(Pack, color);
	Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_Base_mix_key_test(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t srart, const uint16_t delay, const uint8_t tick)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
	OHID_add_8b(Pack, order);
	OHID_add_8b(Pack, srart);
	OHID_add_16b(Pack, delay);
	OHID_add_8b(Pack, tick);
	Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_Base_mix_arg(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t arg)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
    OHID_add_8b(Pack, order);
    OHID_add_8b(Pack, arg);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_Base_mix_arg2(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t arg1, const uint8_t arg2)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
    OHID_add_8b(Pack, order);
    OHID_add_8b(Pack, arg1);
	OHID_add_8b(Pack, arg2);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_Base_mix_arg2B(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t arg1, const uint16_t arg2)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
    OHID_add_8b(Pack, order);
    OHID_add_8b(Pack, arg1);
    OHID_add_16b(Pack, arg2);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_Base_mix_product_sn(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_MIX_ORDER order, const uint8_t SN[4*6], const uint8_t SIGN[16])
{
    uint16_t cnt;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_MIX));
    else      OHIDM_init(Pack, OHID_CMD_BASE_MIX);
    OHID_add_8b(Pack, order);
    for(cnt=0; cnt<24; cnt++) OHID_add_8b(Pack, SN[cnt]);
    for(cnt=0; cnt<16; cnt++) OHID_add_8b(Pack, SIGN[cnt]);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Base_mix_product_sn(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t SN[4*6], const uint8_t SIGN[16])
{
    uint16_t cnt;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_BASE_MIX);
        OHID_add_8b(Pack, order);
        for(cnt=0; cnt<24; cnt++) OHID_add_8b(Pack, SN[cnt]);
        for(cnt=0; cnt<16; cnt++) OHID_add_8b(Pack, SIGN[cnt]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDS_Base_mix(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
	{
		OHIDS_init(Pack, OHID_CMD_BASE_MIX);
		OHID_add_8b(Pack, order);
		Pack->ohid.checksum = OHID_Checksum(Pack);
	}
}
extern void OHIDS_Base_mix_bin(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t bin[59], uint8_t len)
{
    uint8_t cnt;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
	{
		OHIDS_init(Pack, OHID_CMD_BASE_MIX);
		OHID_add_8b(Pack, order);
        if(len>59) len=59;  // 64-4-1 = 59
        for(cnt=0; cnt<len; cnt++) OHID_add_8b(Pack, bin[cnt]);
		Pack->ohid.checksum = OHID_Checksum(Pack);
	}
}
void OHIDS_Base_mix_word(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint32_t word[14], uint8_t len)
{
    uint8_t cnt;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_BASE_MIX);
        OHID_add_8b(Pack, order);
        if(len>14) len=14;  // (64-4-1)/4 = 59/4 = 14
        for(cnt=0; cnt<len; cnt++) OHID_add_32b(Pack, word[cnt]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDS_Base_mix_rgb_test(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t srart, const uint32_t color)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_BASE_MIX);
        OHID_add_8b(Pack, order);
		OHID_add_8b(Pack, srart);
		OHID_add_32b(Pack, color);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDS_Base_mix_key_test(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t srart, const uint16_t delay, const uint8_t tick)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_BASE_MIX);
        OHID_add_8b(Pack, order);
		OHID_add_8b(Pack, srart);
		OHID_add_16b(Pack, delay);
		OHID_add_8b(Pack, tick);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDS_Base_mix_arg(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t arg)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
	{
		OHIDS_init(Pack, OHID_CMD_BASE_MIX);
		OHID_add_8b(Pack, order);
		OHID_add_8b(Pack, arg);
		Pack->ohid.checksum = OHID_Checksum(Pack);
	}
}
void OHIDS_Base_mix_arg2(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t arg1, const uint8_t arg2)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
	{
		OHIDS_init(Pack, OHID_CMD_BASE_MIX);
		OHID_add_8b(Pack, order);
		OHID_add_8b(Pack, arg1);
		OHID_add_8b(Pack, arg2);
		Pack->ohid.checksum = OHID_Checksum(Pack);
	}
}
void OHIDS_Base_mix_arg3(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_MIX_ORDER order, const uint8_t arg1, const uint8_t arg2, const uint8_t arg3)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_MIX, code);
    }
    else
	{
		OHIDS_init(Pack, OHID_CMD_BASE_MIX);
		OHID_add_8b(Pack, order);
		OHID_add_8b(Pack, arg1);
		OHID_add_8b(Pack, arg2);
		OHID_add_8b(Pack, arg3);
		Pack->ohid.checksum = OHID_Checksum(Pack);
	}
}
uint8_t OHID_Base_mix_get_byte(union OpenAgreementHID_t* const Pack, const uint8_t index)
{
    return OHID_get_8b(Pack, 1+index);
}
uint16_t OHID_Base_mix_get_half(union OpenAgreementHID_t* const Pack, const uint8_t index)
{
    return OHID_get_16b(Pack, 1+(index<<1));
}
uint32_t OHID_Base_mix_get_word(union OpenAgreementHID_t* const Pack, const uint8_t index)
{
    return OHID_get_32b(Pack, 1+(index<<2));
}
/******************************* 驱动指令 ***********************************/
// Param
void OHIDM_Driver_Param(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_PARAM], const enum OHID_PARAM_PAGE Item[GROUP_LEN_PARAM], const uint16_t Value[GROUP_LEN_PARAM])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_PARAM));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_PARAM);
    for(col=0; col<GROUP_LEN_PARAM; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
        Pack->ohid.data[Pack->ohid.length++] = (uint8_t)Item[col];
        OHID_add_16b(Pack, Value[col]);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Param(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_PARAM], const enum OHID_PARAM_PAGE Item[GROUP_LEN_PARAM], const uint16_t Value[GROUP_LEN_PARAM])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_PARAM, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_PARAM);
        for(col=0; col<GROUP_LEN_PARAM; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            Pack->ohid.data[Pack->ohid.length++] = (uint8_t)Item[col];
            OHID_add_16b(Pack, Value[col]);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
// MT, Reference Wooting
void OHIDM_Driver_MT(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_MT], const uint16_t KEY1[GROUP_LEN_MT], const uint16_t KEY2[GROUP_LEN_MT], const uint16_t delay[GROUP_LEN_MT])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_MT));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_MT);
    for(col=0; col<GROUP_LEN_MT; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
		OHID_add_16b(Pack, KEY1[col]);
		OHID_add_16b(Pack, KEY2[col]);
        OHID_add_16b(Pack, delay[col]);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_MT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_MT], const uint16_t KEY1[GROUP_LEN_MT], const uint16_t KEY2[GROUP_LEN_MT], const uint16_t delay[GROUP_LEN_MT])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_MT, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_MT);
        for(col=0; col<GROUP_LEN_MT; col++)
        {
			OHID_add_16b(Pack, MKEY[col]);
			OHID_add_16b(Pack, KEY1[col]);
			OHID_add_16b(Pack, KEY2[col]);
            OHID_add_16b(Pack, delay[col]);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}

// TGL, Reference Wooting
void OHIDM_Driver_TGL(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_TGL], const uint16_t KEY1[GROUP_LEN_TGL], const uint16_t delay[GROUP_LEN_TGL])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_TGL));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_TGL);
    for(col=0; col<GROUP_LEN_TGL; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
		OHID_add_16b(Pack, KEY1[col]);
        OHID_add_16b(Pack, delay[col]);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_TGL(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_TGL], const uint16_t KEY1[GROUP_LEN_TGL], const uint16_t delay[GROUP_LEN_TGL])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_TGL, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_TGL);
        for(col=0; col<GROUP_LEN_TGL; col++)
        {
			OHID_add_16b(Pack, MKEY[col]);
			OHID_add_16b(Pack, KEY1[col]);
            OHID_add_16b(Pack, delay[col]);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}

/*
 * DKS(Digital keying system)功能设置, Reference Wooting
 * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
 | S/M    | Pack Len     | Des  |        Head[4B]         | <Data>                                                                                                                     |
 | Master | [4B]+[4x15B] | DKS  | 0xAA | len | 0x13 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | KEY3[2B]  | KEY4[2B]  | TrPs1[1B]  | TrPs2[1B]  | TrPs3[1B]  | TrPs4[1B]  | mm1 | ...  | 0xFF 无效
 | Slave  | [4B]+[4x15B] | ACK  | 0xA5 | len | 0x13 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | KEY3[2B]  | KEY4[2B]  | TrPs1[1B]  | TrPs2[1B]  | TrPs3[1B]  | TrPs4[1B]  | mm1 | ...  | 0xFF 无效
*/
void OHIDM_Driver_DKS(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_DKS], const uint16_t KEY1[GROUP_LEN_DKS], const uint16_t KEY2[GROUP_LEN_DKS], const uint16_t KEY3[GROUP_LEN_DKS], const uint16_t KEY4[GROUP_LEN_DKS], const uint8_t TrPs1[GROUP_LEN_DKS], const uint8_t TrPs2[GROUP_LEN_DKS], const uint8_t TrPs3[GROUP_LEN_DKS], const uint8_t TrPs4[GROUP_LEN_DKS], const uint8_t mm1[GROUP_LEN_DKS])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_DKS));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_DKS);
    for(col=0; col<GROUP_LEN_DKS; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
		OHID_add_16b(Pack, KEY1[col]);
		OHID_add_16b(Pack, KEY2[col]);
		OHID_add_16b(Pack, KEY3[col]);
		OHID_add_16b(Pack, KEY4[col]);
        Pack->ohid.data[Pack->ohid.length++] = TrPs1[col];
        Pack->ohid.data[Pack->ohid.length++] = TrPs2[col];
        Pack->ohid.data[Pack->ohid.length++] = TrPs3[col];
        Pack->ohid.data[Pack->ohid.length++] = TrPs4[col];
        Pack->ohid.data[Pack->ohid.length++] = mm1[col];
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_DKS(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_DKS], const uint16_t KEY1[GROUP_LEN_DKS], const uint16_t KEY2[GROUP_LEN_DKS], const uint16_t KEY3[GROUP_LEN_DKS], const uint16_t KEY4[GROUP_LEN_DKS], const uint8_t TrPs1[GROUP_LEN_DKS], const uint8_t TrPs2[GROUP_LEN_DKS], const uint8_t TrPs3[GROUP_LEN_DKS], const uint8_t TrPs4[GROUP_LEN_DKS], const uint8_t mm1[GROUP_LEN_DKS])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_DKS, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_DKS);
        for(col=0; col<GROUP_LEN_DKS; col++)
        {
			OHID_add_16b(Pack, MKEY[col]);
			OHID_add_16b(Pack, KEY1[col]);
			OHID_add_16b(Pack, KEY2[col]);
			OHID_add_16b(Pack, KEY3[col]);
			OHID_add_16b(Pack, KEY4[col]);
            Pack->ohid.data[Pack->ohid.length++] = TrPs1[col];
            Pack->ohid.data[Pack->ohid.length++] = TrPs2[col];
            Pack->ohid.data[Pack->ohid.length++] = TrPs3[col];
            Pack->ohid.data[Pack->ohid.length++] = TrPs4[col];
            Pack->ohid.data[Pack->ohid.length++] = mm1[col];
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}

/*
 * AKS(Analog keying system)功能设置, Reference 海盗船
 * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
 | S/M    | Pack Len     | Des  |        Head[4B]         | <Data1>                                                                        |<Datan>|
 | Master | [4B]+[5x11B] | AKS3 | 0xAA | len | 0x14 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | KEY3[2B]  | mm1[1B]  | mm2[1B]  | mm3[1B]  | ...   | 0xFF 无效
 | Slave  | [4B]+[5x11B] | ACK  | 0xA5 | len | 0x14 | crc | MKEY[2B]  | KEY1[2B]  | KEY2[2B]  | KEY3[2B]  | mm1[1B]  | mm2[1B]  | mm3[1B]  | ...   | 0xFF 无效
*/
void OHIDM_Driver_AKS3(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_AKS3], const uint16_t KEY1[GROUP_LEN_AKS3], const uint16_t KEY2[GROUP_LEN_AKS3], const uint16_t KEY3[GROUP_LEN_AKS3], const uint8_t mm1[GROUP_LEN_AKS3], const uint8_t mm2[GROUP_LEN_AKS3], const uint8_t mm3[GROUP_LEN_AKS3])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_AKS3));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_AKS3);
    for(col=0; col<GROUP_LEN_AKS3; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
		OHID_add_16b(Pack, KEY1[col]);
		OHID_add_16b(Pack, KEY2[col]);
		OHID_add_16b(Pack, KEY3[col]);
        Pack->ohid.data[Pack->ohid.length++] = mm1[col];
        Pack->ohid.data[Pack->ohid.length++] = mm2[col];
        Pack->ohid.data[Pack->ohid.length++] = mm3[col];
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_AKS3(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_AKS3], const uint16_t KEY1[GROUP_LEN_AKS3], const uint16_t KEY2[GROUP_LEN_AKS3], const uint16_t KEY3[GROUP_LEN_AKS3], const uint8_t mm1[GROUP_LEN_AKS3], const uint8_t mm2[GROUP_LEN_AKS3], const uint8_t mm3[GROUP_LEN_AKS3])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_AKS3, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_AKS3);
        for(col=0; col<GROUP_LEN_AKS3; col++)
        {
			OHID_add_16b(Pack, MKEY[col]);
			OHID_add_16b(Pack, KEY1[col]);
			OHID_add_16b(Pack, KEY2[col]);
			OHID_add_16b(Pack, KEY3[col]);
            Pack->ohid.data[Pack->ohid.length++] = mm1[col];
            Pack->ohid.data[Pack->ohid.length++] = mm2[col];
            Pack->ohid.data[Pack->ohid.length++] = mm3[col];
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * KEY,按键设置通用指令,会自动根据按键的模式设置合适的数据格式,该指令最大的用处在于驱动启动时获取按键的数据会很简单，
 * 一般的方法是根据按键的模式参数调佣不同的指令获取按键的数据，但该指令提供了一种通用的方法；
 * 用于读取数据参数 Mode 将被忽略
 | S/M    | Pack Len     | Des |        Head[4B]         | <Data1>             | <Data2>             | <Data3>             | <Data4>             |
 | Master | [4B]+[4x15B] | KEY | 0xAA | len | 0x15 | crc | MKEY1[2B] | D1[13B] | MKEY2[2B] | D2[13B] | MKEY3[2B] | D3[13B] | MKEY4[2B] | D4[13B] |
 | Slave  | [4B]+[4x15B] | ACK | 0xA5 | len | 0x15 | crc | MKEY1[2B] | D1[13B] | MKEY2[2B] | D2[13B] | MKEY3[2B] | D3[13B] | MKEY4[2B] | D4[13B] |
*/
void OHIDM_Driver_Key(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_KEY], const union OHIDM_Driver_Key_t Keys[GROUP_LEN_KEY], const enum OHID_KB_MODE Mode[GROUP_LEN_KEY])
{
    uint8_t col;
    if(write)
    {
        OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_KEY));
        for(col=0; col<GROUP_LEN_KEY; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            switch (Mode[col])
            {
            case KB_SINGLE:
                OHID_add_8b(Pack, Keys[col].Signal.Gmm);
                OHID_add_8b(Pack, Keys[col].Signal.RSTmm);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_RT:
                OHID_add_8b(Pack, Keys[col].RT.Gmm);
                OHID_add_8b(Pack, Keys[col].RT.RTmm);
                OHID_add_8b(Pack, Keys[col].RT.RSTmm);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_DT:
                OHID_add_8b(Pack, Keys[col].DT.Gmm);
                OHID_add_8b(Pack, Keys[col].DT.RSTmm);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_MT:
                OHID_add_16b(Pack, Keys[col].MT.KEY1);
                OHID_add_16b(Pack, Keys[col].MT.KEY2);
                OHID_add_16b(Pack, Keys[col].MT.delay);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_TGL:
                OHID_add_16b(Pack, Keys[col].TGL.KEY1);
                OHID_add_16b(Pack, Keys[col].TGL.delay);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_MACRO:
                OHID_add_16b(Pack, Keys[col].Macro.Addr);
                OHID_add_16b(Pack, Keys[col].Macro.Long);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_APPEND:
                OHID_add_16b(Pack, Keys[col].Append.NewKEY);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_DKS:
                OHID_add_16b(Pack, Keys[col].DKS.KEY1);
                OHID_add_16b(Pack, Keys[col].DKS.KEY2);
                OHID_add_16b(Pack, Keys[col].DKS.KEY3);
                OHID_add_16b(Pack, Keys[col].DKS.KEY4);
                OHID_add_8b(Pack, Keys[col].DKS.TrPs1);
                OHID_add_8b(Pack, Keys[col].DKS.TrPs2);
                OHID_add_8b(Pack, Keys[col].DKS.TrPs3);
                OHID_add_8b(Pack, Keys[col].DKS.TrPs4);
                OHID_add_8b(Pack, Keys[col].DKS.mm1);
                break;
            case KB_AKS3:
                OHID_add_16b(Pack, Keys[col].AKS.KEY1);
                OHID_add_16b(Pack, Keys[col].AKS.KEY2);
                OHID_add_16b(Pack, Keys[col].AKS.KEY3);
                OHID_add_8b(Pack,  Keys[col].AKS.mm1);
                OHID_add_8b(Pack,  Keys[col].AKS.mm2);
                OHID_add_8b(Pack,  Keys[col].AKS.mm3);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_NORMAL:
            default:
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_8b(Pack, 0xFF);
                break;
            }
        }
    }
    else
    {
        OHIDM_init(Pack, OHID_CMD_DRIVER_KEY);
        for(col=0; col<GROUP_LEN_KEY; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            OHID_add_16b(Pack, 0xFFFF);
            OHID_add_16b(Pack, 0xFFFF);
            OHID_add_16b(Pack, 0xFFFF);
            OHID_add_16b(Pack, 0xFFFF);
            OHID_add_16b(Pack, 0xFFFF);
            OHID_add_16b(Pack, 0xFFFF);
            OHID_add_8b(Pack, 0xFF);
        }
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Key(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_KEY], const union OHIDM_Driver_Key_t Keys[GROUP_LEN_KEY], const enum OHID_KB_MODE Mode[GROUP_LEN_KEY])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_KEY, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_KEY);
        for(col=0; col<GROUP_LEN_KEY; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            switch (Mode[col])
            {
            case KB_SINGLE:
                OHID_add_8b(Pack, Keys[col].Signal.Gmm);
                OHID_add_8b(Pack, Keys[col].Signal.RSTmm);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_RT:
                OHID_add_8b(Pack, Keys[col].RT.Gmm);
                OHID_add_8b(Pack, Keys[col].RT.RTmm);
                OHID_add_8b(Pack, Keys[col].RT.RSTmm);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_DT:
                OHID_add_8b(Pack, Keys[col].DT.Gmm);
                OHID_add_8b(Pack, Keys[col].DT.RSTmm);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_MT:
                OHID_add_16b(Pack, Keys[col].MT.KEY1);
                OHID_add_16b(Pack, Keys[col].MT.KEY2);
                OHID_add_16b(Pack, Keys[col].MT.delay);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_TGL:
                OHID_add_16b(Pack, Keys[col].TGL.KEY1);
                OHID_add_16b(Pack, Keys[col].TGL.delay);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_MACRO:
                OHID_add_16b(Pack, Keys[col].Macro.Addr);
                OHID_add_16b(Pack, Keys[col].Macro.Long);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_APPEND:
                OHID_add_16b(Pack, Keys[col].Append.NewKEY);
                OHID_add_8b(Pack, 0xFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_DKS:
                OHID_add_16b(Pack, Keys[col].DKS.KEY1);
                OHID_add_16b(Pack, Keys[col].DKS.KEY2);
                OHID_add_16b(Pack, Keys[col].DKS.KEY3);
                OHID_add_16b(Pack, Keys[col].DKS.KEY4);
                OHID_add_8b(Pack, Keys[col].DKS.TrPs1);
                OHID_add_8b(Pack, Keys[col].DKS.TrPs2);
                OHID_add_8b(Pack, Keys[col].DKS.TrPs3);
                OHID_add_8b(Pack, Keys[col].DKS.TrPs4);
                OHID_add_8b(Pack, Keys[col].DKS.mm1);
                break;
            case KB_AKS3:
                OHID_add_16b(Pack, Keys[col].AKS.KEY1);
                OHID_add_16b(Pack, Keys[col].AKS.KEY2);
                OHID_add_16b(Pack, Keys[col].AKS.KEY3);
                OHID_add_8b(Pack,  Keys[col].AKS.mm1);
                OHID_add_8b(Pack,  Keys[col].AKS.mm2);
                OHID_add_8b(Pack,  Keys[col].AKS.mm3);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                break;
            case KB_NORMAL:
            default:
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_16b(Pack, 0xFFFF);
                OHID_add_8b(Pack, 0xFF);
                break;
            }
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
// Trigger
/*void OHIDM_Driver_Trigger(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint8_t MM, const uint8_t Reset, const uint8_t RTmm, const uint8_t RTreset, const uint8_t mm1, const uint8_t mm2, const uint8_t mm3)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_TRIGGER));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_TRIGGER);
    Pack->ohid.data[Pack->ohid.length++] = MM;
    Pack->ohid.data[Pack->ohid.length++] = Reset;
    Pack->ohid.data[Pack->ohid.length++] = RTmm;
    Pack->ohid.data[Pack->ohid.length++] = RTreset;
    Pack->ohid.data[Pack->ohid.length++] = mm1;
    Pack->ohid.data[Pack->ohid.length++] = mm2;
    Pack->ohid.data[Pack->ohid.length++] = mm3;
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Trigger(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t MM, const uint8_t Reset, const uint8_t RTmm, const uint8_t RTreset, const uint8_t mm1, const uint8_t mm2, const uint8_t mm3)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_TRIGGER, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_TRIGGER);
        Pack->ohid.data[Pack->ohid.length++] = MM;
        Pack->ohid.data[Pack->ohid.length++] = Reset;
        Pack->ohid.data[Pack->ohid.length++] = RTmm;
        Pack->ohid.data[Pack->ohid.length++] = RTreset;
        Pack->ohid.data[Pack->ohid.length++] = mm1;
        Pack->ohid.data[Pack->ohid.length++] = mm2;
        Pack->ohid.data[Pack->ohid.length++] = mm3;
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}*/
// RGB_PARAM
/*
 * RGB_PARAM 设置
 | S/M    | Pack Len   | Des     |        Head[4B]         | <Data>                                                                                                         |
 | Master | [4B]+[43B] | PARAM   | 0xAA | len | 0x16 | crc | BACK[4B]  | Palettes[8*4B] | Gray[1B] | MODE[1B] | SPEED[1B] | SLEEP[1B] | ON[1B] | ON_SLEEP[1B] | REVERSE[1B] |
 | Slave  | [4B]+[43B] | ACK     | 0xA5 | len | 0x16 | crc | BACK[4B]  | Palettes[8*4B] | Gray[1B] | MODE[1B] | SPEED[1B] | SLEEP[1B] | ON[1B] | ON_SLEEP[1B] | REVERSE[1B] |
*/
void OHIDM_Driver_RGB_PARAM(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint32_t back, const uint32_t Palettes[8], const uint8_t gray, const uint8_t mode, const uint8_t speed, const uint8_t sleep, const uint8_t on, const uint8_t on_sleep, const uint8_t reverse)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_RGB_PARAM));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_RGB_PARAM);
	OHID_add_32b(Pack, back);
	OHID_add_32b(Pack, Palettes[0]);
	OHID_add_32b(Pack, Palettes[1]);
	OHID_add_32b(Pack, Palettes[2]);
	OHID_add_32b(Pack, Palettes[3]);
	OHID_add_32b(Pack, Palettes[4]);
	OHID_add_32b(Pack, Palettes[5]);
	OHID_add_32b(Pack, Palettes[6]);
	OHID_add_32b(Pack, Palettes[7]);
    Pack->ohid.data[Pack->ohid.length++] = gray;
    Pack->ohid.data[Pack->ohid.length++] = mode;
    Pack->ohid.data[Pack->ohid.length++] = speed;
    Pack->ohid.data[Pack->ohid.length++] = sleep;
	Pack->ohid.data[Pack->ohid.length++] = on;
	Pack->ohid.data[Pack->ohid.length++] = on_sleep;
    Pack->ohid.data[Pack->ohid.length++] = reverse;
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_RGB_PARAM(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t back, const uint32_t Palettes[8],  const uint8_t gray, const uint8_t mode, const uint8_t speed, const uint8_t sleep, const uint8_t on, const uint8_t on_sleep, const uint8_t reverse)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_RGB_PARAM, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_RGB_PARAM);
		OHID_add_32b(Pack, back);
		OHID_add_32b(Pack, Palettes[0]);
		OHID_add_32b(Pack, Palettes[1]);
		OHID_add_32b(Pack, Palettes[2]);
		OHID_add_32b(Pack, Palettes[3]);
		OHID_add_32b(Pack, Palettes[4]);
		OHID_add_32b(Pack, Palettes[5]);
		OHID_add_32b(Pack, Palettes[6]);
		OHID_add_32b(Pack, Palettes[7]);
        Pack->ohid.data[Pack->ohid.length++] = gray;
        Pack->ohid.data[Pack->ohid.length++] = mode;
        Pack->ohid.data[Pack->ohid.length++] = speed;
        Pack->ohid.data[Pack->ohid.length++] = sleep;
		Pack->ohid.data[Pack->ohid.length++] = on;
		Pack->ohid.data[Pack->ohid.length++] = on_sleep;
        Pack->ohid.data[Pack->ohid.length++] = reverse;
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
// KRGB, 提供以下三种接口
/*
 * RGB 设置, 根据 RGB 对应的按键的键值识别, 此方法使用的时候更方便, 但如果一个 RGB 没有对应的按键将不可设置
 * MKEY : <HID Usage Tables> - 10 Keyboard/Keypad Page (0x07) range[0x04-0xA4], [0xE0-0xE7]
 | S/M    | Pack Len     | Des  |        Head[4B]         | <Data1>                                          |<Datan>|
 | Master | [4B]+[12x5B] | KRGB | 0xAA | len | 0x17 | crc | MKEY[2B]  | RGB.R[1B]  | RGB.G[1B]  | RGB.B[1B]  | ...   | 0xFF 无效
 | Slave  | [4B]+[12x5B] | ACK  | 0xA5 | len | 0x17 | crc | MKEY[2B]  | RGB.R[1B]  | RGB.G[1B]  | RGB.B[1B]  | ...   | 0xFF 无效
*/
void OHIDM_Driver_krgb2(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_KRBG], const uint8_t  RGBR[GROUP_LEN_KRBG], const uint8_t RGBG[GROUP_LEN_KRBG], const uint8_t RGBB[GROUP_LEN_KRBG])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_KRGB));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_KRGB);
    for(col=0; col<GROUP_LEN_KRBG; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
        Pack->ohid.data[Pack->ohid.length++] = RGBR[col];
        Pack->ohid.data[Pack->ohid.length++] = RGBG[col];
        Pack->ohid.data[Pack->ohid.length++] = RGBB[col];
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_krgb2(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_KRBG], const uint8_t  RGBR[GROUP_LEN_KRBG], const uint8_t RGBG[GROUP_LEN_KRBG], const uint8_t RGBB[GROUP_LEN_KRBG])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_KRGB, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_KRGB);
        for(col=0; col<GROUP_LEN_KRBG; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            Pack->ohid.data[Pack->ohid.length++] = RGBR[col];
            Pack->ohid.data[Pack->ohid.length++] = RGBG[col];
            Pack->ohid.data[Pack->ohid.length++] = RGBB[col];
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDM_Driver_krgb3(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_KRBG], const uint32_t RGB[GROUP_LEN_KRBG])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_KRGB));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_KRGB);
    for(col=0; col<GROUP_LEN_KRBG; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
        Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>16)&0xFF;
        Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>8)&0xFF;
        Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>0)&0xFF;
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_krgb3(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code,  const uint16_t MKEY[GROUP_LEN_KRBG], const uint32_t RGB[GROUP_LEN_KRBG])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_KRGB, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_KRGB);
        for(col=0; col<GROUP_LEN_KRBG; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>16)&0xFF;
            Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>8)&0xFF;
            Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>0)&0xFF;
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * RGB 设置, 根据 RGB 坐标识别, 此方法的通用性更高, IDX<128 为按键灯 (Matrix6*21) , IDX>=128 为侧灯
 | S/M    | Pack Len     | Des  |        Head[4B]         | <Data1>                                         |<Datan>|
 | Master | [4B]+[15x4B] | IRGB | 0xAA | len | 0x18 | crc | IDX[1B]  | RGB.R[1B]  | RGB.G[1B]  | RGB.B[1B]  | ...   | 0xFF 无效
 | Slave  | [4B]+[15x4B] | ACK  | 0xA5 | len | 0x18 | crc | IDX[1B]  | RGB.R[1B]  | RGB.G[1B]  | RGB.B[1B]  | ...   | 0xFF 无效
*/
void OHIDM_Driver_IRGB1(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint32_t irgb[GROUP_LEN_IRBG])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_IRGB));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_IRGB);
    for(col=0; col<GROUP_LEN_IRBG; col++) OHID_add_32b(Pack, irgb[col]);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_IRGB1(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t irgb[GROUP_LEN_IRBG])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_IRGB, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_IRGB);
        for(col=0; col<GROUP_LEN_IRBG; col++) OHID_add_32b(Pack, irgb[col]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDM_Driver_IRGB2(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint8_t idx[GROUP_LEN_IRBG], const uint32_t RGB[GROUP_LEN_IRBG])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_IRGB));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_IRGB);
    for(col=0; col<GROUP_LEN_IRBG; col++)
    {
        Pack->ohid.data[Pack->ohid.length++] = idx[col];
        Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>16)&0xFF;
        Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>8)&0xFF;
        Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>0)&0xFF;
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_IRGB2(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t idx[GROUP_LEN_IRBG], const uint32_t RGB[GROUP_LEN_IRBG])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_IRGB, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_IRGB);
        for(col=0; col<GROUP_LEN_IRBG; col++)
        {
            Pack->ohid.data[Pack->ohid.length++] = idx[col];
            Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>16)&0xFF;
            Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>8)&0xFF;
            Pack->ohid.data[Pack->ohid.length++] = (RGB[col]>>0)&0xFF;
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
// MacroList
void OHIDM_Driver_MacroList(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint32_t macro[GROUP_LEN_MACROLIST], uint8_t len, const uint16_t offset)
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_MACROLIST));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_MACROLIST);
    OHID_add_16b(Pack, offset);
    if(len>GROUP_LEN_MACROLIST) len = GROUP_LEN_MACROLIST;
    OHID_add_8b(Pack, len);
    for(col=0; col<len; col++) OHID_add_32b(Pack, macro[col]);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_MacroList(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t macro[14], uint8_t len, const uint16_t offset)
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_MACROLIST, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_MACROLIST);
        OHID_add_16b(Pack, offset);
        if(len>GROUP_LEN_MACROLIST) len = GROUP_LEN_MACROLIST;
        OHID_add_8b(Pack, len);
        for(col=0; col<len; col++) OHID_add_32b(Pack, macro[col]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}

// RP
/*void OHIDM_Driver_RP(union OpenAgreementHID_t* const Pack, const uint8_t write, const enum OHID_RP_TYPE type, const uint16_t delay)
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_RP));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_RP);
    OHID_add_8b(Pack, (uint8_t)type);
	OHID_add_16b(Pack, delay);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}*/
void OHIDS_Driver_RP(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_RP_TYPE  type, const uint8_t bin[GROUP_LEN_RP])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_RP, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_RP);
        OHID_add_8b(Pack, (uint8_t)type);
        for(col=0; col<GROUP_LEN_RP; col++) OHID_add_8b(Pack, bin[col]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDS_Driver_RP_Half(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_RP_TYPE type, const uint16_t half[GROUP_LEN_RP_HALF])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_RP, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_RP);
        OHID_add_8b(Pack, (uint8_t)type);
        for(col=0; col<GROUP_LEN_RP_HALF; col++) OHID_add_16b(Pack, half[col]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
// curve
void OHIDM_Driver_Curve(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint8_t idx, uint8_t len, const uint16_t offset, const uint16_t curve[GROUP_LEN_CURVE])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_CURVE));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_CURVE);
    if(len>GROUP_LEN_CURVE) len = GROUP_LEN_CURVE;
    OHID_add_8b(Pack, idx);
    OHID_add_8b(Pack, len);
    OHID_add_16b(Pack, offset);
    for(col=0; col<len; col++) OHID_add_16b(Pack, curve[col]);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Curve(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t idx, uint8_t len, const uint16_t offset, const uint16_t curve[GROUP_LEN_CURVE])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_CURVE, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_CURVE);
        if(len>GROUP_LEN_CURVE) len = GROUP_LEN_CURVE;
        OHID_add_8b(Pack, idx);
        OHID_add_8b(Pack, len);
        OHID_add_16b(Pack, offset);
        for(col=0; col<len; col++) OHID_add_16b(Pack, curve[col]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * NORMAL, 普通触发
 | S/M    | Pack Len   | Des    |        Head[4B]         | <Data>                       |
 | Master | [4B]+[2nB] | NORMAL | 0xAA | len | 0x1C | crc | MKEY1[2B] | ... | MKEY30[2B] |
 | Slave  | [4B]+[2nB] | ACK    | 0xA5 | len | 0x1C | crc | MKEY1[2B] | ... | MKEY30[2B] |
*/
void OHIDM_Driver_Normal(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_NORMAL])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_NORMAL));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_NORMAL);
    for(col=0; col<GROUP_LEN_NORMAL; col++) OHID_add_16b(Pack, MKEY[col]);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Normal(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_NORMAL])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_NORMAL, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_NORMAL);
        for(col=0; col<GROUP_LEN_NORMAL; col++) OHID_add_16b(Pack, MKEY[col]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * QUICK, 快速触发,快速触发模式下仅上报最后一次按下的按键
 | S/M    | Pack Len     | Des    |        Head[4B]         | <Data>                                                |
 | Master | [4B]+[20x3B] | QUICK  | 0xAA | len | 0x23 | crc | MKEY1[2B] | Mode1[1B] | ... | MKEY20[2B] | Mode20[1B] |
 | Slave  | [4B]+[20x3B] | ACK    | 0xA5 | len | 0x23 | crc | MKEY1[2B] | Mode1[1B] | ... | MKEY20[2B] | Mode20[1B] |
*/
void OHIDM_Driver_Quick(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_QUICK], const uint8_t Mode[GROUP_LEN_QUICK])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_QUICK));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_QUICK);
    for(col=0; col<GROUP_LEN_QUICK; col++) 
	{
		OHID_add_16b(Pack, MKEY[col]);
		OHID_add_8b(Pack, Mode[col]);
	}
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Quick(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_QUICK], const uint8_t Mode[GROUP_LEN_QUICK])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_QUICK, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_QUICK);
        for(col=0; col<GROUP_LEN_QUICK; col++) 
		{
			OHID_add_16b(Pack, MKEY[col]);
			OHID_add_8b(Pack, Mode[col]);
		}
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * SOCD, SOCD 模式只上报按下最深的按键
 | S/M    | Pack Len     | Des    |        Head[4B]         | <Data>                                                                                                |
 | Master | [4B]+[12x5B] | SOCD   | 0xAA | len | 0x25 | crc | MM1[1B] | Reset1[1B] | MKEY1[2B] | Mode1[1B] | ... | MM19[1B] | Reset19[1B] | MKEY19[2B] | Mode19[1B] |
 | Slave  | [4B]+[12x5B] | ACK    | 0xA5 | len | 0x25 | crc | MM1[1B] | Reset1[1B] | MKEY1[2B] | Mode1[1B] | ... | MM19[1B] | Reset19[1B] | MKEY19[2B] | Mode29[1B] |
*/
void OHIDM_Driver_SOCD(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_SOCD], const uint8_t Mode[GROUP_LEN_SOCD], const uint8_t MM[GROUP_LEN_SOCD], const uint8_t RST[GROUP_LEN_SOCD])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_SOCD));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_SOCD);
    for(col=0; col<GROUP_LEN_SOCD; col++) 
	{
		OHID_add_8b(Pack, MM[col]);
		OHID_add_8b(Pack, RST[col]);
		OHID_add_16b(Pack, MKEY[col]);
		OHID_add_8b(Pack, Mode[col]);
	}
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_SOCD(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_SOCD], const uint8_t Mode[GROUP_LEN_SOCD], const uint8_t MM[GROUP_LEN_SOCD], const uint8_t RST[GROUP_LEN_SOCD])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_SOCD, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_SOCD);
        for(col=0; col<GROUP_LEN_SOCD; col++) 
		{
			OHID_add_8b(Pack, MM[col]);
			OHID_add_8b(Pack, RST[col]);
			OHID_add_16b(Pack, MKEY[col]);
			OHID_add_8b(Pack, Mode[col]);
		}
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * SINGLE, 单键触发
 | S/M    | Pack Len    | Des    |        Head[4B]         | <Data>                        |
 | Master | [4B]+[ 4nB] | SINGLE | 0xAA | len | 0x1D | crc | MKEY[2B] | MM[1B] | Reset[1B] |
 | Slave  | [4B]+[ 4nB] | ACK    | 0xA5 | len | 0x1D | crc | MKEY[2B] | MM[1B] | Reset[1B] |
*/
void OHIDM_Driver_SIGNAL(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_SINGLE], const uint8_t MM[GROUP_LEN_SINGLE], const uint8_t Reset[GROUP_LEN_SINGLE])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_SINGLE));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_SINGLE);
    for(col=0; col<GROUP_LEN_SINGLE; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
        OHID_add_8b(Pack,  MM[col]);
        OHID_add_8b(Pack,  Reset[col]);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_SIGNAL(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_SINGLE], const uint8_t MM[GROUP_LEN_SINGLE], const uint8_t Reset[GROUP_LEN_SINGLE])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_SINGLE, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_SINGLE);
        for(col=0; col<GROUP_LEN_SINGLE; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            OHID_add_8b(Pack, MM[col]);
            OHID_add_8b(Pack, Reset[col]);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * RT, 快速触发
 | S/M    | Pack Len    | Des   |        Head[4B]         | <Data>                                   |
 | Master | [4B]+[ 5nB] | RT    | 0xAA | len | 0x1E | crc | MKEY[2B] | MM[1B] | RTmm[1B] | Reset[1B] |
 | Slave  | [4B]+[ 5nB] | ACK   | 0xA5 | len | 0x1E | crc | MKEY[2B] | MM[1B] | RTmm[1B] | Reset[1B] |
*/
void OHIDM_Driver_RT(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_RT], const uint8_t MM[GROUP_LEN_RT], const uint8_t RTmm[GROUP_LEN_RT], const uint8_t Reset[GROUP_LEN_RT])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_RT));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_RT);
    for(col=0; col<GROUP_LEN_RT; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
        OHID_add_8b(Pack,  MM[col]);
        OHID_add_8b(Pack,  RTmm[col]);
        OHID_add_8b(Pack,  Reset[col]);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_RT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_RT], const uint8_t MM[GROUP_LEN_RT], const uint8_t RTmm[GROUP_LEN_RT], const uint8_t Reset[GROUP_LEN_RT])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_RT, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_RT);
        for(col=0; col<GROUP_LEN_RT; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            OHID_add_8b(Pack,  MM[col]);
            OHID_add_8b(Pack,  RTmm[col]);
            OHID_add_8b(Pack,  Reset[col]);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * DT, 增程触发
 | S/M    | Pack Len    | Des   |        Head[4B]         | <Data>                            |
 | Master | [4B]+[ 4nB] | DT    | 0xAA | len | 0x1F | crc | MKEY[2B] | DTmm[1B] | DTreset[1B] |
 | Slave  | [4B]+[ 4nB] | ACK   | 0xA5 | len | 0x1F | crc | MKEY[2B] | DTmm[1B] | DTreset[1B] |
*/
void OHIDM_Driver_DT(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_DT], const uint8_t DTmm[GROUP_LEN_DT], const uint8_t DTreset[GROUP_LEN_DT])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_DT));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_DT);
    for(col=0; col<GROUP_LEN_DT; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
        OHID_add_8b(Pack,  DTmm[col]);
        OHID_add_8b(Pack,  DTreset[col]);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_DT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_DT], const uint8_t DTmm[GROUP_LEN_DT], const uint8_t DTreset[GROUP_LEN_DT])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_DT, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_DT);
        for(col=0; col<GROUP_LEN_DT; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            OHID_add_8b(Pack,  DTmm[col]);
            OHID_add_8b(Pack,  DTreset[col]);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * MACRO, 宏指令
 | S/M    | Pack Len    | Des   |        Head[4B]         | <Data>                         |
 | Master | [4B]+[ 6nB] | MACRO | 0xAA | len | 0x20 | crc | MKEY[2B] | Addr[2B] | Long[2B] |
 | Slave  | [4B]+[ 6nB] | ACK   | 0xA5 | len | 0x20 | crc | MKEY[2B] | Addr[2B] | Long[2B] |
*/
void OHIDM_Driver_Macro_Key(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_MACRO], const uint16_t Addr[GROUP_LEN_MACRO], const uint16_t Long[GROUP_LEN_MACRO])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_MACRO));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_MACRO);
    for(col=0; col<GROUP_LEN_MACRO; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
        OHID_add_16b(Pack, Addr[col]);
        OHID_add_16b(Pack, Long[col]);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Macro_Key(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_MACRO], const uint16_t Addr[GROUP_LEN_MACRO], const uint16_t Long[GROUP_LEN_MACRO])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_MACRO, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_MACRO);
        for(col=0; col<GROUP_LEN_MACRO; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            OHID_add_16b(Pack, Addr[col]);
            OHID_add_16b(Pack, Long[col]);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * MPT(Macro Partition Table), 宏分区指令, 该内容和 OHID_CMD_DRIVER_MACROLIST 是一个整体
 | S/M    | Pack Len      | Des   |        Head[4B]         | <Data>     宏ID       宏地址      宏长度                                                          |
 | Master | [4B]+[2+9x6B] | MPT   | 0xAA | len | 0x22 | crc | Page[2B] | MID1[2B] | Addr1[2B] | Long1[2B] | MID2[2B] | Addr2[2B] | Long2[2B] |
 | Slave  | [4B]+[2+9x6B] | ACK   | 0xA5 | len | 0x22 | crc | Page[2B] | MID1[2B] | Addr1[2B] | Long1[2B] | MID2[2B] | Addr2[2B] | Long2[2B] |
*/
void OHIDM_Driver_Macro_PT(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t Page, const struct Macro_PT_t MPT[GROUP_LEN_MACRO_PT])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_MACRO_PT));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_MACRO_PT);
	OHID_add_16b(Pack, Page);
    for(col=0; col<GROUP_LEN_MACRO_PT; col++)
    {
        OHID_add_16b(Pack, MPT[col].MacroID);
        OHID_add_16b(Pack, MPT[col].Addr);
        OHID_add_16b(Pack, MPT[col].Long);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Macro_PT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t Page, const struct Macro_PT_t MPT[GROUP_LEN_MACRO_PT])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_MACRO_PT, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_MACRO_PT);
		OHID_add_16b(Pack, Page);
        for(col=0; col<GROUP_LEN_MACRO_PT; col++)
        {
			OHID_add_16b(Pack, MPT[col].MacroID);
			OHID_add_16b(Pack, MPT[col].Addr);
			OHID_add_16b(Pack, MPT[col].Long);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDS_Driver_Get_MPT(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, struct Macro_PT_t MPT[GROUP_LEN_MACRO_PT])
{
    uint8_t col;
	for(col=0; col<GROUP_LEN_MACRO_PT; col++)
	{
		MPT[col].MacroID = OHID_get_16b(Pack,  2+(col<<2)+(col<<1)+0);  // col*6;
		MPT[col].Addr = OHID_get_16b(Pack,     2+(col<<2)+(col<<1)+2);  // col*6;
		MPT[col].Long = OHID_get_16b(Pack,     2+(col<<2)+(col<<1)+4);  // col*6;
	}
}

/*
 * APPEND, 追加
 | S/M    | Pack Len      | Des    |        Head[4B]         | <Data>                |
 | Master | [4B]+[15x4B]  | APPEND | 0xAA | len | 0x21 | crc | MKEY[2B] | NewKEY[2B] |
 | Slave  | [4B]+[15x4B]  | ACK    | 0xA5 | len | 0x21 | crc | MKEY[2B] | NewKEY[2B] |
*/
void OHIDM_Driver_Append(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint16_t MKEY[GROUP_LEN_APPEND], const uint16_t NewKEY[GROUP_LEN_APPEND])
{
    uint8_t col;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_DRIVER_APPEND));
    else      OHIDM_init(Pack, OHID_CMD_DRIVER_APPEND);
    for(col=0; col<GROUP_LEN_APPEND; col++)
    {
        OHID_add_16b(Pack, MKEY[col]);
        OHID_add_16b(Pack, NewKEY[col]);
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Driver_Append(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint16_t MKEY[GROUP_LEN_APPEND], const uint16_t NewKEY[GROUP_LEN_APPEND])
{
    uint8_t col;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_DRIVER_APPEND, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_DRIVER_APPEND);
        for(col=0; col<GROUP_LEN_APPEND; col++)
        {
            OHID_add_16b(Pack, MKEY[col]);
            OHID_add_16b(Pack, NewKEY[col]);
        }
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}


/************************** (C) COPYRIGHT Merafour **************************/
