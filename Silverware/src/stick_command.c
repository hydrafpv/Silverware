#include "config.h"
#include "defines.h"
#include "drv_time.h"
#include "project.h"
#include "util.h"
#include <stdio.h>

#if defined(USE_BEESIGN)
#include "menu.h"
#include "stick_command.h"

#define ROL_LO (1 << (2 * ROLL_SC))
#define ROL_CE (3 << (2 * ROLL_SC))
#define ROL_HI (2 << (2 * ROLL_SC))
#define PIT_LO (1 << (2 * PITCH_SC))
#define PIT_CE (3 << (2 * PITCH_SC))
#define PIT_HI (2 << (2 * PITCH_SC))
#define YAW_LO (1 << (2 * YAW_SC))
#define YAW_CE (3 << (2 * YAW_SC))
#define YAW_HI (2 << (2 * YAW_SC))
#define THR_LO (1 << (2 * THROTTLE_SC))
#define THR_CE (3 << (2 * THROTTLE_SC))
#define THR_HI (2 << (2 * THROTTLE_SC))
#define AUX1_LO (1 << (2 * (AUX1_SC - RX_NON_AUX_CH_COUNT)))
#define AUX1_CE (3 << (2 * (AUX1_SC - RX_NON_AUX_CH_COUNT)))
#define AUX1_HI (2 << (2 * (AUX1_SC - RX_NON_AUX_CH_COUNT)))
#define AUX2_LO (1 << (2 * (AUX2_SC - RX_NON_AUX_CH_COUNT)))
#define AUX2_CE (3 << (2 * (AUX2_SC - RX_NON_AUX_CH_COUNT)))
#define AUX2_HI (2 << (2 * (AUX2_SC - RX_NON_AUX_CH_COUNT)))
#define AUX3_LO (1 << (2 * (AUX3_SC - RX_NON_AUX_CH_COUNT)))
#define AUX3_CE (3 << (2 * (AUX3_SC - RX_NON_AUX_CH_COUNT)))
#define AUX3_HI (2 << (2 * (AUX3_SC - RX_NON_AUX_CH_COUNT)))
#define AUX4_LO (1 << (2 * (AUX4_SC - RX_NON_AUX_CH_COUNT)))
#define AUX4_CE (3 << (2 * (AUX4_SC - RX_NON_AUX_CH_COUNT)))
#define AUX4_HI (2 << (2 * (AUX4_SC - RX_NON_AUX_CH_COUNT)))

#define SET_BIT(x, b) (x |= (1 << b))
#define CLR_BIT(x, b) (x &= ~(1 << b))
#define GET_BIT(x, b) ((x >> b) & 1)

// rcCmdXXX settings:
//   e.g.: rcCmdArm = RC_SW_ENABLED + RC_ACTIVE_HI + AUX1;
//                    RC_SW_ENABLED:  ENABLE using rc command to arm
//                    RC_ACTIVE_HI:   rc command is enabled when selected channel is HIGH
//                    AUX1:           using AUX1
uint8_t rcCmdArm = RC_SW_ENABLED + RC_ACTIVE_LO + AUX1_SC;
uint8_t rcCmdAlti = RC_SW_DISABLED + RC_ACTIVE_NONE + AUX4_SC;
uint8_t rcCmdLevel = RC_SW_ENABLED + RC_ACTIVE_LO + AUX2_SC;
uint8_t rcCmdRace = RC_SW_ENABLED + RC_ACTIVE_LO + AUX3_SC;
uint8_t rcCmdHorizon = RC_SW_ENABLED + RC_ACTIVE_LO + AUX4_SC;

extern volatile float rx[4];
extern char aux[AUXNUMBER];
extern int armed_state;

void stickCommandTask(void) {
   static uint8_t lastStickPos = 0;
   // static uint8_t lastArmSwPos = 0;
   // uint8_t rcCmdArmNewState = 0;
   uint8_t stickPos = 0;

   if (rx[ROLL_SC] > -0.4f) {
      stickPos |= 0x80;
   }
   if (rx[ROLL_SC] < 0.6f) {
      stickPos |= 0x40;
   }
   stickPos >>= 2;
   if (rx[PITCH_SC] > -0.4f) {
      stickPos |= 0x80;
   }
   if (rx[PITCH_SC] < 0.6f) {
      stickPos |= 0x40;
   }
   stickPos >>= 2;
   if (rx[YAW_SC] > -0.4f) {
      stickPos |= 0x80;
   }
   if (rx[YAW_SC] < 0.6f) {
      stickPos |= 0x40;
   }
   stickPos >>= 2;
   if (rx[THROTTLE_SC] > 0.2f) {
      stickPos |= 0x80;
   }
   if (rx[THROTTLE_SC] < 0.8f) {
      stickPos |= 0x40;
   }

   if (stickPos != lastStickPos) {
      lastStickPos = stickPos;
      // if (FC_DISARMED()) {
      if (!armed_state) {
         switch (stickPos) {
         case RC_SC_OSD_ACTIVATE:
            menuHandleKey(menu_Join);
            break;
         case RC_SC_OSD_UP:
            menuHandleKey(menu_Up);
            break;
         case RC_SC_OSD_DOWN:
            menuHandleKey(menu_Down);
            break;
         case RC_SC_OSD_RIGHT:
            menuHandleKey(menu_Right);
            break;
         case RC_SC_OSD_LEFT:
            menuHandleKey(menu_Left);
            break;
         default:
            break;
         }
      }
      if (stickPos == RC_SC_FC_ARM) {
      }
      if (stickPos == RC_SC_FC_DISARM) {
      }
   }
}

uint8_t getAuxCommand(uint8_t auxCommand) {
   if (auxCommand & RC_SW_MASK) {
      uint8_t chanStatus = aux[(auxCommand & RC_CH_MASK) - AUX1_SC];
      uint8_t rcCmdCfg = ((auxCommand & RC_CFG_MASK) >> RC_CFG_OFFSET);
      if (rcCmdCfg == RC_NONE_MASK) {
         return 1;
      } else {
         if (chanStatus == rcCmdCfg) {
            return 1;
         } else {
            return 0;
         }
      }
   }
   return 0;
}

#endif
