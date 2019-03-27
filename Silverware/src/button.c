#include "project.h"
#include "config.h"
#include "buzzer.h"
#include "defines.h"

#if defined (USE_BUTTON)

#if defined (USE_BEESIGN)
#include "menu.h"
#endif

enum {
    button_none = 0,
    button_double_press = 1,
    button_long_press = 2,
};

void buttonInit(void) {
    
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(BUTTON_PIN_PORT,&GPIO_InitStructure);
}

uint8_t buttonDoublePress(void) {
    static uint8_t pressSecondFlg = 0;
    static uint8_t pressFirstFlg = 0;
    static uint16_t count = 0;
    if(GPIO_ReadInputDataBit(BUTTON_PIN_PORT, BUTTON_PIN)) {
        count++;
        if (pressFirstFlg == 0 && count > 1) {
            pressFirstFlg = 1;
        }
        if (pressSecondFlg) {
            pressFirstFlg = 0;
            pressSecondFlg = 0;
            count = 0;
            return button_double_press;
        }
        if (count > 500) {
            pressFirstFlg = 0;
            pressSecondFlg = 0;
            count = 0;
            return button_long_press;
        }
    } else {
        if (pressFirstFlg != 0) {
            count++;
            if (count > 500) {
                pressFirstFlg = 0;
                pressSecondFlg = 0;
                count = 0;
            } else {
                pressSecondFlg = 1;
            }
        }
    }
    return button_none;
}

void buttonTask(void) {
    if(buttonDoublePress() == button_double_press) {
        #if defined (USE_BEESIGN)
        menuJoinRxMenu();
        #endif
    }
    if(buttonDoublePress() == button_long_press) {
        #if defined (USE_BEESIGN)
        menuExitRxMenu();
        #endif
    }
}

#endif

