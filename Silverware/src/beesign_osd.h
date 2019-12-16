#ifndef _BEESIGN_OSD_H_
#define _BEESIGN_OSD_H_

#include "beesign.h"
#include "stm32f0xx.h"

#define OSD_POSTION_COUNT 31

extern const char *const osdPositionName[];

void bsOsdSetMiniPostion(uint8_t volPostion, uint8_t rssiPostion, uint8_t namePostion, uint8_t fmodePosition, uint8_t ftimePosition, uint8_t vtxPosition);

#endif // #ifndef _BEESIGN_OSD_H_
