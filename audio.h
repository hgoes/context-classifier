#ifndef __AUDIO_H__
#define __AUDIO_H__

// Standard stuff
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "types.h"

// Fast fourier transformation
#include <fftw3.h>

// Alsa stuff
#include <alsa/asoundlib.h>

typedef struct {
  snd_pcm_t* handle;
#if NATIVE_AUDIO_FORMAT != DOUBLE
  audio_t buf[AUDIO_SAMPLE_COUNT*2];
#endif
  double dbuf[AUDIO_SAMPLE_COUNT*2];
  fftw_complex tbuf[AUDIO_SAMPLE_COUNT+1];
  fftw_plan plan;
} audio_system;

int init_audio_system(audio_system* sys);
void destroy_audio_system(audio_system* sys);
int fetch_audio_sample(audio_system* sys,double* vec);
const char* audio_error(int code);

#endif
