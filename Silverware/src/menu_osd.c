#include "stm32f0xx.h"
#include "config.h"
#if defined (USE_BEESIGN)
#include "menu_osd.h"
#include "menu.h"
#include "beesign_osd.h"
#include "string.h"

static uint8_t osd_menuVolPostion = 0;
static uint8_t osd_lastMenuVolPostion = 0xFF;
static uint8_t osd_menuRssiPostion = 0;
static uint8_t osd_lastMenuRssiPostion = 0xFF;
static uint8_t osd_menuNamePostion = 0;
static uint8_t osd_lastMenuNamePostion = 0xFF;

static const menuTab_t bsOsdEntryVolPostion =  {&osd_menuVolPostion,  &osd_lastMenuVolPostion, OSD_POSTION_COUNT - 1, &osdPostionName[1]};
static const menuTab_t bsOsdEntryRssiPostion = {&osd_menuRssiPostion, &osd_lastMenuRssiPostion, OSD_POSTION_COUNT - 1, &osdPostionName[1]};
static const menuTab_t bsOsdEntryNamePostion = {&osd_menuNamePostion, &osd_lastMenuNamePostion, OSD_POSTION_COUNT - 1, &osdPostionName[1]};

static void osdMenuExitFun(void) {
    osd_lastMenuVolPostion = 0xFF;
    osd_lastMenuRssiPostion = 0xFF;
    osd_lastMenuNamePostion = 0xFF;
}

static void osdMenuEnterFun(void) {
    osd_menuVolPostion =  bsDevice.osd.voltagePosition;
    osd_menuRssiPostion = bsDevice.osd.rssiPosition;
    osd_menuNamePostion = bsDevice.osd.namePosition;
}

void osdMenuInit(void) {
    osd_menuVolPostion =  bsDevice.osd.voltagePosition;
    osd_menuRssiPostion = bsDevice.osd.rssiPosition;
    osd_menuNamePostion = bsDevice.osd.namePosition;
}

void osdMenuSaveFun(void) {
    bsOsdSetMiniPostion(osd_menuVolPostion, osd_menuRssiPostion, osd_menuNamePostion);
}

static menuEntry_t osdMenuEntries[] = {
    {"- OSD -", menu_Label,     NULL,           NULL,                     0},
    {"VOL",     menu_Tab,       NULL,           &bsOsdEntryVolPostion,    0},
    {"RSSI",    menu_Tab,       NULL,           &bsOsdEntryRssiPostion,   0},
    {"NAME",    menu_Tab,       NULL,           &bsOsdEntryNamePostion,   0},
    {"EXIT",    menu_Back,      NULL,           NULL,                     0},
    { NULL,     menu_End,       NULL,           NULL,                     0}
};

const menu_t osdMenu = {
    .uperMenu = &mainMenu,
    .onEnter = osdMenuEnterFun,
    .onExit = osdMenuExitFun,
    .entries = osdMenuEntries
};
#endif
