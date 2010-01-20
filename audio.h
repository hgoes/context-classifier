#ifndef __AUDIO_H__
#define __AUDIO_H__

// Standard stuff
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "types.h"
#include "performance.h"

#ifdef INT_FFT
#include "kiss_fftr.h"
#else
// Fast fourier transformation
#include <fftw3.h>
#endif

// Alsa stuff
#include <alsa/asoundlib.h>

typedef struct {
  snd_pcm_t* handle;
#ifdef INT_FFT
  audio_t buf[AUDIO_SAMPLE_COUNT*2];
  kiss_fftr_cfg plan;
  kiss_fft_cpx tbuf[AUDIO_SAMPLE_COUNT+1];
#else
#ifndef NATIVE_AUDIO_FORMAT_DOUBLE
  audio_t buf[AUDIO_SAMPLE_COUNT*2];
#endif
  double dbuf[AUDIO_SAMPLE_COUNT*2];
  fftw_complex tbuf[AUDIO_SAMPLE_COUNT+1];
  fftw_plan plan;
#endif
} audio_system;

int init_audio_system(audio_system* sys);
void destroy_audio_system(audio_system* sys);
int fetch_audio_sample(audio_system* sys,double* vec);
const char* audio_error(int code);

#ifdef INT_FFT
typedef kiss_fft_cpx complex_type;
#define COMPLEX_R(arg) arg.r
#define COMPLEX_I(arg) arg.i
#else
typedef fftw_complex complex_type;
#define COMPLEX_R(arg) arg[0]
#define COMPLEX_I(arg) arg[1]
#endif

#endif
