#include "config.h"
#include "stm32f0xx.h"
#if defined(USE_BEESIGN)
#include "menu.h"
#include "menu_fc.h"
#include "menu_osd.h"
#include "menu_rx.h"
#include "menu_vtx.h"
#include "string.h"
#include <stdbool.h>

#define MENU_INTERVAL_COUNT 30
#define MENU_DYNAMIC_COUNT 600

typedef struct menuStatus_s {
   const menu_t *menu; // menu for this context
   int8_t page;        // page in the menu
   int8_t cursorRow;   // cursorRow in the page
   uint8_t cursorLock; // cursor is lock, can't move
   int8_t pageCount;   // page count in the menu
   uint8_t pageMaxRow; // max row in the page
} menuStatus_t;

static menuStatus_t currentMenu;
static menuEntry_t *pageTop; // First entry for the current page
static uint8_t newPageFlg;
static int8_t displayCursorRow;
static uint8_t dispalyInMenu = 0;

static void mainMenuEnterFun(void);
static void mainMenuExitFun(void);
static uint8_t mainMenuSaveFun(void *key);

menuEntry_t mainMenuEntries[] = {
    {"- MENU -", menu_Label, NULL, NULL, 0},
    {"VTX", menu_Submenu, NULL, &vtxMenu, 0},
    {"OSD", menu_Submenu, NULL, &osdMenu, 0},
    {"FC", menu_Submenu, NULL, &fcMenu, 0},
    {"RX", menu_Submenu, NULL, &rxMenu, 0},
    {"SAVE&EXIT", menu_Exit, mainMenuSaveFun, NULL, 0},
    {"EXIT", menu_Exit, NULL, NULL, 0},
    {NULL, menu_End, NULL, NULL, 0}};

const menu_t mainMenu = {
    .uperMenu = NULL,
    .onEnter = mainMenuEnterFun,
    .onExit = mainMenuExitFun,
    .entries = mainMenuEntries};

static void mainMenuEnterFun(void) {
   bsSetOsdMode(BEESIGN_OSD_MODE_CUSTOM, BEESIGN_CMD_ADD_BUFF);
}

static void mainMenuExitFun(void) {
   menuClearPage();
   bsSetOsdMode(BEESIGN_OSD_MODE_MINI, BEESIGN_CMD_ADD_BUFF);
   bsSaveSetting(BEESIGN_CMD_ADD_BUFF);
}

static uint8_t mainMenuSaveFun(void *key) {
   extern void flash_save(void);
   key = key;
   vtxMenuSaveFun();
   osdMenuSaveFun();
   fcMenuSaveFun();
   flash_save();
   return 0;
}

static char *_i2a(unsigned i, char *a, unsigned base) {
   if (i / base > 0)
      a = _i2a(i / base, a, base);
   *a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % base];
   return a + 1;
}

static char *itoa(int i, char *a, int base) {
   if ((base < 2) || (base > 36))
      base = 10;
   if (i < 0) {
      *a = '-';
      *_i2a(-(unsigned)i, a + 1, base) = 0;
   } else
      *_i2a(i, a, base) = 0;
   return a;
}

// static void menuFormatFloat(int32_t value, char *floatString)
// {
//     uint8_t k;
//     // np. 3450

//     itoa(100000 + value, floatString, 10); // Create string from abs of integer value

//     // 103450

//     floatString[0] = floatString[1];
//     floatString[1] = floatString[2];
//     floatString[2] = '.';

//     // 03.450
//     // usuwam koncowe zera i kropke
//     // Keep the first decimal place
//     for (k = 5; k > 3; k--)
//         if (floatString[k] == '0' || floatString[k] == '.')
//             floatString[k] = 0;
//         else
//             break;

//     // oraz zero wiodonce
//     if (floatString[0] == '0')
//         floatString[0] = ' ';
// }

static void menuUpdateMaxRow(void) {
   currentMenu.pageMaxRow = 0;
   for (menuEntry_t *ptr = pageTop; ptr->type != menu_End; ptr++) {
      currentMenu.pageMaxRow++;
   }
   if (currentMenu.pageMaxRow > SCREEN_LINES) {
      currentMenu.pageMaxRow = SCREEN_LINES;
   }
   currentMenu.pageMaxRow--;
}

//static uint8_t menuCursorAbsolute(void) {
//    return currentMenu.cursorRow + currentMenu.page * SCREEN_LINES;
//}

static void menuPageSelect(int8_t newPage) {
   currentMenu.page = ((int8_t)newPage + currentMenu.pageCount) % currentMenu.pageCount;
   if (pageTop != &(currentMenu.menu->entries[currentMenu.page * SCREEN_LINES])) {
      pageTop = &(currentMenu.menu->entries[currentMenu.page * SCREEN_LINES]);
      menuUpdateMaxRow();
      newPageFlg = 1;
      displayCursorRow = -1;
      menuClearPage();
   }
}

static void menuPageNext(void) {
   menuPageSelect(currentMenu.page + 1);
}

static void menuPagePrev(void) {
   menuPageSelect(currentMenu.page - 1);
}

// Pad buffer to the left, i.e. align right
static void menuPadLeftToSize(char *buf, int size) {
   int i, j;
   int len = strlen(buf);

   for (i = size - 1, j = size - len; i - j >= 0; i--) {
      buf[i] = buf[i - j];
   }

   for (; i >= 0; i--) {
      buf[i] = ' ';
   }

   buf[size] = 0;
}

static uint8_t menuDrawEntry(menuEntry_t *p, uint8_t row) {
   char buff[MENU_ENTRY_MAX_CHARS + 1] = {0x00};
   if (p->type >= menu_End)
      return 1;

   switch (p->type) {
   case menu_Submenu:
   case menu_FunCall:
      if (IS_PRINTVALUE(p)) {
         strncpy(buff, ">", MENU_ENTRY_MAX_CHARS);
         menuPadLeftToSize(buff, MENU_ENTRY_MAX_CHARS);
         menuShowChars(SCREEN_CHARS_PER_ROW - MENU_ENTRY_MAX_CHARS - 1, row, buff);
         CLR_PRINTVALUE(p);
      }
      break;
   case menu_Uint8:
      if (IS_PRINTVALUE(p) && p->data) {
         const menuUint8_t *ptr = p->data;
         if (*ptr->val != *ptr->lastVal) {
            *ptr->lastVal = *ptr->val;
            itoa(*ptr->val, buff, 10);
            menuPadLeftToSize(buff, MENU_ENTRY_MAX_CHARS);
            menuShowChars(SCREEN_CHARS_PER_ROW - MENU_ENTRY_MAX_CHARS - 1, row, buff);
         }
         CLR_PRINTVALUE(p);
      }
      break;
   case menu_Uint16:
      if (IS_PRINTVALUE(p) && p->data) {
         const menuUint16_t *ptr = p->data;
         if (*ptr->val != *ptr->lastVal) {
            *ptr->lastVal = *ptr->val;
            itoa(*ptr->val, buff, 10);
            menuPadLeftToSize(buff, MENU_ENTRY_MAX_CHARS);
            menuShowChars(SCREEN_CHARS_PER_ROW - MENU_ENTRY_MAX_CHARS - 1, row, buff);
         }
         CLR_PRINTVALUE(p);
      }
      break;
   case menu_Tab:
      if (IS_PRINTVALUE(p) && p->data) {
         const menuTab_t *ptr = p->data;
         if (*ptr->val != *ptr->lastVal) {
            char *str = (char *)ptr->names[*ptr->val];
            *ptr->lastVal = *ptr->val;
            strncpy(buff, str, MENU_ENTRY_MAX_CHARS);
            menuPadLeftToSize(buff, MENU_ENTRY_MAX_CHARS);
            menuShowChars(SCREEN_CHARS_PER_ROW - MENU_ENTRY_MAX_CHARS - 1, row, buff);
         }
         CLR_PRINTVALUE(p);
      }
      break;
   // case menu_Float:
   //     if (IS_PRINTVALUE(p) && p->data) {
   //         menuFloat_t *ptr = p->data;
   //         // itoa(*ptr->val, buff, 10);
   //         menuFormatFloat(*ptr->val * ptr->multipler, buff);
   //         menuPadLeftToSize(buff, MENU_ENTRY_MAX_CHARS);
   //         menuShowChars(SCREEN_CHARS_PER_ROW - MENU_ENTRY_MAX_CHARS - 1, row, buff);
   //         CLR_PRINTVALUE(p);
   //     }
   //     break;
   // case menu_Int8:
   //     if (IS_PRINTVALUE(p) && p->data) {
   //         menuUint8_t *ptr = p->data;
   //         itoa(*ptr->val, buff, 10);
   //         menuPadLeftToSize(buff, MENU_ENTRY_MAX_CHARS);
   //         menuShowChars(SCREEN_CHARS_PER_ROW - MENU_ENTRY_MAX_CHARS - 1, row, buff);
   //         CLR_PRINTVALUE(p);
   //     }
   //     break;
   // case menu_Int16:
   //     if (IS_PRINTVALUE(p) && p->data) {
   //         menuUint16_t *ptr = p->data;
   //         itoa(*ptr->val, buff, 10);
   //         menuPadLeftToSize(buff, MENU_ENTRY_MAX_CHARS);
   //         menuShowChars(SCREEN_CHARS_PER_ROW - MENU_ENTRY_MAX_CHARS - 1, row, buff);
   //         CLR_PRINTVALUE(p);
   //     }
   //     break;
   case menu_String:
      if (IS_PRINTVALUE(p) && p->data) {
         const menuString_t *ptr = p->data;
         strncpy(buff, ptr->str, MENU_ENTRY_MAX_CHARS);
         menuPadLeftToSize(buff, MENU_ENTRY_MAX_CHARS);
         menuShowChars(SCREEN_CHARS_PER_ROW - MENU_ENTRY_MAX_CHARS - 1, row, buff);
         CLR_PRINTVALUE(p);
      }
      break;
   default:
      break;
   }
   return 0;
}

static void menuDrawPage(uint32_t TimesCount) {
   bool drawPolled = false;
   static uint32_t lastPolledCount = 0;
   static uint32_t lastDynamicCount = 0;
   menuEntry_t *p;
   uint8_t i;

   if (!dispalyInMenu)
      return;
   if (TimesCount > lastPolledCount + MENU_INTERVAL_COUNT) {
      drawPolled = true;
      lastPolledCount = TimesCount;
   } else {
      return;
   }

   if (newPageFlg) {
      for (p = pageTop, i = 0; p->type != menu_End; p++, i++) {
         SET_PRINTLABEL(p);
         SET_PRINTVALUE(p);
      }
      newPageFlg = 0;
   } else if (drawPolled && TimesCount - lastDynamicCount > MENU_DYNAMIC_COUNT) {
      for (p = pageTop; p <= pageTop + currentMenu.pageMaxRow; p++) {
         if (IS_DYNAMIC(p)) {
            lastDynamicCount = TimesCount;
            SET_PRINTVALUE(p);
         }
      }
   }

   // Print text labels
   for (i = 0, p = pageTop; i < SCREEN_LINES && p->type != menu_End; i++, p++) {
      if (IS_PRINTLABEL(p)) {
         uint8_t coloff = SELECT_SET_IN_ROW + 1;
         coloff = (p->type == menu_Label) ? 0 : coloff;
         menuShowChars(coloff, i, p->text);
         CLR_PRINTLABEL(p);
         return;
      }

      // Print values

      // XXX Polled values at latter positions in the list may not be
      // XXX printed if not enough room in the middle of the list.

      if (IS_PRINTVALUE(p)) {
         menuDrawEntry(p, i);
         CLR_PRINTVALUE(p);
         return;
      }
   }
   if (displayCursorRow != currentMenu.cursorRow) {
      menuShowChars(SELECT_SET_IN_ROW, displayCursorRow, " ");
      menuShowChars(SELECT_SET_IN_ROW, currentMenu.cursorRow, ">");
      displayCursorRow = currentMenu.cursorRow;
   }
}

static void menuCountPage(void) {
   menuEntry_t *p;
   for (p = currentMenu.menu->entries; p->type != menu_End; p++)
      ;
   currentMenu.pageCount = (p - currentMenu.menu->entries - 1) / SCREEN_LINES + 1;
}

static void menuChange(const menu_t *pMenu) {
   menuEntry_t *p;

   currentMenu.menu = pMenu;
   currentMenu.cursorRow = 0;
   displayCursorRow = 0;
   if (pMenu) {
      for (p = currentMenu.menu->entries; p->type == menu_Label || p->type == NULL; p++) {
         currentMenu.cursorRow++;
      }
      if (currentMenu.menu->onEnter)
         currentMenu.menu->onEnter();
      menuCountPage();
      menuPageSelect(0);
   } else {
      currentMenu.page = 0;
      currentMenu.pageCount = 0;
      currentMenu.pageMaxRow = 0;
   }

   // menuShowChars(SELECT_SET_IN_ROW, currentMenu.menu->cursorRow, '>');
}

static void menuBack(void) {
   if (currentMenu.menu->onExit)
      currentMenu.menu->onExit();
   menuChange(currentMenu.menu->uperMenu);
}

static uint8_t menuJoin(void) {
   if (dispalyInMenu)
      return 1;

   menuChange(&mainMenu);
   dispalyInMenu = 1;

   return dispalyInMenu;
}

static uint8_t menuExit(void) {
   if (!dispalyInMenu)
      return 0;
   if (currentMenu.menu->onExit)
      currentMenu.menu->onExit();
   menuChange(NULL);
   dispalyInMenu = 0;
   pageTop = NULL;
   return dispalyInMenu;
}

uint8_t menuHandleKey(menuControl_e order) {
   menuEntry_t *p;
   if (!currentMenu.menu && order != menu_Join)
      return 0;
   switch (order) {
   case menu_Join:
      menuJoin();
      break;
   case menu_Up:
      if (currentMenu.cursorLock) {
         p = pageTop + currentMenu.cursorRow;
         currentMenu.cursorLock = (pageTop + currentMenu.cursorRow)->func(&order);
         SET_PRINTVALUE(p);
      } else {
         currentMenu.cursorRow--;
         if ((pageTop + currentMenu.cursorRow)->type == menu_Label && currentMenu.cursorRow > 0)
            currentMenu.cursorRow--;
         if (currentMenu.cursorRow == -1 || (pageTop + currentMenu.cursorRow)->type == menu_Label) {
            // Goto previous page
            menuPagePrev();
            currentMenu.cursorRow = currentMenu.pageMaxRow;
         }
      }
      break;
   case menu_Down:
      if (currentMenu.cursorLock) {
         p = pageTop + currentMenu.cursorRow;
         currentMenu.cursorLock = (pageTop + currentMenu.cursorRow)->func(&order);
         SET_PRINTVALUE(p);
      } else {
         if (currentMenu.cursorRow < currentMenu.pageMaxRow) {
            currentMenu.cursorRow++;
         } else {
            menuPageNext();
            currentMenu.cursorRow = 0; // Goto top in any case
            if ((pageTop + currentMenu.cursorRow)->type == menu_Label) {
               currentMenu.cursorRow++;
            }
         }
      }
      break;
   case menu_Left:
   case menu_Right:
      p = pageTop + currentMenu.cursorRow;
      switch (p->type) {
      case menu_Submenu:
         if (order == menu_Right) {
            menuChange(p->data);
         }
         break;
      case menu_FunCall:
         if (p->func && (order == menu_Right)) {
            currentMenu.cursorLock = p->func(&order);
         }
         break;
      case menu_Back:
         menuBack();
         break;
      case menu_Exit:
         if (p->func && (order == menu_Right)) {
            currentMenu.cursorLock = p->func(&order);
         }
         menuExit();
         break;
      case menu_Uint8:
         if (p->data) {
            const menuUint8_t *ptr = p->data;
            if (order == menu_Right) {
               if (*ptr->val < ptr->max)
                  *ptr->val += ptr->step;
            } else {
               if (*ptr->val > ptr->min)
                  *ptr->val -= ptr->step;
            }
            SET_PRINTVALUE(p);
            if (p->func) {
               currentMenu.cursorLock = p->func(&order);
            }
         }
         break;
      case menu_Uint16:
         if (p->data) {
            const menuUint16_t *ptr = p->data;
            if (order == menu_Right) {
               if (*ptr->val < ptr->max)
                  *ptr->val += ptr->step;
            } else {
               if (*ptr->val > ptr->min)
                  *ptr->val -= ptr->step;
            }
            SET_PRINTVALUE(p);
            if (p->func) {
               currentMenu.cursorLock = p->func(&order);
            }
         }
         break;
      case menu_String:
         if (p->data) {
            SET_PRINTVALUE(p);
            if (p->func) {
               currentMenu.cursorLock = p->func(&order);
            }
         }
         break;
      // case menu_Int8:
      //     if (p->data) {
      //         menuInt8_t *ptr = p->data;
      //         if (order == menu_Right) {
      //             if (*ptr->val < ptr->max)
      //                 *ptr->val += ptr->step;
      //         }
      //         else {
      //             if (*ptr->val > ptr->min)
      //                 *ptr->val -= ptr->step;
      //         }
      //         SET_PRINTVALUE(p);
      //         if (p->func) {
      //             currentMenu.cursorLock = p->func(&order);
      //         }
      //     }
      //     break;
      // case menu_Float:
      //     if (p->data) {
      //         menuFloat_t *ptr = p->data;
      //         if (order == menu_Right) {
      //             if (*ptr->val < ptr->max)
      //                 *ptr->val += ptr->step;
      //         }
      //         else {
      //             if (*ptr->val > ptr->min)
      //                 *ptr->val -= ptr->step;
      //         }
      //         SET_PRINTVALUE(p);
      //         if (p->func) {
      //             currentMenu.cursorLock = p->func(&order);
      //         }
      //     }
      //     break;
      // case menu_Int16:
      //     if (p->data) {
      //         menuInt16_t *ptr = p->data;
      //         if (order == menu_Right) {
      //             if (*ptr->val < ptr->max)
      //                 *ptr->val += ptr->step;
      //         }
      //         else {
      //             if (*ptr->val > ptr->min)
      //                 *ptr->val -= ptr->step;
      //         }
      //         SET_PRINTVALUE(p);
      //         if (p->func) {
      //             currentMenu.cursorLock = p->func(&order);
      //         }
      //     }
      //     break;
      case menu_Tab:
         if (p->type == menu_Tab) {
            const menuTab_t *ptr = p->data;

            if (order == menu_Right) {
               if (*ptr->val < ptr->max)
                  *ptr->val += 1;
            } else {
               if (*ptr->val > 0)
                  *ptr->val -= 1;
            }
            SET_PRINTVALUE(p);
            if (p->func)
               currentMenu.cursorLock = p->func(&order);
         }
         break;
      default:
         break;
      }
      break;
   default:
      break;
   }
   return 1;
}

uint8_t getMenuState(void) {
   return dispalyInMenu;
}

void menuTask(void) {
   static uint32_t currentTimesCount;
   currentTimesCount++;
   menuDrawPage(currentTimesCount);
}

void menuInit(void) {
   vtxMenuInit();
   osdMenuInit();
   fcMenuInit();
}

void menuJoinRxMenu(void) {
   if (dispalyInMenu)
      return;
   menuJoin();
   menuChange(&rxMenu);
}

void menuExitRxMenu(void) {
   if (!dispalyInMenu)
      return;
   bsSetOsdMode(BEESIGN_OSD_MODE_MINI, BEESIGN_CMD_ADD_BUFF);
   menuExit();
}
#endif
