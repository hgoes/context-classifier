#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include "config.h"

#if NATIVE_AUDIO_FORMAT == S16
typedef int16_t audio_t;
#define PCM_FORMAT SND_PCM_FORMAT_S16_LE
#else
#error "No native audio format defined"
#endif

#endif
