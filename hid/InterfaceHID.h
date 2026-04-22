/******************** (C) COPYRIGHT 2018 merafour ********************
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.0.0
* Date               : 01/01/2024
* Description        : OpenAgreementHID.
* Description        : 通信接口
********************************************************************************
* merafour.blog.163.com
* merafour@163.com
* github.com/Merafour
*******************************************************************************/
#ifndef _INTERFACEHID_H_
#define _INTERFACEHID_H_

#include <stdint.h>
#include "hidapi/hidapi.h"

class InterfaceList
{

private:

#define LIST_MAX   256
#define PATH_LEN   256
    char Paths[LIST_MAX][PATH_LEN];
    uint16_t Size;

public:
    InterfaceList()
    {
        init();
    }
    void init()
    {
        uint16_t row, col;
        for(row=0; row<LIST_MAX; row++)
        {
            for(col=0; col<PATH_LEN; col++) Paths[row][col] = 0x00;
        }
        Size = 0;
    }

    int push(const char path[])
    {
        uint16_t index;
        if(Size>=(LIST_MAX)) return -1;
        for(index=0; index<sizeof(Paths[0]); index++)
        {
            Paths[Size][index] = path[index];
            if('\0' == path[index]) break;
        }
        Size++;
        return 0;
    }

    int pull(const uint8_t Index, char path[], const uint16_t size)
    {
        uint16_t ii;
        if(Index>=size) return -1;
        for(ii=0; ii<size; ii++)
        {
            path[ii] = Paths[Index][ii];
            if('\0' == path[ii]) break;
        }
        return 0;
    }

    int remove(const uint8_t Index)
    {
        uint16_t row, col, next;
        if(Index>=Size) return -1;
        for(row=Index; row<(Size-1); row++)
        {
            next = row+1;
            for(col=0; col<sizeof(Paths[0]); col++)
            {
                Paths[row][col] = Paths[next][col];
                Paths[next][col] = 0x00;
            }
        }
        Size--;
        return 0;
    }
    uint8_t size(void)
    {
        return Size;
    }
};

class InterfaceHID
{
public:
    InterfaceHID();
    ~InterfaceHID();
    void close(void);
    int Init(InterfaceList &list, const unsigned short vendor_id, const unsigned short product_id);
    int Init(InterfaceList &list, const unsigned short vendor_id, const unsigned short product_id, const int log);
    int Open(const char path[], const int log=0);
    int Writes(const unsigned char data[], const uint16_t length);
    int Reads(unsigned char data[], const uint16_t length)
    {
        if(NULL==handle) return 0;
        //return hid_read(handle, data, length);
        return hid_read_timeout(handle, data, length, 1);
    }
    //    int Reads(unsigned char data[], const uint16_t length, int timeout)
    //    {
    //        int len;
    //        int delay;
    //        if(NULL==handle) return 0;
    //        //return hid_read_timeout(handle, data, length, timeout);
    //        len = 0;
    //        for(delay=0; delay<timeout; delay+=10)
    //        {
    //            len = hid_read_timeout(handle, data, length, 10);
    //        }
    //        return len;
    //    }
    int Reads(unsigned char data[], const uint16_t length, int timeout)
    {
        if(NULL==handle) return 0;
        return hid_read_timeout(handle, data, length, timeout);
    }
    void clear(void)
    {
        uint8_t buf[64];
        if(NULL==handle) return ;
        while(1)
        {
            if(hid_read_timeout(handle, buf, sizeof(buf), 10)<=0) break;
        }
    }
    int is_open(void)
    {
        if(NULL==handle) return 0;
        return 1;
    }

    int GetProductString(wchar_t *string, size_t maxlen);
    int GetManufacturerString(wchar_t *string, size_t maxlen);

protected:
    hid_device *handle;

private:
};

#endif // _INTERFACEHID_H_
