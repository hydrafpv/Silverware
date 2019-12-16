#include "config.h"
#include "stm32f0xx.h"
#if defined(USE_BEESIGN)
#include "beesign.h"
#include "beesign_osd.h"

const char *const osdPostionName[] = {
    "-",
    "OFF",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
    "17",
    "18",
    "19",
    "20",
    "21",
    "22",
    "23",
    "24",
    "25",
    "26",
    "27",
    "28",
    "29",
    "30",
};

void bsOsdSetMiniPostion(uint8_t volPostion, uint8_t rssiPostion, uint8_t namePostion, uint8_t fmodePosition, uint8_t ftimePosition, uint8_t vtxPosition) {
   bsSetMiniLayout(volPostion, rssiPostion, namePostion, 0, fmodePosition, ftimePosition, vtxPosition, BEESIGN_CMD_ADD_BUFF);
}
#endif
