/******************** (C) COPYRIGHT 2018 merafour ********************
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.1.0
* Date               : 2024.04.07
* Description        : OpenAgreementHID.
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
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#include <OHID/OHID_Port.h>

/**************************************** Pack API **********************************************/
void OHIDS_fail(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd, const enum OHID_ERR_CODE code)
{
    OHIDS_init(Pack, OHID_CMD_BASE_FAIL);
    OHID_add_8b(Pack, (uint8_t)code);
    OHID_add_8b(Pack, (uint8_t)cmd);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_fail_arg2(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd, const enum OHID_ERR_CODE code, const uint8_t arg1, const uint8_t arg2)
{
    OHIDS_init(Pack, OHID_CMD_BASE_FAIL);
    OHID_add_8b(Pack, (uint8_t)code);
    OHID_add_8b(Pack, (uint8_t)cmd);
    OHID_add_8b(Pack, arg1);
    OHID_add_8b(Pack, arg2);
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDS_Debug(union OpenAgreementHID_t* const Pack, const uint8_t str[], uint8_t len)
{
    int cnt;
    OHIDS_init(Pack, OHID_CMD_BASE_DEBUG);
    if(len>60) len = 60;
    for(cnt=0; cnt<len; cnt++)
    {
        Pack->ohid.data[Pack->ohid.length++] = str[cnt];
    }
    Pack->ohid.checksum = OHID_Checksum(Pack);
}
void OHIDM_init(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd)
{
    int cnt;
    for(cnt=0; cnt<(64/4); cnt++) Pack->word[cnt] = 0x00;
    // head
    Pack->ohid.HEAD = OHID_PACK_HEAD_MASTRT;
    // len
    Pack->ohid.length = 0;
    // cmd
    Pack->ohid.cmd = (uint8_t)cmd;
    Pack->ohid.checksum = 0;
}
void OHIDS_init(union OpenAgreementHID_t* const Pack, const enum OHID_CMD cmd)
{
    int cnt;
    for(cnt=0; cnt<(64/4); cnt++) Pack->word[cnt] = 0x00;
    // head
    Pack->ohid.HEAD = OHID_PACK_HEAD_SLAVE;
    // len
    Pack->ohid.length = 0;
    // cmd
    Pack->ohid.cmd = (uint8_t)cmd;
    Pack->ohid.checksum = 0;
}
// crc=head+len+cmd+<data.end>+1+2+3+…+(len+3)；
uint8_t OHID_Checksum(const union OpenAgreementHID_t* const Pack)
{
    uint8_t checksum = 0;
    uint8_t cnt;
    const uint8_t len = Pack->ohid.length;

    checksum += Pack->ohid.HEAD + 1;
    checksum += len + 2;
    checksum += Pack->ohid.cmd + 3;
    if((len>0) && (len<=OHID_DATA_SIZE))
    {
        for(cnt=0; cnt<len; cnt++) checksum += Pack->ohid.data[cnt]+cnt+4;
    }
    return checksum;
}
int OHIDM_encode(union OpenAgreementHID_t* const Pack)
{
	int cnt;
    if(Pack->ohid.length>=OHID_DATA_SIZE) return 0;
    // head
    Pack->ohid.HEAD = OHID_PACK_HEAD_MASTRT;
    Pack->ohid.checksum = OHID_Checksum(Pack);
    for(cnt=Pack->ohid.length+4; cnt<OHID_PORT_SIZE; cnt++) Pack->bin[cnt] = 0xFF;
    return (Pack->ohid.length+4);
}
int OHIDS_encode(union OpenAgreementHID_t* const Pack)
{
	int cnt;
    if(Pack->ohid.length>=OHID_DATA_SIZE) return 0;
    // head
    Pack->ohid.HEAD = OHID_PACK_HEAD_SLAVE;
    Pack->ohid.checksum = OHID_Checksum(Pack);
    for(cnt=Pack->ohid.length+4; cnt<OHID_PORT_SIZE; cnt++) Pack->bin[cnt] = 0xFF;
    return (Pack->ohid.length+4);
}
int _OHIDM_Decode(union OpenAgreementHID_t* const Pack, const uint16_t bSize)
{
    uint8_t crc=0;
    if(bSize<OHID_HEAD_SIZE) return -1;
    if(OHID_PACK_HEAD_MASTRT!=Pack->ohid.HEAD) return -2;
    if(Pack->ohid.length>OHID_DATA_SIZE) return -3;
    if((Pack->ohid.length+(uint16_t)OHID_HEAD_SIZE)>bSize) return -4;
    crc = OHID_Checksum(Pack);
    if(crc==Pack->ohid.checksum)
    {
        return (OHID_HEAD_SIZE+Pack->ohid.length);
    }
    return -5;
}
int _OHIDS_Decode(union OpenAgreementHID_t* const Pack, const uint16_t bSize)
{
    uint8_t crc=0;
    if(bSize<OHID_HEAD_SIZE) return -1;
    if(OHID_PACK_HEAD_SLAVE!=Pack->ohid.HEAD) return -2;
    if(Pack->ohid.length>OHID_DATA_SIZE) return -3;
    if((Pack->ohid.length+(uint16_t)OHID_HEAD_SIZE)>bSize) return -4;
    crc = OHID_Checksum(Pack);
    if(crc==Pack->ohid.checksum)
    {
        return (OHID_HEAD_SIZE+Pack->ohid.length);
    }
    return -5;
}
int _OHIDM_decode(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint16_t bSize)
{
	int j;
    uint8_t crc=0;
    const union OpenAgreementHID_t* const OHID_Pack = (union OpenAgreementHID_t*)(bin);
    if(bSize<OHID_HEAD_SIZE) return -1;
    if(OHID_PACK_HEAD_MASTRT!=OHID_Pack->ohid.HEAD) return -2;
    if(OHID_Pack->ohid.length>=OHID_DATA_SIZE) return -3;
    if((OHID_Pack->ohid.length+(uint16_t)OHID_HEAD_SIZE)>bSize) return -4;
    crc = OHID_Checksum(OHID_Pack);
    if(crc==OHID_Pack->ohid.checksum)
    {
        for(j=0; j<(OHID_HEAD_SIZE+OHID_Pack->ohid.length); j++) Pack->bin[j] = OHID_Pack->bin[j];
        return (OHID_HEAD_SIZE+OHID_Pack->ohid.length);
    }
    return -5;
}
int _OHIDS_decode(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint16_t bSize)
{
	int j;
    uint8_t crc=0;
    const union OpenAgreementHID_t* const OHID_Pack = (union OpenAgreementHID_t*)(bin);
    if(bSize<OHID_HEAD_SIZE) return -1;
    if(OHID_PACK_HEAD_SLAVE!=OHID_Pack->ohid.HEAD) return -2;
    if(OHID_Pack->ohid.length>=OHID_DATA_SIZE) return -3;
    if((OHID_Pack->ohid.length+(uint16_t)OHID_HEAD_SIZE)>bSize) return -4;
    crc = OHID_Checksum(OHID_Pack);
    if(crc==OHID_Pack->ohid.checksum)
    {
        for(j=0; j<(OHID_HEAD_SIZE+OHID_Pack->ohid.length); j++) Pack->bin[j] = OHID_Pack->bin[j];
        return (OHID_HEAD_SIZE+OHID_Pack->ohid.length);
    }
    return -5;
}
int OHIDM_decode(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint16_t bSize)
{
    int len;
    uint16_t ii=0;
    if(bSize<OHID_HEAD_SIZE) return -1;
    ii=0;
    do {
        len = _OHIDM_decode(Pack, &bin[ii], bSize-ii);
        if(len>0) return len;
    } while(ii++<bSize-OHID_DATA_SIZE);
    return -2;
}
int OHIDS_decode(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint16_t bSize)
{
    int len;
    uint16_t ii=0;
    if(bSize<OHID_HEAD_SIZE) return -1;
    ii=0;
    do {
        len = _OHIDS_decode(Pack, &bin[ii], bSize-ii);
        if(len>0) return len;
    } while(ii++<bSize-OHID_DATA_SIZE);
    return -2;
}

/**************************************** data API **********************************************/
int OHID_copy(uint8_t bin[], const uint8_t src[], const uint16_t len)
{
    uint16_t ii;
    for(ii=0; ii<len; ii++) { bin[ii] = src[ii]; }
    return len;
}
int OHID_add_32b(union OpenAgreementHID_t* const Pack, const uint32_t value)
{
    union OHID_uint32_t Value;
    if((Pack->ohid.length+4)>OHID_DATA_SIZE) return -1;
    Value.data = value;
    Pack->ohid.data[Pack->ohid.length++] = Value.ohid.Byte0;
    
    Pack->ohid.data[Pack->ohid.length++] = Value.ohid.Byte1;
    Pack->ohid.data[Pack->ohid.length++] = Value.ohid.Byte2;
    Pack->ohid.data[Pack->ohid.length++] = Value.ohid.Byte3;
    return 4;
}
int OHID_add_16b(union OpenAgreementHID_t* const Pack, const uint16_t value)
{
    union OHID_uint16_t Value;
    if((Pack->ohid.length+2)>OHID_DATA_SIZE) return -1;
    Value.data = value;
    Pack->ohid.data[Pack->ohid.length++] = Value.ohid.Byte0;
    Pack->ohid.data[Pack->ohid.length++] = Value.ohid.Byte1;
    return 2;
}
int OHID_add_8b(union OpenAgreementHID_t* const Pack, const uint8_t value)
{
    if((Pack->ohid.length+1)>OHID_DATA_SIZE) return -1;
    Pack->ohid.data[Pack->ohid.length++] = value;
    return 1;
}
int OHID_add_bin(union OpenAgreementHID_t* const Pack, const uint8_t bin[], const uint8_t blen)
{
    if((Pack->ohid.length+1+blen)>OHID_DATA_SIZE) return -1;
    Pack->ohid.data[Pack->ohid.length++] = blen;
    Pack->ohid.length += OHID_copy(&Pack->ohid.data[Pack->ohid.length], bin, blen);
    return (blen+1);
}
uint32_t OHID_get_32b(const union OpenAgreementHID_t* const Pack, const uint8_t pos)
{
    uint32_t value=0;
    union OHID_uint32_t Bytes4;
    if((pos+4)>OHID_DATA_SIZE) return 0;
    Bytes4.ohid.Byte0 = Pack->ohid.data[pos+0];
    Bytes4.ohid.Byte1 = Pack->ohid.data[pos+1];
    Bytes4.ohid.Byte2 = Pack->ohid.data[pos+2];
    Bytes4.ohid.Byte3 = Pack->ohid.data[pos+3];
    value = Bytes4.data;
    return value;
}
uint16_t OHID_get_16b(const union OpenAgreementHID_t* const Pack, const uint8_t pos)
{
    uint32_t value=0;
    union OHID_uint16_t Bytes2;
    if((pos+2)>OHID_DATA_SIZE) return 0;
    Bytes2.ohid.Byte0 = Pack->ohid.data[pos+0];
    Bytes2.ohid.Byte1 = Pack->ohid.data[pos+1];
    value = Bytes2.data;
    return value;
}
uint8_t OHID_get_8b(const union OpenAgreementHID_t* const Pack, const uint8_t pos)
{
    uint32_t value=0;
    if((pos+1)>OHID_DATA_SIZE) return 0;
    value = Pack->ohid.data[pos];
    return value;
}
int OHID_get_bin(const union OpenAgreementHID_t* const Pack, const uint8_t pos, uint8_t bin[], const uint16_t bsize)
{
    uint32_t len=0;
    len = Pack->ohid.data[pos+0];
    if((pos+1+len)>OHID_DATA_SIZE) return -1;
    if(len>bsize) len=bsize;
    return OHID_copy(bin, &Pack->ohid.data[pos+1], len);
}

/************************** (C) COPYRIGHT Merafour **************************/
