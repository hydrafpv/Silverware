#ifndef _MENU_OSD_H_
#define _MENU_OSD_H_

#include "menu.h"
#include "stm32f0xx.h"

extern const menu_t osdMenu;

void osdMenuSaveFun(void);
void osdMenuInit(void);

#endif // #ifndef _MENU_OSD_H_
