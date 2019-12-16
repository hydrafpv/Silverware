#ifndef _MENU_H_
#define _MENU_H_

#include "beesign.h"
#include "stm32f0xx.h"
#include <stdint.h>

#define SELECT_SET_IN_ROW 1
#define SCREEN_CHARS_PER_ROW BEESIGN_CHARS_PER_LINE
#define SCREEN_LINES BEESIGN_LINES_PER_SCREEN
#define MENU_ENTRY_MAX_CHARS 10

#define menuShowChars(x, row, data) bsSetDisplayOneRow(x, row, data, BEESIGN_CMD_ADD_BUFF)
#define menuClearPage() bsClearDispaly(BEESIGN_CMD_ADD_BUFF)

struct menu_s;

typedef enum {
   menu_None = 0,
   menu_Up,
   menu_Down,
   menu_Left,
   menu_Right,
   menu_Enter,
   menu_Join
} menuControl_e;

typedef enum {
   menu_Label = 0,
   menu_Back,
   menu_Exit,
   menu_Submenu,
   menu_String,
   menu_Uint8,
   menu_Uint16,
   // menu_Int8,
   // menu_Int16,
   // menu_Float,
   menu_Tab,
   menu_FunCall,
   menu_End,
   menu_Max
} menuElement_e;

typedef uint8_t (*menuEntryFuncPtr)(void *ptr);

typedef struct {
   const char *text;
   const menuElement_e type;
   const menuEntryFuncPtr func;
   const void *data;
   uint8_t flags;
} menuEntry_t;

// Bits in flags
#define PRINT_VALUE 0x01 // Value has been changed, need to redraw
#define PRINT_LABEL 0x02 // Text label should be printed
#define DYNAMIC 0x04     // Value should be updated dynamically

#define IS_PRINTVALUE(p) ((p)->flags & PRINT_VALUE)
#define SET_PRINTVALUE(p) \
   { (p)->flags |= PRINT_VALUE; }
#define CLR_PRINTVALUE(p) \
   { (p)->flags &= ~PRINT_VALUE; }

#define IS_PRINTLABEL(p) ((p)->flags & PRINT_LABEL)
#define SET_PRINTLABEL(p) \
   { (p)->flags |= PRINT_LABEL; }
#define CLR_PRINTLABEL(p) \
   { (p)->flags &= ~PRINT_LABEL; }

#define IS_DYNAMIC(p) ((p)->flags & DYNAMIC)

typedef void (*menuFuncPtr)(void);

typedef struct menu_s {
   const struct menu_s *uperMenu;
   const menuFuncPtr onEnter;
   const menuFuncPtr onExit;
   menuEntry_t *entries;
} const menu_t;

// typedef struct {
//     int8_t *val;
//     const int8_t min;
//     const int8_t max;
//     const int8_t step;
//     const uint16_t multipler;
// } menuFloat_t;

// typedef struct
// {
//     int8_t *val;
//     const int8_t min;
//     const int8_t max;
//     const int8_t step;
// } menuInt8_t;

// typedef struct
// {
//     int16_t *val;
//     const int16_t min;
//     const int16_t max;
//     const int16_t step;
// } menuInt16_t;

typedef struct {
   uint8_t *val;
   uint8_t *lastVal;
   const uint8_t min;
   const uint8_t max;
   const uint8_t step;
} menuUint8_t;

typedef struct {
   uint16_t *val;
   uint16_t *lastVal;
   const uint16_t min;
   const uint16_t max;
   const uint16_t step;
} menuUint16_t;

typedef struct {
   uint8_t *val;
   uint8_t *lastVal;
   const uint8_t max;
   const char *const *names;
} menuTab_t;

typedef struct {
   char *str;
} menuString_t;

extern const menu_t mainMenu;

uint8_t menuHandleKey(menuControl_e order);
uint8_t getMenuState(void);
void menuTask(void);
void menuInit(void);
void menuJoinRxMenu(void);
void menuExitRxMenu(void);

#endif // menu.h
