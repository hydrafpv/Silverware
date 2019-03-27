#include "stm32f0xx.h"
#include "config.h"
#if defined (USE_BEESIGN)
#include "menu_fc.h"
#include "menu.h"
#include "string.h"
#include "stick_command.h"

#define AUX_NAME_COUNT      14
#define FC_NAME_POINTER_COUNT   8

static uint8_t fc_armAt = 0;
static uint8_t fc_lastArmAt = 0xFF;
static uint8_t fc_altiAt = 0;
static uint8_t fc_lastAltiAt = 0xFF;
static uint8_t fc_levelAt = 0;
static uint8_t fc_lastLevelAt = 0xFF;
static uint8_t fc_raceAt = 0;
static uint8_t fc_lastRaceAt = 0xFF;
static uint8_t fc_horizonAt = 0;
static uint8_t fc_lastHorizonAt = 0xFF;
static uint8_t fc_namePointer = 0;
static uint8_t fc_lastNamePointer = 0xFF;
static char fcName[9] = {0};

const char * const AuxName[] = {
    "---", "OFF", "ON",
    "AUX1_L", "AUX1_M", "AUX1_H",
    "AUX2_L", "AUX2_M", "AUX2_H",
    "AUX3_L", "AUX3_M", "AUX3_H",
    "AUX4_L", "AUX4_M", "AUX4_H",
};

const char * const fc_namePointerShows[] = {
    "--------",
    "        ",
    "^       ",
    " ^      ",
    "  ^     ",
    "   ^    ",
    "    ^   ",
    "     ^  ",
    "      ^ ",
    "       ^",
};

static const menuTab_t fcEntryArmAt =         {&fc_armAt,     &fc_lastArmAt, AUX_NAME_COUNT - 1, &AuxName[1]};
static const menuTab_t fcEntryAltiAt =        {&fc_altiAt,    &fc_lastAltiAt, AUX_NAME_COUNT - 1, &AuxName[1]};
static const menuTab_t fcEntryLevelAt =       {&fc_levelAt,   &fc_lastLevelAt, AUX_NAME_COUNT - 1, &AuxName[1]};
static const menuTab_t fcEntryRaceAt =        {&fc_raceAt,    &fc_lastRaceAt, AUX_NAME_COUNT - 1, &AuxName[1]};
static const menuTab_t fcEntryHorizonAt =     {&fc_horizonAt, &fc_lastHorizonAt, AUX_NAME_COUNT - 1, &AuxName[1]};
static const menuTab_t fcEntryNamePointer =   {&fc_namePointer,&fc_lastNamePointer, 0, &fc_namePointerShows[1]};
static const menuString_t fcEntryName =       {fcName};

static uint8_t fcConfigName(void *menuElement) {
    static uint8_t cursorRowLock = 0;
    switch (*(menuElement_e*)menuElement) {
    case menu_Up:
        if (cursorRowLock && fcName[fc_namePointer - 1] > ' ') {
            fcName[fc_namePointer - 1]--;
        }
        break;
    case menu_Down:
        if (cursorRowLock && fcName[fc_namePointer - 1] < 'Z') {
                fcName[fc_namePointer - 1]++;
            }
        break;
    case menu_Left:
        if (cursorRowLock && fc_namePointer > 0) {
            fc_namePointer--;
        }
        if (fc_namePointer == 0) {
            cursorRowLock = 0;
        }
        break;
    case menu_Right:
        if (cursorRowLock == 0) cursorRowLock = 1;
        if (cursorRowLock && fc_namePointer < FC_NAME_POINTER_COUNT + 1) {
            fc_namePointer++;
        }
        if (fc_namePointer > FC_NAME_POINTER_COUNT) {
            fc_namePointer = 0;
            cursorRowLock = 0;
        }
        break;
    default:
        break;
    }
    return cursorRowLock;
}

static void fcMenuExitFun(void) {
    fc_lastArmAt = 0xFF;
    fc_lastAltiAt = 0xFF;
    fc_lastLevelAt = 0xFF;
    fc_lastRaceAt = 0xFF;
    fc_lastHorizonAt = 0xFF;
    fc_lastNamePointer = 0xFF;
}


static uint8_t analysisStickCommands(uint8_t rcCmd) {
    if(rcCmd & (0x01 << RC_SW_OFFSET)){
        if ((rcCmd & (0x03 << RC_CFG_OFFSET)) == (0x03 << RC_CFG_OFFSET)) {
            return 1;
        } else {
            return (rcCmd & 0x1F - RX_NON_AUX_CH_COUNT) * 3 + 2 + ((rcCmd & (0x03 << RC_CFG_OFFSET)) >> RC_CFG_OFFSET);
        }
    } else {
        return 0;
    }
}

static uint8_t analysisFcMenuAt(uint8_t at) {
    if (at == 0) return RC_SW_DISABLED;
    if (at == 1) return RC_SW_ENABLED | RC_ACTIVE_NONE;
    return RC_SW_ENABLED | (((at + 1) % 3) << RC_CFG_OFFSET) | ((at + 1) / 3 + RX_NON_AUX_CH_COUNT - 1);
}

static void fcMenuEnterFun(void) {
    strncpy(fcName, bsDevice.others.name, FC_NAME_POINTER_COUNT);
    fc_armAt = analysisStickCommands(rcCmdArm);
    fc_altiAt = analysisStickCommands(rcCmdAlti);
    fc_levelAt = analysisStickCommands(rcCmdLevel);
    fc_raceAt = analysisStickCommands(rcCmdRace);
    fc_horizonAt = analysisStickCommands(rcCmdHorizon);
}

void fcMenuInit(void) {
    fc_armAt = analysisStickCommands(rcCmdArm);
    fc_altiAt = analysisStickCommands(rcCmdAlti);
    fc_levelAt = analysisStickCommands(rcCmdLevel);
    fc_raceAt = analysisStickCommands(rcCmdRace);
    fc_horizonAt = analysisStickCommands(rcCmdHorizon);
    strncpy(fcName, bsDevice.others.name, FC_NAME_POINTER_COUNT);
}

void fcMenuSaveFun(void) {
    rcCmdArm = analysisFcMenuAt(fc_armAt);
    rcCmdAlti = analysisFcMenuAt(fc_altiAt);
    rcCmdLevel = analysisFcMenuAt(fc_levelAt);
    rcCmdRace = analysisFcMenuAt(fc_raceAt);
    rcCmdHorizon = analysisFcMenuAt(fc_horizonAt);
    bsSetName(fcName, 8, BEESIGN_CMD_ADD_BUFF);
}

static menuEntry_t fcMenuEntries[] = {
    {"- FC -", menu_Label,     NULL,           NULL,                     0},
    {"ARM AT",     menu_Tab,       NULL,        &fcEntryArmAt,    0},
    {"ALTI AT",    menu_Tab,       NULL,        &fcEntryAltiAt,   0},
    {"LEVEL AT",    menu_Tab,       NULL,       &fcEntryLevelAt,   0},
    {"RACE AT",    menu_Tab,       NULL,        &fcEntryRaceAt,   0},
    {"HORIZON AT",    menu_Tab,       NULL,     &fcEntryHorizonAt,   0},
    {"NAME",    menu_String,       fcConfigName,        &fcEntryName,   0},
    {"",    menu_Tab,       NULL,           &fcEntryNamePointer,  DYNAMIC},
    {"EXIT",    menu_Back,      NULL,           NULL,                     0},
    { NULL,     menu_End,       NULL,           NULL,                     0}
};

const menu_t fcMenu = {
    .uperMenu = &mainMenu,
    .onEnter = fcMenuEnterFun,
    .onExit = fcMenuExitFun,
    .entries = fcMenuEntries
};
#endif

