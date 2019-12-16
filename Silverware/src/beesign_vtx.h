#ifndef _BEESIGN_VTX_H_
#define _BEESIGN_VTX_H_

#include "beesign.h"
#include "stm32f0xx.h"

#define VTX_STRING_BAND_COUNT 5
#define VTX_STRING_CHAN_COUNT 8

extern const char *const vtx58BandNames[];
extern const char *const vtx58ChannelNames[];
extern const char *const bsPowerNames[];

uint16_t vtx58_BandchanFreq(uint8_t band, uint8_t channel);
void bsVtxSetBandAndChannel(uint8_t band, uint8_t channel);
void bsVtxSetPowerIndex(uint8_t index);
void bsVtxSetFreq(uint16_t freq);

#endif // #ifndef _BEESIGN_VTX_H_
