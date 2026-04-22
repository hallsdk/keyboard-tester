/******************** (C) COPYRIGHT 2018 merafour ********************
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.0.0
* Date               : 2024.04.07
* Description        : OpenAgreementHID.
* Description        : 开源键盘通信协议 API 接口.
* Description        : 这些接口对数据包进行封装,可以让对协议的操作更简单
* Description        : 注：
* Description        : OHIDM_* : 主机(电脑上位机)发送给从机的数据包
* Description        : OHIDS_* : 从机(键盘)发送给主机的数据包
* Description        : 如无必要,勿增实体.
********************************************************************************
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#include <OHID/OHID_Pack.h>

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

/******************************* 基础指令 ***********************************/
void OHIDM_None(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd)
{
    OHIDM_init(Pack, cmd);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_None(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd)
{
    OHIDS_init(Pack, cmd);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
/*
 * Routing, 路由指令,用于上位机连接与主 MCU连接的辅助 MCU
 | S/M    | Pack Len    | Des     |        Head[4B]         | <Data>                   |
 | Master | [4B]+[1+nB] | Routing | 0xAA | len | 0x7E | crc | endpoint[1B] | <arg>[nB] |
 | Slave  | [4B]+[1+nB] | ACK     | 0xA5 | len | 0x7E | crc | endpoint[1B] | <arg>[nB] |
*/
void OHIDM_Routing(union OpenAgreementHID_t* const Pack, const enum OHID_ROUTING endpoint, const uint8_t arg[], const uint8_t len)
{
	uint16_t cnt;
    OHIDM_init(Pack, OHID_CMD_BASE_ROUTING);
    OHID_add_8b(Pack, endpoint);
    for(cnt=0; cnt<len; cnt++) OHID_add_8b(Pack, arg[cnt]);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Routing(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_ROUTING endpoint, const uint8_t arg[], const uint8_t len)
{
	uint16_t cnt;
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_ROUTING, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_BASE_ROUTING);
		OHID_add_8b(Pack, endpoint);
		for(cnt=0; cnt<len; cnt++) OHID_add_8b(Pack, arg[cnt]);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * Bridging, 桥接指令,用于上位机连接与主 MCU连接的辅助 MCU
 * 与 Routing 不同的是, Routing 把需要发送的数据放在 arg 字段，接收到该指令直接将整个数据包转发到 endpoint,
 * 而 Bridging 开启后是将所有的数据包都转发到 endpoint, 知道接收到新的 Bridging 指令关闭 Bridging 功能,
 * 两者都是为了实现上位机连接与主 MCU 相连的辅助 MCU, 但 Bridging 实现起来更为复杂
 | S/M    | Pack Len    | Des     |        Head[4B]         | <Data>       |
 | Master | [4B]+[1+nB] | Bridging| 0xAA | len | 0x7D | crc | endpoint[1B] |
 | Slave  | [4B]+[1+nB] | ACK     | 0xA5 | len | 0x7D | crc | endpoint[1B] |
*/
void OHIDM_Bridging(union OpenAgreementHID_t* const Pack, const enum OHID_ROUTING endpoint)
{
    OHIDM_init(Pack, OHID_CMD_BASE_BRIDGING);
    OHID_add_8b(Pack, endpoint);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Bridging(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_ROUTING endpoint)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_BRIDGING, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_BASE_BRIDGING);
        OHID_add_8b(Pack, endpoint);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
/*
 * 同步指令,用于获取设备的基础信息,通常为 Master 给 Slave 发送的第一个指令
 | S/M    | Pack Len   | Des  |        Head[4B]         | <Data>                                                             |
 | Master | [4B]+[ 0B] | SYNC | 0xAA | len | 0x01 | crc |                                                                    |
 | Slave  | [4B]+[41B] | ACK  | 0xA5 | len | 0x01 | crc | Board ID[4B] | fw_size[2B] | run_mode[1B] | SN[17B] | version[17B] |
*/
void OHIDS_Base_sync(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_BOARDS_ID_LIST board_id, const uint16_t fw_size, const enum OHID_RUN_MODE run_mode, const char SN[16], const char version[16])
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_SAFE, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_BASE_SYNC);
        OHID_add_32b(Pack, (uint32_t)board_id);
        OHID_add_16b(Pack, fw_size);
        OHID_add_8b(Pack, (uint8_t)run_mode);
        OHID_add_bin(Pack, (const uint8_t*)SN, 16);
        OHID_add_bin(Pack, (const uint8_t*)version, 16);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDM_Base_SAFE(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint8_t SN[20], const uint8_t Encryption[32])
{
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_BASE_SAFE));
    else      OHIDM_init(Pack, OHID_CMD_BASE_SAFE);
    OHID_add_bin(Pack, (const uint8_t*)SN, 20);
    OHID_add_bin(Pack, (const uint8_t*)Encryption, 32);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Base_SAFE(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t SN[20], const uint8_t Encryption[32])
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_BASE_SAFE, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_BASE_SAFE);
        OHID_add_bin(Pack, (const uint8_t*)SN, 20);
        OHID_add_bin(Pack, (const uint8_t*)Encryption, 32);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}

/******************************* IAP 指令 ***********************************/
void OHIDM_IAP_Sign(union OpenAgreementHID_t* const Pack, const enum OHID_UNLOCK unlock, const uint8_t SIGN[16], const uint8_t Datas[16])
{
    OHIDM_init(Pack, OHID_CMD_IAP_SIGN);
    Pack->ohid.data[Pack->ohid.length++] = unlock;
    OHID_add_bin(Pack, SIGN, 16);
    OHID_add_bin(Pack, Datas, 16);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_IAP_Sign(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const enum OHID_UNLOCK unlock, const uint8_t Datas[16])
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_IAP_SIGN, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_IAP_SIGN);
        Pack->ohid.data[Pack->ohid.length++] = unlock;
        OHID_add_bin(Pack, Datas, 16);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDM_IAP_Erase(union OpenAgreementHID_t* const Pack, const uint32_t address, const uint32_t eSize)
{
    OHIDM_init(Pack, OHID_CMD_IAP_ERASE);
    OHID_add_32b(Pack, address);
    OHID_add_32b(Pack, eSize);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_IAP_Erase(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t progress)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_IAP_ERASE, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_IAP_ERASE);
        Pack->ohid.data[Pack->ohid.length++] = progress;
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDM_IAP_Reboot(union OpenAgreementHID_t* const Pack, const uint8_t rand)
{
    OHIDM_init(Pack, OHID_CMD_IAP_REBOOT);
    OHID_add_8b(Pack, rand);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_IAP_Reboot(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint8_t rand)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_IAP_REBOOT, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_IAP_REBOOT);
        OHID_add_8b(Pack, rand);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDM_IAP_Jump(union OpenAgreementHID_t* const Pack, const uint32_t address, const uint32_t Size, const uint32_t CRCin)
{
    OHIDM_init(Pack, OHID_CMD_IAP_JUMP);
    OHID_add_32b(Pack, address);
    OHID_add_32b(Pack, Size);
    OHID_add_32b(Pack, CRCin);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_IAP_Jump(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t address, const uint32_t Size, const uint32_t CRCin)
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_IAP_JUMP, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_IAP_JUMP);
        OHID_add_32b(Pack, address);
        OHID_add_32b(Pack, Size);
        OHID_add_32b(Pack, CRCin);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDM_IAP_Program(union OpenAgreementHID_t* const Pack, const uint8_t write, const uint32_t address, const uint16_t Size, const uint8_t binary[])
{
	uint8_t kk;
    if(write) OHIDM_init(Pack, OHID_CMD_WRITE(OHID_CMD_IAP_PROGRAM));
    else      OHIDM_init(Pack, OHID_CMD_IAP_PROGRAM);
    OHID_add_32b(Pack, address);
    OHID_add_16b(Pack, Size);
    for(kk=0; kk<Size; kk++) Pack->ohid.data[Pack->ohid.length++] = binary[kk];
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_IAP_Program(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t address, const uint16_t Size, const uint8_t binary[])
{
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_IAP_PROGRAM, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_IAP_PROGRAM);
        OHID_add_32b(Pack, address);
        OHID_add_16b(Pack, Size);
        for(uint8_t kk=0; kk<Size; kk++) Pack->ohid.data[Pack->ohid.length++] = binary[kk];
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}
void OHIDM_IAP_RCRC(union OpenAgreementHID_t* const Pack, const uint32_t address, const uint32_t Size)
{
    OHIDM_init(Pack, OHID_CMD_IAP_RCRC);
    OHID_add_32b(Pack, address);
    OHID_add_32b(Pack, Size);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_IAP_RCRC(union OpenAgreementHID_t* const Pack, const enum OHID_ERR_CODE code, const uint32_t address, const uint32_t Size, const uint32_t CRCin)
{ 
    if(OHID_ERR_CODE_NONE!=code)
    {
        OHIDS_fail(Pack, OHID_CMD_IAP_RCRC, code);
    }
    else
    {
        OHIDS_init(Pack, OHID_CMD_IAP_RCRC);
        OHID_add_32b(Pack, address);
        OHID_add_32b(Pack, Size);
        OHID_add_32b(Pack, CRCin);
        Pack->ohid.checksum = OHID_Checksum(Pack);
    }
}

/************************** (C) COPYRIGHT Merafour **************************/
