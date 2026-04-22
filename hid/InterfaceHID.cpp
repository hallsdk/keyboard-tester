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
#include "InterfaceHID.h"
#include <string.h>
#include <QThread>
#include <QDebug>

InterfaceHID::InterfaceHID()
{
    handle = nullptr;
}

InterfaceHID::~InterfaceHID()
{
    close();
    /* Free static InterfaceHIDAPI objects. */
    hid_exit();
}

void InterfaceHID::close()
{
    if(NULL!=handle)
    {
        hid_exit();
        hid_close(handle);
        handle = NULL;
    }
}

int InterfaceHID::Init(InterfaceList &list, const unsigned short vendor_id, const unsigned short product_id)
{
    struct hid_device_info *devs, *cur_dev;
    qDebug("\r\nhidapi test/example tool. Compiled with hidapi version %s, runtime version %s.\r\n", HID_API_VERSION_STR, hid_version_str());
    if (hid_version()->major == HID_API_VERSION_MAJOR && hid_version()->minor == HID_API_VERSION_MINOR && hid_version()->patch == HID_API_VERSION_PATCH) {
        qDebug("Compile-time version matches runtime version of hidapi.\r\n");
    }
    else {
        qDebug("Compile-time version is different than runtime version of hidapi.\r\n");
    }

    hid_exit();
    hid_init();
    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;
    while (cur_dev)
    {
        qDebug("hid_dev:[0x%04X-%04X 0x%04X-%04X]      %ls-%ls", vendor_id, product_id, cur_dev->vendor_id, cur_dev->product_id, cur_dev->manufacturer_string, cur_dev->product_string);
        if( (vendor_id==cur_dev->vendor_id) && (product_id==cur_dev->product_id) )
        {
            list.push(cur_dev->path);
            qDebug("--hid_dev:[0x%04X-%04X 0x%04X-%04X] size:%d path:%s", vendor_id, product_id, cur_dev->vendor_id, cur_dev->product_id, list.size(), cur_dev->path);
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);
    return 0;
}
int InterfaceHID::Init(InterfaceList &list, const unsigned short vendor_id, const unsigned short product_id, const int log)
{
    struct hid_device_info *devs, *cur_dev;
    if(log) qDebug("\r\nhidapi test/example tool. Compiled with hidapi version %s, runtime version %s.\r\n", HID_API_VERSION_STR, hid_version_str());
    if (hid_version()->major == HID_API_VERSION_MAJOR && hid_version()->minor == HID_API_VERSION_MINOR && hid_version()->patch == HID_API_VERSION_PATCH) {
        if(log) qDebug("Compile-time version matches runtime version of hidapi.\r\n");
    }
    else {
        if(log) qDebug("Compile-time version is different than runtime version of hidapi.\r\n");
    }

    hid_exit();
    hid_init();
    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;
    while (cur_dev)
    {
        if(log) qDebug("hid_dev:[0x%04X-%04X 0x%04X-%04X]      %ls-%ls", vendor_id, product_id, cur_dev->vendor_id, cur_dev->product_id, cur_dev->manufacturer_string, cur_dev->product_string);
        if( (vendor_id==cur_dev->vendor_id) && (product_id==cur_dev->product_id) )
        {
            list.push(cur_dev->path);
            if(log) qDebug("--hid_dev:[0x%04X-%04X 0x%04X-%04X] size:%d path:%s", vendor_id, product_id, cur_dev->vendor_id, cur_dev->product_id, list.size(), cur_dev->path);
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);
    return 0;
}

int InterfaceHID::Open(const char path[], const int log)
{
    int res;
#define MAX_STR 255
    wchar_t wstr[MAX_STR];

    if(log) qDebug("%s--%d: Open path: [%s]", __func__, __LINE__, path);
    handle = hid_open_path(path);
    if (!handle) {
        qDebug("unable to open device\n");
        return 1;
    }
    if(log) qDebug("%s--%d: Open Hid [%s] is OK", __func__, __LINE__, path);
    wstr[0] = 0x0000;
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    if (res < 0)
        qDebug("Unable to read manufacturer string\n");
    if(log) qDebug("Manufacturer String: %ls\n", wstr);

    // Read the Product String
    wstr[0] = 0x0000;
    res = hid_get_product_string(handle, wstr, MAX_STR);
    if (res < 0)
        qDebug("Unable to read product string\n");
    if(log) qDebug("Product String: %ls\n", wstr);

    // Read the Serial Number String
    wstr[0] = 0x0000;
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    if (res < 0)
        qDebug("Unable to read serial number string\n");
    if(log) qDebug("Serial Number String: (%d) %ls", wstr[0], wstr);
    if(log) qDebug("\n");

    // Read Indexed String 1
    wstr[0] = 0x0000;
    res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
    if (res < 0)
        qDebug("Unable to read indexed string 1\n");
    if(log) qDebug("Indexed String 1: %ls\n", wstr);

    // Set the hid_read() function to be non-blocking.
    hid_set_nonblocking(handle, 1);
    return 0;
}

int InterfaceHID::Writes(const unsigned char data[], const uint16_t length)
{
    static uint8_t bin[128];
    uint16_t len;
    const int LEN=64;
    int ret=-1;
    if(NULL==handle)
    {
        return 0;
    }
    for(len=0; len<sizeof(bin); len++) bin[len] = len;
    //qDebug("[%s--%d] length: %d \r\n", __func__, __LINE__, length);
#if 1
    for(len=0; len<length; len+=LEN)
    {
        bin[0] = 0x05;
        memcpy(&bin[1], &data[len], LEN);
        //memcpy(&bin[1], &data[len], 4);
        ret = hid_write(handle, bin, 1+LEN);
        if(ret<=0) return ret;
        // std::this_thread::sleep_for(std::chrono::milliseconds(2));s
    }
#else
    for(len=0; len<length; len+=LEN)
    {
        memcpy(&bin[0], &data[len], LEN);
        ret = hid_write(handle, bin, 1+LEN);
        if(ret<=0) return ret;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
#endif
    if(ret>0) return len;
    return ret;
}

int InterfaceHID::GetProductString(wchar_t *string, size_t maxlen)
{
    if (!handle) return -1;
    return hid_get_product_string(handle, string, maxlen);
}

int InterfaceHID::GetManufacturerString(wchar_t *string, size_t maxlen)
{
    if (!handle) return -1;
    return hid_get_manufacturer_string(handle, string, maxlen);
}

