#ifndef JOEY_NATIVE_AUDIO_H
#define JOEY_NATIVE_AUDIO_H
#include <stdint.h>
#include "duel_audio_assets.h"
#define JOEY_AUDIO_NONE 255u
extern volatile uint32_t joey_audio_diagnostic[32];
int joey_audio_open(void);
void joey_audio_music(unsigned);
void joey_audio_effect(unsigned);
void joey_audio_tick(void);
void joey_audio_close(void);
#endif
