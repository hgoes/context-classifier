#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include "config.h"

#ifdef NATIVE_AUDIO_FORMAT_S16
typedef int16_t audio_t;
#define PCM_FORMAT SND_PCM_FORMAT_S16_LE
#endif

#ifdef NATIVE_AUDIO_FORMAT_DOUBLE
typedef double audio_t;
#define PCM_FORMAT SND_PCM_FORMAT_FLOAT64
#endif

#ifndef PCM_FORMAT
#error "No native audio format defined"
#endif

#endif
