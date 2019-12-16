#ifndef _BEESIGN_H_
#define _BEESIGN_H_

#include "stm32f0xx.h"

#include <stdint.h>

#define BEESIGN_MIN_BAND 1
#define BEESIGN_MAX_BAND 5
#define BEESIGN_MIN_CHANNEL 1
#define BEESIGN_MAX_CHANNEL 8

#define BEESIGN_VTX_RACE_MODE 0
#define BEESIGN_VTX_MANUAL_MODE 1
#define BEESIGN_VTX_POR_MODE 2
#define BEESIGN_VTX_MODE_COUNT 3

#define BEESIGN_BAND_COUNT (BEESIGN_MAX_BAND - BEESIGN_MIN_BAND + 1)
#define BEESIGN_CHANNEL_COUNT (BEESIGN_MAX_CHANNEL - BEESIGN_MIN_CHANNEL + 1)
#define BEESIGN_ERROR_CHANNEL 0xff

#define BEESIGN_CMD_ADD_BUFF 0
#define BEESIGN_CMD_SEND 1

// #define VTX_PWR_25                  0
// #define VTX_PWR_100                 1
// #define VTX_PWR_200                 2
// #define VTX_PWR_400                 3
// #define VTX_PWR_600                 4
// #define VTX_PWR_PIT                 5
// #define BEESIGN_POWER_COUNT         6

#define VTX_PWR_0_1 0
#define VTX_PWR_25 1
#define BEESIGN_POWER_COUNT 2

#define BEESIGN_DEFAULT_POWER 1
#define BEESIGN_MIN_POWER 1

#define BEESIGN_POR_FREQUENCY_MHZ 5584

#define BEESIGN_MIN_FREQUENCY_MHZ 5000 //min freq in MHz
#define BEESIGN_MAX_FREQUENCY_MHZ 5999 //max freq in MHz

#define BEESIGN_HDR 0xAB
#define BEESIGN_FM_MAX_LEN 64 // frame max length
#define BEESIGN_PL_MAX_LEN 60 // payload max length

#define BEESIGN_BAUD_RATE 115200

#define BEESIGN_OK 0
#define BEESIGN_ERROR 1

#define BEESIGN_TYPE_MASK_FM 0x80  // Bit - 7
#define BEESIGN_TYPE_MASK_CMD 0x7F // Bit - 1 ~ 6

#define BEESIGN_FM_TYPE_CMD 0 // command frame
#define BEESIGN_FM_TYPE_RSP 1 // response frame

#define BEESIGN_PARA_DISP_OFF 0
#define BEESIGN_PARA_DISP_ON 1

#define BEESIGN_VOL_SRC_ADC 1
#define BEESIGN_VOL_SRC_EXT 2
#define BEESIGN_RSSI_SRC_ADC 1
#define BEESIGN_RSSI_SRC_EXT 2
#define BEESIGN_RSSI_SRC_RX 3

#define BEESIGN_VTX_MODE_RACE 0
#define BEESIGN_VTX_MODE_MANU 1
#define BEESIGN_VTX_MODE_POR 2

#define BEESIGN_VTX_UNLOCK 0x403C
#define BEESIGN_VTX_LOCK 0x403D
#define BEESIGN_VTX_MAX_CHAN 40
#define BEESIGN_VTX_POR_FREQ 5584

#define BEESIGN_OSD_MODE_OFF 0
#define BEESIGN_OSD_MODE_MINI 1
#define BEESIGN_OSD_MODE_CUSTOM 2

#define BEESIGN_OSD_HOS_MAX 63
#define BEESIGN_OSD_VOS_MAX 31

#define BEESIGN_CHARS_PER_SCREEN 250

#define BEESIGN_CHARS_MAX 0xBF
#define BEESIGN_CHARS_UNLOCK_ADDR_MIN 0xA0
#define BEESIGN_CHARS_UNLOCK 0xABBA
#define BEESIGN_CHARS_FONT_LEN 36
#define BEESIGN_CHARS_PER_LINE 25
#define BEESIGN_LINES_PER_SCREEN 10

// VTX commands
#define BEESIGN_V_GET_STATUS 0x10
#define BEESIGN_V_SET_CHAN 0x11
#define BEESIGN_V_SET_FREQ 0x12
#define BEESIGN_V_SET_PWR 0x13
#define BEESIGN_V_SET_MODE 0x14
#define BEESIGN_V_UNLOCK 0x15
#define BEESIGN_V_LOCK 0x16

// OSD commands
#define BEESIGN_O_GET_STATUS 0x20
#define BEESIGN_O_SET_MODE 0x21
#define BEESIGN_O_SET_LAYOUT 0x22
#define BEESIGN_O_SET_HOS 0x23
#define BEESIGN_O_SET_VOS 0x24
#define BEESIGN_O_SET_DISPLAY 0x25
#define BEESIGN_O_CLR_DISPLAY 0x26
#define BEESIGN_O_UDT_FONT 0x27
#define BEESIGN_O_FONT_UNLOCK 0x29

// Audio commands
#define BEESIGN_A_GET_STATUS 0x30
#define BEESIGN_A_SET_VOL 0x31
#define BEESIGN_A_SET_RSSI 0x32

// Other commands
#define BEESIGN_S_GET_STATUS 0x70
#define BEESIGN_S_SET_VOL 0x71
#define BEESIGN_S_SET_RSSI 0x72
#define BEESIGN_M_SET_NAME 0x73
#define BEESIGN_M_SET_VOL 0x74
#define BEESIGN_M_SET_CUR 0x75
#define BEESIGN_M_SET_RSSI 0x76
#define BEESIGN_M_SAVE_SETTING 0x77

#define BEESIGN_VOLTAGE_ADC 0
#define BEESIGN_VOLTAGE_EXT 1

#define BEESIGN_RSSI_ADC 0
#define BEESIGN_RSSI_EXT 1

#define BEESIGN_PKT_TYPE(x) (x)[1]
#define BEESIGN_PKT_LEN(x) (x)[2]
#define BEESIGN_PKT_DATA(x, i) (x)[3 + (i)]

#define BS_DEVICE_CHVAL_TO_BAND(val) ((val) / (uint8_t)8)
#define BS_DEVICE_CHVAL_TO_CHANNEL(val) ((val) % (uint8_t)8)
#define BS_BANDCHAN_TO_DEVICE_CHVAL(band, channel) ((band - 1) * (uint8_t)8 + (channel - 1))

/***********************************************
 * The data frame have five parts: header, type, length, payload and CRC, and the maxium length is 64 bytes.
 * The header is fixed to 0xAB; the type segment Bit 7 of the TYPE byte is frame type (0 = command frame,
 * 1 = response frame), bit 0~6 is ID; And the LENGTH is the length of the payload (valid range is 1~60).
 * The frame format is shown in the following table.
 *
 * |------------------------------------------------------------------------------------------|
 * | header (1 byte) | type (1 byte) | length (1 byte) | payload (<= 60 bytes) | CRC (1 byte) |
 * |------------------------------------------------------------------------------------------|
 *
************************************************/
typedef struct BEESIGN_FRAME_T {
   uint8_t hdr;
   uint8_t type;
   uint8_t len;
   uint8_t *payload;
   uint8_t crc;
} beesign_frame_t;

typedef struct beeSignDeviceVtx_s {
   uint8_t channel;
   uint8_t power;
   uint8_t mode;
} beeSignDeviceVtx_t;

typedef struct beeSignDeviceOsd_s {
   uint8_t voltagePosition;
   uint8_t rssiPosition;
   uint8_t namePosition;
   uint8_t currentPosition;
   uint8_t fmodePosition;
   uint8_t ftimePosition;
   uint8_t vtxPosition;
} beeSignDeviceOsd_t;

typedef struct beeSignDeviceOthers_s {
   char name[8];
} beeSignDeviceOthers_t;

typedef struct beeSignDevice_s {
   uint8_t blankFlg;
   beeSignDeviceVtx_t vtx;
   beeSignDeviceOsd_t osd;
   beeSignDeviceOthers_t others;
} beeSignDevice_t;

extern beeSignDevice_t bsDevice;

void beesignInit(void);
void beesignTask(void);

uint8_t bsValidateBandAndChannel(uint8_t band, uint8_t channel);
void bsSetBandAndChannel(uint8_t band, uint8_t channel, uint8_t cmd);
void bsSetPower(uint8_t index, uint8_t cmd);
uint8_t bsValidateFreq(uint16_t freq);
void bsSetFreq(uint16_t freq, uint8_t cmd);
void bsSetName(char *name, uint8_t len, uint8_t cmd);
void bsSaveSetting(uint8_t cmd);

void bsSetOsdMode(uint8_t mode, uint8_t cmd);
void bsSetMiniLayout(uint8_t VolPostion, uint8_t RssiPostion, uint8_t NamePostion, uint8_t CurPostion, uint8_t FModePosition, uint8_t FTimePosition, uint8_t VtxPosition, uint8_t cmd);
void bsClearDispaly(uint8_t cmd);
void bsSetDisplayOneRow(uint8_t x, uint8_t y, const char *buff, uint8_t cmd);

#endif
