#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "GDEW0154M10.h"
#include "epaper.h"
#include "qrcode.h"

#define QRCODE_ERROR_ECC ECC_HIGH
#define qrcode_BufferSize(v)    ((((4 * v + 17) * (4 * v + 17)) + 7) / 8)            


QRCode qrcode;
uint8_t qr_step = 0;
uint8_t qrversion;
uint8_t qrcodeData[qrcode_BufferSize(5)];

void Epaper_Init(void)
{

    GDE_Init();
}

void Epaper_InitQR(uint8_t *buff)
{

    uint8_t len;

    len = strlen(buff) + 8;

    if (len < 10)
    {
        qrversion = 1;
        qr_step = 7;
    }
    else
    {
        if (len < 20)
        {
            qrversion = 2;
            qr_step = 6;
        }
        else
        {
            if (len < 35)
            {
                qrversion = 3;
                qr_step = 5;
            }
            else
            {
                if (len < 64)
                {
                    qrversion = 5;
                    qr_step = 4;
                }
                else
                {
                    return;
                }
            }
        }
    }
    qrcode_initText(&qrcode, qrcodeData, qrversion, QRCODE_ERROR_ECC, buff);
}

void Epaper_PrintQR(void)
{
    uint8_t z, y, x;
    uint8_t epbuff[19] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t ptr, ptr_start;
    bool col;

    /* Alignment the QR in X axes ti the middle*/
    ptr_start = (152 - (qrcode.size * qr_step)) >> 1;

    GDE_StartWriteDataBuffer();

    /* Alignment the QR in Y axes ti the middle*/
    for (z = 0; z < ptr_start; z++)
    {
        GDE_AppendWriteDataBuffer(epbuff, 19);
    }

    for (y = 0; y < qrcode.size; y++)
    {
        epbuff[18] = 0xff;
        epbuff[0] = 0xff;
        ptr = ptr_start;
        for (x = 0; x < qrcode.size; x++)
        {
            col = !qrcode_getModule(&qrcode, x, y);
            for (z = 0; z < qr_step; z++)
            {
                Epaper_SetBitArray(ptr, col, epbuff);
                ptr++;
            }
        }
        for (z = 0; z < qr_step; z++)
        {
            GDE_AppendWriteDataBuffer(epbuff, 19);
        }
    }

    GDE_EndWriteDataBuffer();
    GDE_Refresh();
}

void Epaper_SetBitArray(uint8_t x, bool c, uint8_t *buff)
{
    uint8_t a = 0, b = 0;
    a = x >> 3;
    b = 7 - (x - (a * 8));
    buff[a] = (buff[a] & ~(1UL << b)) | (c << b);
}

void Epaper_Gosleep(void)
{
    GDE_Get_Busy(15000);
    GDE_Sleep();
    GDE_Off();
}