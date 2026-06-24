#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/device.h>

#ifndef __NFC_TAG_H__
#define __NFC_TAG_H__

#define EEPROM_ADDR_E2_DISABLE 0x53
#define EEPROM_ADDR_E2_ENABLE 0x57
#define EEPROM_I2C_LENGTH 512

#define PASSWORD_LENGTH 8

#define LOCK_PROTECT_BIT BIT0
#define WRITE_READ_PROTECT_BIT BIT1
#define PASSWORD_CTRL_BIT BIT3

#define ST25DV_I2C_USER_ADDR 0x53
#define ST25DV_I2C_SYSTEM_ADDR 0x57

#define ST25DV_GPO_CTRL_DYN_ADDR 0x2000
#define ST25DV_I2C_SSO_DYN_ADDR 0x2004
#define ST25DV_IT_STS_DYN_ADDR 0x2005

#define ST25DV_GPO1_ADDR 0x0000
#define ST25DV_EH_MODE_ADDR 0x0002
#define ST25DV_I2C_PWD_ADDR 0x0900

#define ST25DV_EH_CTRL_DYN_ADDR 0x2002
#define LOCK_DSFID_ADDR 0x0010
#define LOCK_AFI_ADDR 0x0011
#define DSFID_ADDR 0x0012
#define AFI_ADDR 0x0013
#define MEMORY_VOLUME_ADDR 0x0014 // MEM_SIZE, 2 bytes
#define IC_NUMBER_ADDR 0x0017     // IC_REF, 1 byte
#define UID_ADDR 0x0018           // 8 bytes
#define UID_LENGTH 8

#define NFC_ERROR_READ 0
#define NFC_ONLY_URL 1
#define NFC_COMPLETE 2

enum AccessMode
{
    USER_MODE = 0x0, // offer simple read/write access right
    ROOT_MODE = 0x1, // offer password change access right
};

enum SectorAccessRight
{
    //      **********************************
    //      *  submit passWd *   no submit   *
    // b2,b1 *  Read * Write  *  Read * Write *
    // 00   *    1       1        1      0   *
    // 01   *    1       1        1      1   *
    // 10   *    1       1        0      0   *
    // 11   *    0       1        0      0   *
    //      **********************************
    Access_1110 = 0,
    Access_1111 = 1,
    Access_1100 = 2,
    Access_0111 = 3,
};

enum SectorSelectPassWd
{
    // 00 => no passWd protect
    // 01 => passWd 1
    // 10 => passWd 2
    // 11 => passwd 3
    noPassWd = 0,
    passWd_1 = 1,
    passWd_2 = 2,
    passWd_3 = 3,
};

void NfcTag_init(void);
void NfcTag_GoSleep(void);
void NfcTag_Resume(void);
bool NfcTag_wasRfWriteEvent(void);
void NfcTag_setEnergyHarvesting(bool enable);
uint8_t NfcGetURL(uint8_t *buff);
uint8_t NFCGetData(uint8_t *url, uint8_t *pwr, uint8_t *adv, uint8_t *ep);
void NfcTag_HideConfig(uint8_t lengh);
void NfcTag_submitPassWd(uint8_t *passWd);
void NfcTag_writePassWd(uint8_t *passWd);
void NfcTag_sectorProtectConfig(unsigned int sectorNumber, bool protectEnable, enum SectorAccessRight accessRight, enum SectorSelectPassWd passWd);
void NfcTag_clearSectorProtect(void);
void NfcTag_sectorWriteSockConfig(unsigned int sectorNumber, bool sockEnable);
uint16_t NfcTag_getRFU();
uint8_t NfcTag_getUID(uint8_t *buf);
uint32_t NfcTag_getMemoryVolume();
uint8_t NfcTag_getICNumber();
void NfcTag_clearMemory();
uint8_t NfcTag_readByte(unsigned int address);
uint8_t NfcTag_writeByte(unsigned int address, uint8_t data);
uint8_t NfcTag_writeBytes(unsigned int address, uint8_t *buf, unsigned int len);
uint8_t NfcTag_readBytes(unsigned int address, uint8_t *buf, unsigned int len);

#endif
