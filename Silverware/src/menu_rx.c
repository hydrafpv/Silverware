#include "config.h"
#include "stm32f0xx.h"
#if defined(USE_BEESIGN)
#include "config.h"
#include "defines.h"
#include "menu.h"
#include "menu_rx.h"
#include "string.h"

extern int channels[9];
static int lastChannels[9] = {0xFF};
#if defined(RX_DSMX_2048) || defined(RX_DSMX_1024) // DSM TAER
#define ROLL_VAILE channels[1]
#define ROLL_LAST_VAILE lastChannels[1]
#define PITCH_VAILE channels[2]
#define PITCH_LAST_VAILE lastChannels[2]
#define THR_VAILE channels[0]
#define THR_LAST_VAILE lastChannels[0]
#define YAW_VAILE channels[3]
#define YAW_LAST_VAILE lastChannels[3]
#else // FRSKY AETR
#define ROLL_VAILE channels[0]
#define ROLL_LAST_VAILE lastChannels[0]
#define PITCH_VAILE channels[1]
#define PITCH_LAST_VAILE lastChannels[1]
#define THR_VAILE channels[2]
#define THR_LAST_VAILE lastChannels[2]
#define YAW_VAILE channels[3]
#define YAW_LAST_VAILE lastChannels[3]
#endif

#define AUX1_VAILE channels[4]
#define AUX1_LAST_VAILE lastChannels[4]
#define AUX2_VAILE channels[5]
#define AUX2_LAST_VAILE lastChannels[5]
#define AUX3_VAILE channels[6]
#define AUX3_LAST_VAILE lastChannels[6]
#define AUX4_VAILE channels[7]
#define AUX4_LAST_VAILE lastChannels[7]

static menuEntry_t rxMenuEntries[] = {
    {"- RX -", menu_Label, NULL, NULL, 0},
    {"ROLL", menu_Uint16, NULL, &(menuUint16_t){(uint16_t *)(&ROLL_VAILE), (uint16_t *)(&ROLL_LAST_VAILE), 0, 2000, 0}, DYNAMIC},
    {"PITCH", menu_Uint16, NULL, &(menuUint16_t){(uint16_t *)(&PITCH_VAILE), (uint16_t *)(&PITCH_LAST_VAILE), 0, 2000, 0}, DYNAMIC},
    {"THR", menu_Uint16, NULL, &(menuUint16_t){(uint16_t *)(&THR_VAILE), (uint16_t *)(&THR_LAST_VAILE), 0, 2000, 0}, DYNAMIC},
    {"YAW", menu_Uint16, NULL, &(menuUint16_t){(uint16_t *)(&YAW_VAILE), (uint16_t *)(&YAW_LAST_VAILE), 0, 2000, 0}, DYNAMIC},
    {"AUX1", menu_Uint16, NULL, &(menuUint16_t){(uint16_t *)(&AUX1_VAILE), (uint16_t *)(&AUX1_LAST_VAILE), 0, 2000, 0}, DYNAMIC},
    // {"AUX2", menu_Uint16, NULL, &(menuUint16_t){(uint16_t *)(&AUX2_VAILE), (uint16_t *)(&AUX2_LAST_VAILE), 0, 2000, 0}, DYNAMIC},
    // {"AUX3", menu_Uint16, NULL, &(menuUint16_t){(uint16_t *)(&AUX3_VAILE), (uint16_t *)(&AUX3_LAST_VAILE), 0, 2000, 0}, DYNAMIC},
    // {"AUX4", menu_Uint16, NULL, &(menuUint16_t){(uint16_t *)(&AUX4_VAILE), (uint16_t *)(&AUX4_LAST_VAILE), 0, 2000, 0}, DYNAMIC},
    {"EXIT", menu_Back, NULL, NULL, 0},
    {NULL, menu_End, NULL, NULL, 0}};

static void rxMenuExitFun(void) {
   ROLL_LAST_VAILE = 0xFF;
   PITCH_LAST_VAILE = 0xFF;
   THR_LAST_VAILE = 0xFF;
   YAW_LAST_VAILE = 0xFF;
   AUX1_LAST_VAILE = 0xFF;
   AUX2_LAST_VAILE = 0xFF;
   AUX3_LAST_VAILE = 0xFF;
   AUX4_LAST_VAILE = 0xFF;
}

const menu_t rxMenu = {
    .uperMenu = &mainMenu,
    .onEnter = NULL,
    .onExit = rxMenuExitFun,
    .entries = rxMenuEntries};
#endif
