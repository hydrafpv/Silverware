#include "stm32f0xx.h"
#include "config.h"
#if defined (USE_BEESIGN)
#include "beesign_osd.h"
#include "menu.h"
#include "menu_osd.h"
#include "string.h"

static uint8_t osd_menuVolPos = 0;
static uint8_t osd_lastMenuVolPos = 0xFF;
static uint8_t osd_menuRssiPos = 0;
static uint8_t osd_lastMenuRssiPos = 0xFF;
static uint8_t osd_menuNamePos = 0;
static uint8_t osd_lastMenuNamePos = 0xFF;
static uint8_t osd_menuFModePos = 0;
static uint8_t osd_lastMenuFModePos = 0xFF;
static uint8_t osd_menuFTimePos = 0;
static uint8_t osd_lastMenuFTimePos = 0xFF;
static uint8_t osd_menuVtxPos = 0;
static uint8_t osd_lastMenuVtxPos = 0xFF;

static const menuTab_t bsOsdEntryVolPos = {&osd_menuVolPos, &osd_lastMenuVolPos, OSD_POSTION_COUNT - 1, &osdPostionName[1]};
static const menuTab_t bsOsdEntryRssiPos = {&osd_menuRssiPos, &osd_lastMenuRssiPos, OSD_POSTION_COUNT - 1, &osdPostionName[1]};
static const menuTab_t bsOsdEntryNamePos = {&osd_menuNamePos, &osd_lastMenuNamePos, OSD_POSTION_COUNT - 1, &osdPostionName[1]};
static const menuTab_t bsOsdEntryFModePos = {&osd_menuFModePos, &osd_lastMenuFModePos, OSD_POSTION_COUNT - 1, &osdPostionName[1]};
static const menuTab_t bsOsdEntryFTimePos = {&osd_menuFTimePos, &osd_lastMenuFTimePos, OSD_POSTION_COUNT - 1, &osdPostionName[1]};
static const menuTab_t bsOsdEntryVtxPos = {&osd_menuVtxPos, &osd_lastMenuVtxPos, OSD_POSTION_COUNT - 1, &osdPostionName[1]};

static void osdMenuExitFun(void) {
   osd_lastMenuVolPos = 0xFF;
   osd_lastMenuRssiPos = 0xFF;
   osd_lastMenuNamePos = 0xFF;
   osd_lastMenuFModePos = 0xFF;
   osd_lastMenuFTimePos = 0xFF;
   osd_lastMenuVtxPos = 0xFF;
}

static void osdMenuEnterFun(void) {
   osd_menuVolPos = bsDevice.osd.voltagePosition;
   osd_menuRssiPos = bsDevice.osd.rssiPosition;
   osd_menuNamePos = bsDevice.osd.namePosition;
   osd_menuFModePos = bsDevice.osd.fmodePosition;
   osd_menuFTimePos = bsDevice.osd.ftimePosition;
   osd_menuVtxPos = bsDevice.osd.vtxPosition;
}

void osdMenuInit(void) {
   osd_menuVolPos = bsDevice.osd.voltagePosition;
   osd_menuRssiPos = bsDevice.osd.rssiPosition;
   osd_menuNamePos = bsDevice.osd.namePosition;
   osd_menuFModePos = bsDevice.osd.fmodePosition;
   osd_menuFTimePos = bsDevice.osd.ftimePosition;
   osd_menuVtxPos = bsDevice.osd.vtxPosition;
}

void osdMenuSaveFun(void) {
   bsOsdSetMiniPostion(osd_menuVolPos, osd_menuRssiPos, osd_menuNamePos, osd_menuFModePos, osd_menuFTimePos, osd_menuVtxPos);
}

static menuEntry_t osdMenuEntries[] = {
    {"- OSD -", menu_Label, NULL, NULL, 0},
    {"VOL", menu_Tab, NULL, &bsOsdEntryVolPos, 0},
    {"RSSI", menu_Tab, NULL, &bsOsdEntryRssiPos, 0},
    {"NAME", menu_Tab, NULL, &bsOsdEntryNamePos, 0},
    // {"MODE", menu_Tab, NULL, &bsOsdEntryFModePos, 0},
    // {"TIME", menu_Tab, NULL, &bsOsdEntryFTimePos, 0},
    // {"VTX", menu_Tab, NULL, &bsOsdEntryVtxPos, 0},
    {"EXIT", menu_Back, NULL, NULL, 0},
    {NULL, menu_End, NULL, NULL, 0}};

const menu_t osdMenu = {.uperMenu = &mainMenu,
                        .onEnter = osdMenuEnterFun,
                        .onExit = osdMenuExitFun,
                        .entries = osdMenuEntries};
#endif
