#ifndef _MENU_FC_H_
#define _MENU_FC_H_

#include "stm32f0xx.h"
#include "menu.h"

extern const menu_t fcMenu;

void fcMenuSaveFun(void);
void fcMenuInit(void);

#endif // #ifndef _MENU_FC_H_
