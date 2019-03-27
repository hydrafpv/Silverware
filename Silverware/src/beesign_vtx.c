#include "stm32f0xx.h"
#include "config.h"
#if defined (USE_BEESIGN)
#include "beesign_vtx.h"
#include "beesign.h"

const uint16_t vtx58frequencyTable[VTX_STRING_BAND_COUNT][VTX_STRING_CHAN_COUNT] =
{
    { 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725 }, // Boscam A
    { 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866 }, // Boscam B
    { 5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945 }, // Boscam E
    { 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880 }, // FatShark
    { 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917 }, // RaceBand
};

const char * const vtx58BandNames[] = {
    "--------",
    "BOSCAM A",
    "BOSCAM B",
    "BOSCAM E",
    "FATSHARK",
    "RACEBAND",
};

const char * const vtx58ChannelNames[] = {
    "-", "1", "2", "3", "4", "5", "6", "7", "8",
};

const char * const bsPowerNames[] = {
    "---",  "0.1", "25",
};

//Converts band and channel values to a frequency (in MHz) value.
// band:  Band value (1 to 5).
// channel:  Channel value (1 to 8).
// Returns frequency value (in MHz), or 0 if band/channel out of range.
uint16_t vtx58_BandchanFreq(uint8_t band, uint8_t channel) {
    if (band > 0 && band <= VTX_STRING_BAND_COUNT &&
                          channel > 0 && channel <= VTX_STRING_CHAN_COUNT) {
        return vtx58frequencyTable[band - 1][channel - 1];
    }
    return 0;
}

void bsVtxSetBandAndChannel(uint8_t band, uint8_t channel) {
    if (bsValidateBandAndChannel(band, channel)) {
        bsSetBandAndChannel(band, channel, BEESIGN_CMD_ADD_BUFF);
    }
}

void bsVtxSetPowerIndex(uint8_t index) {
    bsSetPower(index, BEESIGN_CMD_ADD_BUFF);
}

void bsVtxSetFreq(uint16_t freq) {
    if (bsValidateFreq(freq)) {
        bsSetFreq(freq, BEESIGN_CMD_ADD_BUFF);
    }
}
#endif

