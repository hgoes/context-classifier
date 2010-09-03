/**
 * @file
 * @addtogroup sensors Sensor-Implementations
 * The audio subsystem. Responsible for extracting audio feature
 * vectors from the hardware.
 * @{
 */
#ifndef __AUDIO_H__
#define __AUDIO_H__

// Standard stuff
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "performance.h"
#include "plugin.h"

#ifdef INT_FFT
#include "kiss_fftr.h"
#else
// Fast fourier transformation
#include <fftw3.h>
#endif

// Alsa stuff
#include <alsa/asoundlib.h>

/**
 * The audio system type, contains all data to manage the audio
 * hardware and perform preprocessing.
 */
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

/**
 * Initialize the audio system. Allocate memory, set parameters, etc.
 */
int init_audio_system(audio_system* sys);
/**
 * Destroy the audio system. Free resources, memory, etc.
 */
void destroy_audio_system(audio_system* sys);
/**
 * Generate one audio feature vector.
 * @param vec The target for the feature vector. Must be long enough to store it.
 * @return 0 if a vector was succesfully fetched, <0 otherwise
 */
int fetch_audio_sample(audio_system* sys,double* vec,char** ground_truth,const int* semantics);
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

plugin_t* get_audio_plugin();

#endif
/**
 * @}
 */
