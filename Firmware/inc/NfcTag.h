#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/device.h>

#ifndef __NFC_TAG_H__
#define __NFC_TAG_H__


#define EEPROM_ADDR_E2_DISABLE  0x53 //disable E2 - MODIFICATO
#define EEPROM_ADDR_E2_ENABLE   (0x53|0x04) //enable E2 
#define EEPROM_I2C_LENGTH   8192

#define PASSWORD_LENGTH 4
#define SECTOR_SECURITY_STATUS_BASE_ADDR    0x800 //2048

#define LOCK_PROTECT_BIT        BIT0
#define WRITE_READ_PROTECT_BIT  BIT1
#define PASSWORD_CTRL_BIT       BIT3

#define I2C_PASSWORD_ADDR   2304
#define RF_PASSWORD_1_ADDR  2308
#define RF_PASSWORD_2_ADDR  2312
#define RF_PASSWORD_3_ADDR  2316
#define DSFID_ADDR          2320    //1 byte
#define AFI_ADDR            2321    //1 byte
#define RFU_ADDR            2322    //2 bytes
#define UID_ADDR            2324    //8 bytes
#define UID_LENGTH          8
#define MEMORY_VOLUME_ADDR  2332    //3 bytes
#define IC_NUMBER_ADDR      2335    //1 byte


enum AccessMode{
    USER_MODE = 0x0,    // offer simple read/write access right
    ROOT_MODE = 0x1,    // offer password change access right
};

enum SectorAccessRight{
    //      **********************************
    //      *  submit passWd *   no submit   * 
    //b2,b1 *  Read * Write  *  Read * Write *
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

enum SectorSelectPassWd{
    //00 => no passWd protect
    //01 => passWd 1
    //10 => passWd 2
    //11 => passwd 3
    noPassWd = 0,
    passWd_1 = 1,
    passWd_2 = 2,
    passWd_3 = 3,
};


    void NfcTag_init(const struct device * dev);
    uint8_t NfcGetURL(uint8_t *buff);
    void NfcTag_submitPassWd(uint8_t* passWd);
    void NfcTag_writePassWd(uint8_t* passWd);
    void NfcTag_sectorProtectConfig(unsigned int sectorNumber, bool protectEnable, enum SectorAccessRight accessRight, enum SectorSelectPassWd passWd);
    void NfcTag_clearSectorProtect(void);
    void NfcTag_sectorWriteSockConfig(unsigned int sectorNumber, bool sockEnable);
    static uint8_t NfcTag_getDSFID();
    static uint8_t NfcTag_getAFI();
    uint16_t NfcTag_getRFU();
    uint8_t NfcTag_getUID(uint8_t* buf);
    uint32_t NfcTag_getMemoryVolume();
    uint8_t NfcTag_getICNumber();
    void NfcTag_clearMemory();
    void NfcTag_PrintMemory(void);
    uint8_t NfcTag_readByte(unsigned int address);
    uint8_t NfcTag_writeByte(unsigned int address, uint8_t data);
    uint8_t NfcTag_writeBytes(unsigned int address, uint8_t* buf, unsigned int len);
   uint8_t NfcTag_readBytes(unsigned int address, uint8_t *buf, unsigned int len);
    

#endif
