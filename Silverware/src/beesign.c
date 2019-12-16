#include "config.h"
#include "stm32f0xx.h"
#if defined(USE_BEESIGN)
#include "beesign.h"
#include "drv_softserial.h"
#include "menu.h"
#include "stdlib.h"
#include "string.h"

extern float vbattfilt;
extern int rx_rssi;

#define BEESIGN_TX_PORT GPIOB
#define BEESIGN_TX_PIN GPIO_Pin_4
#define BEESIGN_RX_PORT GPIOB
#define BEESIGN_RX_PIN GPIO_Pin_3
#define BEESIGN_BAUDRATE 115200

#define BEESIGN_CMD_BUFF_SIZE 150

#define MAX_CHARS2UPDATE 100
#define POLY (0xB2)
#define VOL_AND_RSSI_SEND_COUNT 500

#define BEESIGN_GET_FC_VOLTAGE() (vbattfilt * 10)
#define BEESIGN_GET_FC_RSSI() (rx_rssi / 20.47)

#define CALC_CRC(crc, data)                               \
   do {                                                   \
      crc ^= (data);                                      \
      crc = (crc & 1) ? ((crc >> 1) ^ POLY) : (crc >> 1); \
      crc = (crc & 1) ? ((crc >> 1) ^ POLY) : (crc >> 1); \
      crc = (crc & 1) ? ((crc >> 1) ^ POLY) : (crc >> 1); \
      crc = (crc & 1) ? ((crc >> 1) ^ POLY) : (crc >> 1); \
      crc = (crc & 1) ? ((crc >> 1) ^ POLY) : (crc >> 1); \
      crc = (crc & 1) ? ((crc >> 1) ^ POLY) : (crc >> 1); \
      crc = (crc & 1) ? ((crc >> 1) ^ POLY) : (crc >> 1); \
      crc = (crc & 1) ? ((crc >> 1) ^ POLY) : (crc >> 1); \
   } while (0)

typedef enum {
   BS_STATE_HDR = 0,
   BS_STATE_TYPE,
   BS_STATE_LEN,
   BS_STATE_PAYLOAD,
   BS_STATE_CRC
} bsState_e;

beeSignDevice_t bsDevice;

static uint8_t beesignCmdQueue[BEESIGN_CMD_BUFF_SIZE];
static uint8_t *beesignBuffPointer = beesignCmdQueue;
static uint8_t *beesignSendPointer = beesignCmdQueue;
static uint8_t beesignCmdCount;
//static SoftSerialData_t beesignSeiralPort;
//static uint8_t receiveBuffer[BEESIGN_FM_MAX_LEN];
//static uint8_t receiveFrame[BEESIGN_FM_MAX_LEN];
// static uint8_t bsOsdMode = BEESIGN_OSD_MODE_MINI;

static uint8_t beesignCRC(const beesign_frame_t *pPackage) {
   uint8_t i;
   uint8_t crc = 0;

   CALC_CRC(crc, pPackage->hdr);
   CALC_CRC(crc, pPackage->type);
   CALC_CRC(crc, pPackage->len);
   for (i = 0; i < pPackage->len; i++) {
      CALC_CRC(crc, *(pPackage->payload + i));
   }
   return crc;
}

static uint8_t *beesignCmdAfterPointer(uint8_t *p, uint8_t afterLen) {
   if (p + afterLen >= (beesignCmdQueue + BEESIGN_CMD_BUFF_SIZE)) {
      return p + afterLen - BEESIGN_CMD_BUFF_SIZE;
   } else {
      return p + afterLen;
   }
}

static uint8_t beesignCmdGoNextPointer(uint8_t **p, uint8_t data) {
   uint8_t res = **p;
   **p = data;
   (*p)++;
   if (*p >= (beesignCmdQueue + BEESIGN_CMD_BUFF_SIZE)) {
      (*p) = beesignCmdQueue;
   }
   return res;
}

static uint8_t beesignAddCmd(uint8_t id, uint8_t len, uint8_t *pData) {
   beesign_frame_t package = {.hdr = BEESIGN_HDR,
                              .type = id,
                              .len = len,
                              .payload = pData};

   if ((len >= BEESIGN_PL_MAX_LEN) ||
       (pData == 0)) {
      return BEESIGN_ERROR;
   }
   if ((beesignBuffPointer - beesignSendPointer > 0 && beesignSendPointer + BEESIGN_CMD_BUFF_SIZE - beesignBuffPointer < len) ||
       (beesignSendPointer - beesignBuffPointer > 0 && beesignSendPointer - beesignBuffPointer < len)) { // don't have enough buff
      return BEESIGN_ERROR;
   }
   package.crc = beesignCRC(&package);

   beesignCmdCount++;
   beesignCmdGoNextPointer(&beesignBuffPointer, package.hdr);
   beesignCmdGoNextPointer(&beesignBuffPointer, package.type);
   beesignCmdGoNextPointer(&beesignBuffPointer, package.len);
   for (uint8_t i = 0; i < len; i++) {
      beesignCmdGoNextPointer(&beesignBuffPointer, *(package.payload + i));
   }
   beesignCmdGoNextPointer(&beesignBuffPointer, package.crc);
   return BEESIGN_OK;
}

static uint8_t beesignSendCmd(void) {
   if (beesignCmdCount > 0) {
      if (*beesignSendPointer == BEESIGN_HDR) {
         uint8_t crc = 0;
         uint8_t crcCheck = 0xff;
         uint8_t cmdLen = *beesignCmdAfterPointer(beesignSendPointer, 2);
         for (uint8_t i = 0; i < cmdLen + 3; i++) {
            CALC_CRC(crc, *beesignCmdAfterPointer(beesignSendPointer, i));
            crcCheck = *beesignCmdAfterPointer(beesignSendPointer, i + 1);
         }
         if (crc == crcCheck) {
            for (uint8_t i = 0; i < cmdLen + 4; i++) {
               softserial_write_byte(beesignCmdGoNextPointer(&beesignSendPointer, 0));
            }
            beesignCmdCount--;
            return BEESIGN_OK;
         }
      } else {
         beesignSendPointer++;
      }
   }
   return BEESIGN_ERROR;
}

static uint8_t beesignSend(uint8_t id, uint8_t len, uint8_t *pData, uint8_t cmd) {
   if (cmd == BEESIGN_CMD_ADD_BUFF) {
      return beesignAddCmd(id, len, pData);
   } else {
      beesign_frame_t package = {.hdr = BEESIGN_HDR,
                                 .type = id,
                                 .len = len,
                                 .payload = pData};

      if ((len >= BEESIGN_PL_MAX_LEN) ||
          (pData == 0)) {
         return BEESIGN_ERROR;
      }

      package.crc = beesignCRC(&package);

      softserial_write_byte(package.hdr);
      softserial_write_byte(package.type);
      softserial_write_byte(package.len);
      for (uint8_t i = 0; i < len; i++) {
         softserial_write_byte(*(package.payload + i));
      }
      softserial_write_byte(package.crc);
      return BEESIGN_OK;
   }
}

/********************************** BEESIGN VTX ********************************************/
void bsSetVTxUnlock(uint8_t cmd) {
   uint16_t unlock = BEESIGN_VTX_UNLOCK;
   uint8_t unlockData[2] = {unlock >> 8, unlock};
   beesignSend(BEESIGN_V_UNLOCK, 2, unlockData, cmd);
   beesignSend(BEESIGN_M_SAVE_SETTING, 1, 0, cmd);
}

void bsSetVTxLock(uint8_t cmd) {
   uint16_t lock = BEESIGN_VTX_LOCK;
   uint8_t lockData[2] = {lock >> 8, lock};
   beesignSend(BEESIGN_V_LOCK, 2, lockData, cmd);
   beesignSend(BEESIGN_M_SAVE_SETTING, 1, 0, cmd);
}

uint8_t bsValidateBandAndChannel(uint8_t band, uint8_t channel) {
   return (band >= BEESIGN_MIN_BAND && band <= BEESIGN_MAX_BAND &&
           channel >= BEESIGN_MIN_CHANNEL && channel <= BEESIGN_MAX_CHANNEL);
}

void bsSetBandAndChannel(uint8_t band, uint8_t channel, uint8_t cmd) {
   if (!bsValidateBandAndChannel(band, channel))
      return;
   uint8_t deviceChannel = BS_BANDCHAN_TO_DEVICE_CHVAL(band, channel);
   bsDevice.vtx.channel = deviceChannel;
   // bsDevice.freq = vtx58frequencyTable[band][channel];
   beesignSend(BEESIGN_V_SET_CHAN, 1, &deviceChannel, cmd);
}

void bsSetPower(uint8_t index, uint8_t cmd) {

   if (index > BEESIGN_POWER_COUNT) {
      return;
   }
   bsDevice.vtx.power = index;
   beesignSend(BEESIGN_V_SET_PWR, 1, &index, cmd);
}

void bsSetVtxMode(uint8_t mode, uint8_t cmd) {
   if (mode > 2)
      return;
   bsDevice.vtx.mode = mode;
   beesignSend(BEESIGN_V_SET_MODE, 1, &mode, cmd);
}

uint8_t bsValidateFreq(uint16_t freq) {
   return (freq >= BEESIGN_MIN_FREQUENCY_MHZ && freq <= BEESIGN_MAX_FREQUENCY_MHZ);
}

void bsSetFreq(uint16_t freq, uint8_t cmd) {
   uint8_t buf[2];
   if (!bsValidateFreq(freq))
      return;
   buf[0] = (freq >> 8) & 0xff;
   buf[1] = freq & 0xff;
   bsDevice.vtx.channel = BEESIGN_ERROR_CHANNEL;
   beesignSend(BEESIGN_V_SET_FREQ, 2, buf, cmd);
}

/******************************** BEESIGN VTX END ******************************************/

/********************************** BEESIGN OSD ********************************************/
// #if defined(USE_OSD_BEESIGN)

//static uint8_t bsScreenBuffer[BEESIGN_CHARS_PER_SCREEN];
//static uint8_t bsShadowBuffer[BEESIGN_CHARS_PER_SCREEN];

void bsClearDispaly(uint8_t cmd) {
   uint8_t clrData = 0;
   beesignSend(BEESIGN_O_CLR_DISPLAY, 1, &clrData, cmd);
}

void bsSetOsdMode(uint8_t mode, uint8_t cmd) {
   if (mode > BEESIGN_OSD_MODE_CUSTOM) {
      mode = BEESIGN_OSD_MODE_OFF;
   }
   beesignSend(BEESIGN_O_SET_MODE, 1, &mode, cmd);
}

void bsSetMiniLayout(uint8_t VolPostion, uint8_t RssiPostion, uint8_t NamePostion, uint8_t CurPostion, uint8_t FModePosition, uint8_t FTimePosition, uint8_t VtxPosition, uint8_t cmd) {
   // uint8_t layoutData[4] = {VolPostion, RssiPostion, NamePostion, CurPostion};
   uint8_t layoutData[7] = {VolPostion, RssiPostion, NamePostion, CurPostion, FModePosition, FTimePosition, VtxPosition};
   // uint8_t layoutData[7] = {VolPostion, RssiPostion, NamePostion, CurPostion, 0, 0, 0};
   bsDevice.osd.voltagePosition = VolPostion;
   bsDevice.osd.rssiPosition = RssiPostion;
   bsDevice.osd.namePosition = NamePostion;
   bsDevice.osd.currentPosition = CurPostion;
   bsDevice.osd.fmodePosition = FModePosition;
   bsDevice.osd.ftimePosition = FTimePosition;
   bsDevice.osd.vtxPosition = VtxPosition;
   beesignSend(BEESIGN_O_SET_LAYOUT, 7, layoutData, cmd);
}

void bsSetOsdHosOffset(uint8_t offset, uint8_t cmd) {
   offset += 4;
   if (offset > BEESIGN_OSD_HOS_MAX) {
      offset = BEESIGN_OSD_HOS_MAX;
   }
   beesignSend(BEESIGN_O_SET_HOS, 1, &offset, cmd);
}

void bsSetOsdVosOffset(uint8_t offset, uint8_t cmd) {
   offset += 25;
   if (offset > BEESIGN_OSD_VOS_MAX) {
      offset = BEESIGN_OSD_VOS_MAX;
   }
   beesignSend(BEESIGN_O_SET_VOS, 1, &offset, cmd);
}

void bsSetDisplayContentOneFrame(uint8_t pos, const char *data, uint8_t len, uint8_t cmd) {
   uint8_t s[BEESIGN_PL_MAX_LEN];
   if (pos >= BEESIGN_CHARS_PER_SCREEN) {
      return;
   }
   if (len > BEESIGN_PL_MAX_LEN - 1) {
      len = BEESIGN_PL_MAX_LEN - 1;
   }
   s[0] = pos;
   for (int i = 0; i < len; i++) {
      s[i + 1] = data[i];
   }
   beesignSend(BEESIGN_O_SET_DISPLAY, len + 1, s, cmd);
}

void bsSetDisplayOneChar(uint8_t x, uint8_t y, const char data, uint8_t cmd) {
   if (y >= BEESIGN_LINES_PER_SCREEN) {
      return;
   }
   if (x >= BEESIGN_CHARS_PER_LINE) {
      return;
   }
   bsSetDisplayContentOneFrame(y * BEESIGN_CHARS_PER_LINE + x, &data, 1, cmd);
}

void bsSetDisplayOneRow(uint8_t x, uint8_t y, const char *data, uint8_t cmd) {
   int i = 0;
   if (y >= BEESIGN_LINES_PER_SCREEN) {
      return;
   }
   for (i = 0; *(data + i); i++) {
      if (x + i > BEESIGN_CHARS_PER_LINE) {
         break;
      }
   }
   bsSetDisplayContentOneFrame(y * BEESIGN_CHARS_PER_LINE + x, data, i, cmd);
}

// void bsClearScreenBuff(uint8_t cmd) {
//     memset(bsScreenBuffer, 0x20, BEESIGN_CHARS_PER_SCREEN);
//     memset(bsShadowBuffer, 0x20, BEESIGN_CHARS_PER_SCREEN);
//     bsClearDispaly(cmd);

// }

// void bsWriteChar(uint8_t x, uint8_t y, uint8_t c, uint8_t cmd)
// {
//     if (y >= BEESIGN_LINES_PER_SCREEN) {
//         return;
//     }
//     if (x >= BEESIGN_CHARS_PER_LINE) {
//         return;
//     }
//     bsSetDisplayOneChar(x, y, c, cmd);
//     // bsScreenBuffer[y*BEESIGN_CHARS_PER_LINE+x] = c;
// }

// void bsWriteBuffRow(uint8_t x, uint8_t y, const char *buff, uint8_t cmd)
// {
//     if (y >= BEESIGN_LINES_PER_SCREEN) {
//         return;
//     }
//     if (x >= BEESIGN_CHARS_PER_LINE) {
//         return;
//     }
//     bsSetDisplayOneChar(x, y, buff, cmd);
//     // for (int i = 0; *(buff+i); i++) {
//     //     if (x+i < BEESIGN_CHARS_PER_LINE) {// Do not write over screen
//     //         bsScreenBuffer[y*BEESIGN_CHARS_PER_LINE+x+i] = *(buff+i);
//     //     }
//     // }
// }

// void bsDisplay(void) {
//     uint8_t buffStartPos = 0xFF;
//     uint8_t buffEndPos = 0xFF;
//     uint8_t seriaBuff[BEESIGN_CHARS_PER_LINE + 1];
//     for (int i = 0; i < BEESIGN_CHARS_PER_SCREEN; i++) {
//         if (bsScreenBuffer[i] != bsShadowBuffer[i]) {
//             if (buffStartPos == 0xFF) {
//                 buffStartPos = i;
//             }
//             seriaBuff[i - buffStartPos + 1] = bsScreenBuffer[i];
//             if (i - buffStartPos + 1 >= BEESIGN_CHARS_PER_LINE) {
//                 seriaBuff[0] = buffStartPos;
//                 beesignSend(BEESIGN_O_SET_DISPLAY, i - buffStartPos + 2, seriaBuff, BEESIGN_CMD_ADD_BUFF);
//                 buffStartPos = 0xFF;
//             }
//             buffEndPos = i;
//             bsShadowBuffer[i] = bsScreenBuffer[i];
//         } else {
//             if ((buffStartPos != 0xFF)) {
//                 if (i - buffStartPos + 1 < 10) {
//                     seriaBuff[i - buffStartPos + 1] = bsScreenBuffer[i];
//                 } else {
//                     seriaBuff[0] = buffStartPos;
//                     beesignSend(BEESIGN_O_SET_DISPLAY, buffEndPos - buffStartPos + 2, seriaBuff, BEESIGN_CMD_ADD_BUFF);
//                     buffStartPos = 0xFF;
//                 }

//             }
//         }
//     }
//     if (buffStartPos != 0xFF) {
//         seriaBuff[0] = buffStartPos;
//         beesignSend(BEESIGN_O_SET_DISPLAY, buffEndPos - buffStartPos + 2, seriaBuff, BEESIGN_CMD_ADD_BUFF);
//         buffStartPos = 0xFF;
//     }
// }

//void bsDisplayAllScreen(void) {
//    for (int i = 0; i < BEESIGN_LINES_PER_SCREEN;i++) {
//        bsSetDisplayContentOneFrame(i * BEESIGN_CHARS_PER_LINE, &bsScreenBuffer[BEESIGN_CHARS_PER_LINE * i], BEESIGN_CHARS_PER_LINE, BEESIGN_CMD_ADD_BUFF);
//    }
//    memcpy(bsShadowBuffer, bsScreenBuffer, BEESIGN_CHARS_PER_SCREEN);
//}

//uint8_t bsBuffersSynced(void)
//{
//    for (int i = 0; i < BEESIGN_CHARS_PER_SCREEN; i++) {
//        if (bsScreenBuffer[i] != bsShadowBuffer[i]) {
//            return 0;
//        }
//    }
//    return 1;
//}

// #endif //defined(USE_OSD_BEESIGN)
/******************************** BEESIGN OSD END ******************************************/

/******************************** BEESIGN SOURCE ******************************************/
void bsSaveSetting(uint8_t cmd) {
   uint8_t data = 0;
   beesignSend(BEESIGN_M_SAVE_SETTING, 1, &data, cmd);
}

void bsSetVolSource(uint8_t source, uint8_t cmd) {
   if (source > BEESIGN_VOLTAGE_EXT) {
      source = 0;
   }
   beesignSend(BEESIGN_S_SET_VOL, 1, &source, cmd);
}

void bsSetRssiSource(uint8_t source, uint8_t cmd) {
   if (source > BEESIGN_RSSI_EXT) {
      source = 0;
   }
   beesignSend(BEESIGN_S_SET_RSSI, 1, &source, cmd);
}

void bsSetName(char *name, uint8_t len, uint8_t cmd) {
   strncpy(bsDevice.others.name, name, 8);
   beesignSend(BEESIGN_M_SET_NAME, len, (uint8_t *)name, cmd);
}

void bsSetVol(uint16_t value, uint8_t cmd) {
   uint8_t volData[2];
   volData[0] = value >> 8;
   volData[1] = value;
   beesignSend(BEESIGN_M_SET_VOL, 2, volData, cmd);
}

void bsSetCur(uint16_t value, uint8_t cmd) {
   uint8_t curData[2];
   curData[0] = value >> 8;
   curData[1] = value;
   beesignSend(BEESIGN_M_SET_CUR, 2, curData, cmd);
}

void bsSetRssi(uint8_t value, uint8_t cmd) {
   beesignSend(BEESIGN_M_SET_RSSI, 1, &value, cmd);
}

/****************************** BEESIGN SOURCE END ****************************************/

void beesignInit(void) {
   extern void flash_save(void);
   extern void flash_load(void);
   softserial_init(BEESIGN_TX_PORT, BEESIGN_TX_PIN, BEESIGN_RX_PORT, BEESIGN_RX_PIN, BEESIGN_BAUDRATE);
   if (bsDevice.blankFlg != 1) {
      bsDevice.vtx.channel = 0;
      bsDevice.vtx.power = 1;
#ifdef RX_DSMX_2048
      bsDevice.osd.voltagePosition = 3;
      bsDevice.osd.rssiPosition = 0;
#else
      bsDevice.osd.voltagePosition = 28;
      bsDevice.osd.rssiPosition = 30;
#endif
      bsDevice.osd.namePosition = 29;
      strncpy(bsDevice.others.name, "HMG BIRD", 8);
      bsDevice.blankFlg = 1;
      flash_save();
   }
   bsSetOsdMode(BEESIGN_OSD_MODE_MINI, BEESIGN_CMD_SEND);
   delay(50000);
   bsSetMiniLayout(bsDevice.osd.voltagePosition, bsDevice.osd.rssiPosition, bsDevice.osd.namePosition, 0, 0, 0, 0, BEESIGN_CMD_SEND);
   delay(50000);
   bsSetName(bsDevice.others.name, 8, BEESIGN_CMD_SEND);
   delay(50000);
   bsSetVolSource(BEESIGN_VOLTAGE_EXT, BEESIGN_CMD_SEND);
   delay(50000);
   bsSetRssiSource(BEESIGN_RSSI_EXT, BEESIGN_CMD_SEND);
   delay(50000);
   bsSetPower(bsDevice.vtx.power, BEESIGN_CMD_SEND);
   delay(50000);
   bsSetBandAndChannel(bsDevice.vtx.channel / 8 + 1, bsDevice.vtx.channel % 8 + 1, BEESIGN_CMD_SEND);
   delay(50000);
   bsSaveSetting(BEESIGN_CMD_SEND);
   delay(50000);
}

void beesignTask(void) {
   static uint16_t taskCountTime = 0;
   static uint8_t beesignCmdCountTime;
   taskCountTime++;
   beesignCmdCountTime++;
   if (beesignCmdCountTime >= 50) {
      beesignCmdCountTime = 0;
      beesignSendCmd();
   }
   if (taskCountTime >= VOL_AND_RSSI_SEND_COUNT && getMenuState() == 0) { //cycles executed once
      taskCountTime = 0;
      bsSetVol((uint16_t)(BEESIGN_GET_FC_VOLTAGE()), BEESIGN_CMD_ADD_BUFF);
      // clamp to 99: is RSSI comes back as 100, the OSD presents "00" which can be misinterpreted as "no signal"
      // max value of "rx_rssi" is 0x07FF, 2047. The BEESIGN macro divides by "20.47", so a value of "100" is possible.
      uint8_t rssi = BEESIGN_GET_FC_RSSI();
      if (rssi > 99) {
         rssi = 99;
      }
      bsSetRssi(rssi, BEESIGN_CMD_ADD_BUFF);
   }
}
#endif
