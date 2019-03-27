#ifndef _STICK_COMMAND_H_
#define _STICK_COMMAND_H_

#define RX_CHAN_MIN             1000
#define RX_CHAN_MAX             2000
#define RX_CHAN_THRESHOLD_LO    1200
#define RX_CHAN_THRESHOLD_HI    1800
#define RX_CHAN_CENTRAL         1500

#define RX_NON_AUX_CH_COUNT     4
#define RX_AUX_CH_COUNT         (RX_CHAN_COUNT - RX_NON_AUX_CH_COUNT)

// stick commands combos
#define RC_SC_ALL_CE                (THR_CE + YAW_CE + PIT_CE + ROL_CE)
#define RC_SC_OSD_ACTIVATE          (THR_CE + YAW_LO + PIT_HI + ROL_CE)
#define RC_SC_OSD_UP                (THR_CE + YAW_CE + PIT_HI + ROL_CE)
#define RC_SC_OSD_DOWN              (THR_CE + YAW_CE + PIT_LO + ROL_CE)
#define RC_SC_OSD_RIGHT             (THR_CE + YAW_CE + PIT_CE + ROL_HI)
#define RC_SC_OSD_LEFT              (THR_CE + YAW_CE + PIT_CE + ROL_LO)
#define RC_SC_FC_ARM                (THR_LO + YAW_HI + PIT_CE + ROL_CE)
#define RC_SC_FC_DISARM             (THR_LO + YAW_LO + PIT_CE + ROL_CE)

#define RC_SW_OFFSET    7
#define RC_CFG_OFFSET   5
#define RC_NONE_MASK    0x03
#define RC_SW_MASK      0x80
#define RC_CFG_MASK     0x60
#define RC_CH_MASK      0x07
#define RC_SW_ENABLED   (1 << RC_SW_OFFSET)
#define RC_SW_DISABLED  (0 << RC_SW_OFFSET)
#define RC_ACTIVE_LO    (0 << RC_CFG_OFFSET)
#define RC_ACTIVE_CE    (1 << RC_CFG_OFFSET)
#define RC_ACTIVE_HI    (2 << RC_CFG_OFFSET)
#define RC_ACTIVE_NONE  (3 << RC_CFG_OFFSET)

// Aileron	--> roll
// Elevator --> pitch
// Throttle --> throttle
// Rudder   --> yaw

typedef enum rx_channel_order_e {
    RX_CH_ORDER_TAER = 0,
    RX_CH_ORDER_AETR
} RX_CHANNEL_ORDER_E;

typedef enum rx_channel_aetr_e {
    ROLL_AETR = 0,
    PITCH_AETR,
    THROTTLE_AETR,
    YAW_AETR
} RX_CHANNEL_AETR_E;

typedef enum rx_channel_taer_e {
    THROTTLE_TAER = 0,
    ROLL_TAER,
    PITCH_TAER,
    YAW_TAER
} RX_CHANNEL_TAER_E;

typedef enum rx_chan_e {
    ROLL_SC = 0,
    PITCH_SC,
    YAW_SC,
    THROTTLE_SC,
    AUX1_SC,
    AUX2_SC,
    AUX3_SC,
    AUX4_SC,
    RX_CHAN_COUNT
} rxChan_e;

extern uint8_t rcCmdArm;
extern uint8_t rcCmdAlti;
extern uint8_t rcCmdLevel;
extern uint8_t rcCmdRace;
extern uint8_t rcCmdHorizon;

void stickCommandTask(void);
uint8_t getAuxCommand(uint8_t auxCommand);

#endif  // #ifndef _STICK_COMMAND_H_
