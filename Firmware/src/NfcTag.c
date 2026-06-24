#include "NfcTag.h"
#include "main.h"
#include <stdio.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/util.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/pm/device.h>


#define NFC_BASE_ADDRESS ST25DV_I2C_USER_ADDR
#define NFC_SYSTEM_ADDRESS ST25DV_I2C_SYSTEM_ADDR
#define ST25DV_GPO_ENABLE 0x01
#define ST25DV_GPO_RF_WRITE_ENABLE 0x80
#define ST25DV_IT_STS_RF_WRITE 0x80
#define ST25DV_I2C_PRESENT_PASSWORD 0x09
#define ST25DV_EH_ON_DEMAND 0x01
#define ST25DV_EH_ENABLE 0x01
#define ST25DV_EH_DISABLE 0x00
#define ST25DV_MAX_I2C_WRITE 256
#define ST25DV_I2C_RETRY_COUNT 2
#define ST25DV_EEPROM_WRITE_MS 10

uint8_t NFC_I2C_ADDRESS;
const struct device *const i2c_nfc = DEVICE_DT_GET(DT_NODELABEL(i2c0));

/* RF/NFC passwords are independent; production changes only those. */
static const uint8_t st25dv_default_i2c_password[PASSWORD_LENGTH] = {0};
static bool st25dv_present_i2c_password(const uint8_t *passWd);
static uint8_t st25dv_read_byte_at(uint8_t i2c_address, unsigned int address, uint8_t *data);
static uint8_t st25dv_write_byte_at(uint8_t i2c_address, unsigned int address, uint8_t data);
static uint8_t st25dv_write_eeprom_byte_checked(uint8_t i2c_address, unsigned int address, uint8_t data);

void NfcTag_init(void)
{
    if (!device_is_ready(i2c_nfc))
    {
        return;
    }

    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
    NfcTag_setEnergyHarvesting(false);
    (void)st25dv_write_byte_at(NFC_BASE_ADDRESS, ST25DV_GPO_CTRL_DYN_ADDR, ST25DV_GPO_ENABLE);
    if (st25dv_present_i2c_password(st25dv_default_i2c_password))
    {
        (void)st25dv_write_eeprom_byte_checked(NFC_SYSTEM_ADDRESS, ST25DV_GPO1_ADDR,
                                               ST25DV_GPO_ENABLE | ST25DV_GPO_RF_WRITE_ENABLE);
        (void)st25dv_write_eeprom_byte_checked(NFC_SYSTEM_ADDRESS, ST25DV_EH_MODE_ADDR,
                                               ST25DV_EH_ON_DEMAND);
    }
    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
    (void)NfcTag_wasRfWriteEvent();
    k_msleep(10);
}

void NfcTag_GoSleep(void)
{

     pm_device_action_run(i2c_nfc, PM_DEVICE_ACTION_SUSPEND);
}

void NfcTag_Resume(void)
{

    pm_device_action_run(i2c_nfc, PM_DEVICE_ACTION_RESUME);
}

bool NfcTag_wasRfWriteEvent(void)
{
    uint8_t it_status;

    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
    if (st25dv_read_byte_at(NFC_BASE_ADDRESS, ST25DV_IT_STS_DYN_ADDR, &it_status) != 0)
    {
        return false;
    }

    return (it_status & ST25DV_IT_STS_RF_WRITE) != 0;
}

void NfcTag_setEnergyHarvesting(bool enable)
{
    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
    (void)st25dv_write_byte_at(NFC_BASE_ADDRESS, ST25DV_EH_CTRL_DYN_ADDR,
                               enable ? ST25DV_EH_ENABLE : ST25DV_EH_DISABLE);
}

static bool st25dv_present_i2c_password(const uint8_t *passWd)
{
    uint8_t sso;

    for (int attempt = 0; attempt < ST25DV_I2C_RETRY_COUNT; attempt++)
    {
        NfcTag_submitPassWd((uint8_t *)passWd);
        k_msleep(ST25DV_EEPROM_WRITE_MS);

        if ((st25dv_read_byte_at(NFC_BASE_ADDRESS, ST25DV_I2C_SSO_DYN_ADDR, &sso) == 0) &&
            ((sso & 0x01) != 0))
        {
            NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
            return true;
        }
    }

    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
    return false;
}

/**
 * Get full Configuration
 */
uint8_t NFCGetData(uint8_t *url, uint8_t *pwr, uint8_t *adv, uint8_t *ep)
{

    uint8_t dummy[50] = {0};
    uint8_t index = 0;
    uint8_t url_len = 0;
    uint8_t ret = NFC_ERROR_READ;

    *pwr = 6;
    *adv = 10;
    *ep = 1;
    if (NfcTag_readBytes(0x00, dummy, sizeof(dummy)) != 0)
    {
        url[0] = 0;
        return NFC_ERROR_READ;
    }
    if ((dummy[0] == 0xE1) && (dummy[4] == 0x03))
    {

        if ((dummy[8] > 0) && (dummy[9] == 0x55))
        {
            url_len = dummy[8] - 1;
            if (url_len >= URL_BUFFER)
            {
                url_len = URL_BUFFER - 1;
            }
            if ((uint16_t)11 + url_len > sizeof(dummy))
            {
                url[0] = 0;
                return NFC_ERROR_READ;
            }
            for (index = 0; index < url_len; index++)
            {

                url[index] = dummy[11 + index];
            }
            url[url_len] = 0;

            index = dummy[8] + 13;
            if (index >= sizeof(dummy))
            {
                return ret;
            }
            ret = NFC_ONLY_URL;
        }
        if (dummy[index] == 0x54)
        {
            index = index + 4;
            if (dummy[index] == 0x2B)
            {
                index += 1;
                *pwr = (dummy[index] - '0');
                index += 1;
                if (dummy[index] != 0x2B)
                {
                    *pwr = ((*pwr) * 10) + (dummy[index] - '0');
                    index += 1;
                }
                ret = NFC_COMPLETE;
            }
            if (dummy[index] == 0x2B)
            {
                index += 1;
                *adv = (dummy[index] - '0');
                index += 1;
                if (dummy[index] != 0x2B)
                {
                    *adv = ((*adv) * 10) + (dummy[index] - '0');
                    index += 1;
                }
                ret = NFC_COMPLETE;
            }

            if (dummy[index] == 0x2B)
            {
                index += 1;
                *ep = (dummy[index] - '0');
                ret = NFC_COMPLETE;
            }
        }
    }
    return ret;
}

void NfcTag_HideConfig(uint8_t lengh)
{
    (void)st25dv_write_eeprom_byte_checked(NFC_BASE_ADDRESS, 0x05, lengh + 5);
    (void)st25dv_write_eeprom_byte_checked(NFC_BASE_ADDRESS, 0x06, 0xD1);
}

void NfcTag_submitPassWd(uint8_t *passWd)
{
    uint8_t data[17];
    uint8_t ret;

    memcpy(&data[0], passWd, PASSWORD_LENGTH);
    data[8] = ST25DV_I2C_PRESENT_PASSWORD;
    memcpy(&data[9], passWd, PASSWORD_LENGTH);

    NFC_I2C_ADDRESS = NFC_SYSTEM_ADDRESS;
    for (int attempt = 0; attempt < ST25DV_I2C_RETRY_COUNT; attempt++)
    {
        ret = NfcTag_writeBytes(ST25DV_I2C_PWD_ADDR, data, sizeof(data));
        if (ret == 0)
        {
            break;
        }
        k_msleep(ST25DV_EEPROM_WRITE_MS);
    }
    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
}

void NfcTag_writePassWd(uint8_t *passWd)
{
    ARG_UNUSED(passWd);
}

// void NfcTag_sectorProtectConfig(unsigned int sectorNumber, bool protectEnable, enum SectorAccessRight accessRight = Access_1110, enum SectorSelectPassWd passWd = noPassWd)
void NfcTag_sectorProtectConfig(unsigned int sectorNumber, bool protectEnable, enum SectorAccessRight accessRight, enum SectorSelectPassWd passWd)
{
    ARG_UNUSED(sectorNumber);
    ARG_UNUSED(protectEnable);
    ARG_UNUSED(accessRight);
    ARG_UNUSED(passWd);
}

void NfcTag_clearSectorProtect(void)
{
}

void NfcTag_sectorWriteSockConfig(unsigned int sectorNumber, bool sockEnable)
{
    ARG_UNUSED(sectorNumber);
    ARG_UNUSED(sockEnable);
}

uint16_t NfcTag_getRFU()
{
    return 0;
}

uint8_t NfcTag_getUID(uint8_t *buf)
{
    NFC_I2C_ADDRESS = NFC_SYSTEM_ADDRESS;
    NfcTag_readBytes(UID_ADDR, buf, UID_LENGTH);
    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
    return UID_LENGTH;
}

uint32_t NfcTag_getMemoryVolume()
{
    uint16_t volume = 0x0;
    NFC_I2C_ADDRESS = NFC_SYSTEM_ADDRESS;
    volume = NfcTag_readByte(MEMORY_VOLUME_ADDR);
    volume = volume << 8 | NfcTag_readByte(MEMORY_VOLUME_ADDR + 1);
    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
    return ((uint32_t)volume + 1U) * 4U;
}

uint8_t NfcTag_getICNumber()
{
    uint8_t ic_number;
    NFC_I2C_ADDRESS = NFC_SYSTEM_ADDRESS;
    ic_number = NfcTag_readByte(IC_NUMBER_ADDR);
    NFC_I2C_ADDRESS = NFC_BASE_ADDRESS;
    return ic_number;
}

void NfcTag_clearMemory()
{
    for (int i = 0; i < EEPROM_I2C_LENGTH; i++)
    {
        NfcTag_writeByte(i, 0x0);
    }
}

uint8_t NfcTag_writeByte(unsigned int address, uint8_t data)
{
    return st25dv_write_byte_at(NFC_I2C_ADDRESS, address, data);
}

static uint8_t st25dv_write_byte_at(uint8_t i2c_address, unsigned int address, uint8_t data)
{
    uint8_t dummybuff[3];

    dummybuff[0] = (address >> 8) & 0xFF;
    dummybuff[1] = address & 0xFF;
    dummybuff[2] = data;

    for (int attempt = 0; attempt < ST25DV_I2C_RETRY_COUNT; attempt++)
    {
        if (i2c_write(i2c_nfc, dummybuff, 3, i2c_address) == 0)
        {
            return 0;
        }
        k_msleep(ST25DV_EEPROM_WRITE_MS);
    }

    return 1;
}

uint8_t NfcTag_readByte(unsigned int address)
{
    uint8_t data;

    if (st25dv_read_byte_at(NFC_I2C_ADDRESS, address, &data) != 0)
    {
        return 0xFF;
    }

    return data;
}

static uint8_t st25dv_read_byte_at(uint8_t i2c_address, unsigned int address, uint8_t *data)
{
    uint8_t dummybuff[2] = {0};

    dummybuff[0] = (address >> 8) & 0xFF;
    dummybuff[1] = address & 0xFF;

    for (int attempt = 0; attempt < ST25DV_I2C_RETRY_COUNT; attempt++)
    {
        if (i2c_write_read(i2c_nfc, i2c_address, dummybuff, sizeof(dummybuff), data, 1) == 0)
        {
            return 0;
        }
        k_msleep(ST25DV_EEPROM_WRITE_MS);
    }

    return 1;
}

static uint8_t st25dv_write_eeprom_byte_checked(uint8_t i2c_address, unsigned int address, uint8_t data)
{
    uint8_t readback;

    if ((st25dv_read_byte_at(i2c_address, address, &readback) == 0) && (readback == data))
    {
        return 0;
    }

    for (int attempt = 0; attempt < ST25DV_I2C_RETRY_COUNT; attempt++)
    {
        if (st25dv_write_byte_at(i2c_address, address, data) == 0)
        {
            k_msleep(ST25DV_EEPROM_WRITE_MS);
            if ((st25dv_read_byte_at(i2c_address, address, &readback) == 0) && (readback == data))
            {
                return 0;
            }
        }

        k_msleep(ST25DV_EEPROM_WRITE_MS);
    }

    return 1;
}

uint8_t NfcTag_writeBytes(unsigned int address, uint8_t *buf, unsigned int len)
{
    uint8_t buff[ST25DV_MAX_I2C_WRITE + 2];
    struct i2c_msg msgs[1];
    unsigned int i;

    if (len > ST25DV_MAX_I2C_WRITE)
    {
        return 1;
    }

    buff[0] = (address >> 8) & 0xFF;
    buff[1] = address & 0xFF;

    for (i = 0; i < len; i++)
    {
        buff[i + 2] = buf[i];
    }

    /* Setup I2C messages */

    /* Send the address to write to */
    msgs[0].buf = buff;
    msgs[0].len = len + 2;
    msgs[0].flags = I2C_MSG_WRITE;

    for (int attempt = 0; attempt < ST25DV_I2C_RETRY_COUNT; attempt++)
    {
        if (i2c_transfer(i2c_nfc, &msgs[0], 1, NFC_I2C_ADDRESS) == 0)
        {
            return 0;
        }
        k_msleep(ST25DV_EEPROM_WRITE_MS);
    }

    return 1;
}

uint8_t NfcTag_readBytes(unsigned int address, uint8_t *buf, unsigned int len)
{
    uint8_t dummybuff[3];

    dummybuff[0] = (address >> 8) & 0xFF;
    dummybuff[1] = address & 0xFF;

    for (int attempt = 0; attempt < ST25DV_I2C_RETRY_COUNT; attempt++)
    {
        if (i2c_write_read(i2c_nfc, NFC_I2C_ADDRESS, &dummybuff[0], 2, buf, len) == 0)
        {
            return 0;
        }
        k_msleep(ST25DV_EEPROM_WRITE_MS);
    }

    return 1;
}
