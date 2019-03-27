#ifndef _BEESIGN_OSD_H_
#define _BEESIGN_OSD_H_

#include "stm32f0xx.h"
#include "beesign.h"

#define OSD_POSTION_COUNT       31

extern const char * const osdPostionName[];

void bsOsdSetMiniPostion(uint8_t volPostion, uint8_t rssiPostion, uint8_t namePostion);

#endif // #ifndef _BEESIGN_OSD_H_
