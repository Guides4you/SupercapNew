#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/device.h>


    /*
     * Command registers available from the manufacturer documentation
     */
    #define PANEL_SETTING                               0x00
    #define POWER_SETTING                               0x01
    #define POWER_OFF                                   0x02
    #define POWER_OFF_SEQUENCE_SETTING                  0x03
    #define POWER_ON                                    0x04
    #define POWER_ON_MEASURE                            0x05
    #define BOOSTER_SOFT_START                          0x06
    #define DEEP_SLEEP                                  0x07
    #define DATA_START_TRANSMISSION_1                   0x10
    #define DATA_STOP                                   0x11
    #define DISPLAY_REFRESH                             0x12
    #define DATA_START_TRANSMISSION_2                   0x13
    #define PLL_CONTROL                                 0x30
    #define TEMPERATURE_SENSOR_COMMAND                  0x40
    #define TEMPERATURE_SENSOR_CALIBRATION              0x41
    #define TEMPERATURE_SENSOR_WRITE                    0x42
    #define TEMPERATURE_SENSOR_READ                     0x43
    #define VCOM_AND_DATA_INTERVAL_SETTING              0x50
    #define LOW_POWER_DETECTION                         0x51
    #define TCON_SETTING                                0x60
    #define TCON_RESOLUTION                             0x61
    #define SOURCE_AND_GATE_START_SETTING               0x62
    #define GET_STATUS                                  0x71
    #define AUTO_MEASURE_VCOM                           0x80
    #define VCOM_VALUE                                  0x81
    #define VCM_DC_SETTING_REGISTER                     0x82
    #define PARTIAL_WINDOW                              0x90
    #define PARTIAL_IN                                  0x91
    #define PARTIAL_OUT                                 0x92
    #define PROGRAM_MODE                                0xA0
    #define ACTIVE_PROGRAMMING                          0xA1
    #define READ_OTP                                    0xA2
    #define POWER_SAVING                                0xE3


#define GxEPD_BLACK     0x0000
#define GxEPD_DARKGREY  0x7BEF      /* 128, 128, 128 */
#define GxEPD_LIGHTGREY 0xC618      /* 192, 192, 192 */
#define GxEPD_WHITE     0xFFFF
#define GxEPD_RED       0xF800      /* 255,   0,   0 */

void GDE_Init(void);
void GDE_Reset(void);
void GDE_FixReset(void);
bool GDE_Get_Busy(uint16_t timeout);
void GDE_WriteCMD(uint8_t cmd);
void GDE_WriteData(uint8_t data);
void GDE_StartWriteDataBuffer(void);
void GDE_AppendWriteDataBuffer(uint8_t *data, uint16_t size);
void GDE_EndWriteDataBuffer(void);
void GDE_Refresh(void);
void GDE_fill_rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t colour);
void GDE_clear(void);
void GDE_Sleep(void);
void GDE_Off(void);


