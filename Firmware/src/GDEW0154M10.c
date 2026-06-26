#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/pm/device.h>

#include "GDEW0154M10.h"
#include "epaper.h"

const struct gpio_dt_spec rst_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(epreset), gpios);
const struct gpio_dt_spec DC_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(epdc), gpios);
const struct gpio_dt_spec busy_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(epbusy), gpios);
const struct gpio_dt_spec cs_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(csgpios), gpios);

const struct device *const spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi0));
// struct spi_cs_control *ctrl = SPI_CS_CONTROL_PTR_DT(DT_NODELABEL(spi_dev), 2);
struct spi_cs_control cs_ctrl;
struct spi_config spi_cfg = {0};

#define GDE_SET_CMD() gpio_pin_set_dt(&DC_pin, 0)
#define GDE_SET_DATA() gpio_pin_set_dt(&DC_pin, 1)
#define GDE_GET_BUSY() gpio_pin_get_dt(&busy_pin)
#define GDE_CS_HIGH() gpio_pin_set_dt(&cs_pin, 1)
#define GDE_CS_LOW() gpio_pin_set_dt(&cs_pin, 0)

void GDE_Init(void)
{

    if (!device_is_ready(spi_dev))
    {
        return;
    }

    spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE | SPI_FULL_DUPLEX;
    spi_cfg.frequency = 8000000U;
   // spi_cfg.cs = 0; // &cs_ctrl;

    gpio_pin_configure_dt(&rst_pin, GPIO_OUTPUT);
    gpio_pin_configure_dt(&DC_pin, GPIO_OUTPUT);
    gpio_pin_configure_dt(&busy_pin, GPIO_INPUT);
    gpio_pin_configure_dt(&cs_pin, GPIO_OUTPUT);

    GDE_CS_HIGH();
    GDE_Reset();

    GDE_Get_Busy(1000);

    GDE_WriteCMD(BOOSTER_SOFT_START);
    GDE_WriteData(0x17);
    GDE_WriteData(0x17);
    GDE_WriteData(0x17);

    GDE_WriteCMD(POWER_SETTING);
    GDE_WriteData(0x03);
    GDE_WriteData(0x00);
    GDE_WriteData(0x2B);
    GDE_WriteData(0x2B);
    GDE_WriteData(0x09);

    GDE_WriteCMD(POWER_ON);

    GDE_WriteCMD(PANEL_SETTING);
    // GDE_WriteData(0xDF); // LUT from OTB.
    GDE_WriteData(0x1F);

    GDE_WriteCMD(TCON_RESOLUTION);
    GDE_WriteData(0x98);
    GDE_WriteData(0x00);
    GDE_WriteData(0x98);

    GDE_WriteCMD(VCOM_AND_DATA_INTERVAL_SETTING);
    //       GDE_WriteData(0x8F);
    // GDE_WriteData(0xF7);
    GDE_WriteData(0x97);

    GDE_WriteCMD(VCM_DC_SETTING_REGISTER);
    GDE_WriteData(0x0A);

    GDE_clear();
}

void GDE_Refresh(void)
{
    GDE_WriteCMD(DISPLAY_REFRESH);
    k_msleep(15);
}

void GDE_Sleep(void)
{
    //GDE_WriteCMD(POWER_OFF);
    //GDE_Get_Busy(2000);
   // k_msleep(1000);
    GDE_WriteCMD(DEEP_SLEEP);
    GDE_WriteData(0xA5);

}

void GDE_Off(void)
{
    pm_device_action_run(spi_dev, PM_DEVICE_ACTION_SUSPEND);

    /* Porta tutti i pin in input per evitare correnti parassite
     * quando l'alimentazione del display è spenta. */
    gpio_pin_configure_dt(&rst_pin, GPIO_INPUT);
    gpio_pin_configure_dt(&DC_pin, GPIO_INPUT);
    gpio_pin_configure_dt(&busy_pin, GPIO_INPUT);
    gpio_pin_configure_dt(&cs_pin, GPIO_INPUT);
}

void GDE_Reset(void)
{
    gpio_pin_set_dt(&rst_pin, 0);
    k_msleep(200);
    gpio_pin_set_dt(&rst_pin, 1);
    k_msleep(200);
}

void GDE_FixReset(void)
{
    gpio_pin_configure_dt(&rst_pin,GPIO_OUTPUT);
    gpio_pin_set_dt(&rst_pin, 0);
}
bool GDE_Get_Busy(uint16_t timeout)
{
    while (GDE_GET_BUSY() == 0)
    {
        k_msleep(1);
        timeout--;
        if (timeout == 0)
        {
            return false;
        }
    }

    return true;
}

void GDE_WriteCMD(uint8_t cmd)
{
    uint8_t dummybf[1];
    dummybf[0] = cmd;
    struct spi_buf bufs[1];
    struct spi_buf_set tx;

    GDE_SET_CMD();
    bufs->buf = dummybf;
    bufs->len = 1;
    tx.buffers = bufs;
    tx.count = 1;

    GDE_Get_Busy(6000);

    GDE_CS_LOW();
    spi_write(spi_dev, &spi_cfg, &tx);
    GDE_CS_HIGH();
}

void GDE_WriteData(uint8_t data)
{

    uint8_t dummybf[1];
    dummybf[0] = data;
    struct spi_buf bufs[1];
    struct spi_buf_set tx;

    GDE_SET_DATA();
    bufs->buf = dummybf;
    bufs->len = 1;
    tx.buffers = bufs;
    tx.count = 1;
    GDE_CS_LOW();
    spi_write(spi_dev, &spi_cfg, &tx);
    GDE_CS_HIGH();
}

void GDE_StartWriteDataBuffer(void)
{
    GDE_WriteCMD(DATA_START_TRANSMISSION_2);
    GDE_SET_DATA();
    GDE_CS_LOW();
}

void GDE_AppendWriteDataBuffer(uint8_t *data, uint16_t size)
{
    struct spi_buf bufs;
    struct spi_buf_set tx;

    bufs.buf = data;
    bufs.len = size;

    tx.buffers = &bufs;
    tx.count = 1;
    GDE_CS_LOW();
    spi_write(spi_dev, &spi_cfg, &tx);

    GDE_CS_HIGH();
}

void GDE_EndWriteDataBuffer(void)
{
    GDE_CS_HIGH();
}


void GDE_clear(void)
{
    GDE_WriteCMD(DATA_START_TRANSMISSION_1);
    for (uint16_t i = 0; i < 2889; i++)
    {
        GDE_WriteData(0xFF);
    }
    GDE_WriteCMD(DATA_START_TRANSMISSION_2);
    for (uint16_t i = 0; i < 2889; i++)
    {
        GDE_WriteData(0xFF);
    }
}