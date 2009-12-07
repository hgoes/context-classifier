#include "audio.h"

const char* audio_error(int code) {
  return snd_strerror(code);
}

inline void audio_buffer_to_double(audio_t* from,double* to,int sz) {
  int i;
  for(i=0; i<sz; i++) {
    to[i] = (double)from[i];
  }
}

void calculate_statistics(fftw_complex* from,int sz,double* mean,double* var,double* centr) {
  int i;
  double value;
  double acc = 0;
  double accf = 0;
  double accp = 0;
  for(i=0;i<sz;i++) {
    value = sqrt(from[i][0]*from[i][0] + from[i][1]*from[i][1]);
    //printf("%d: %f %f %f\n",i,value,from[i][0],from[i][1]);
    acc += value;
    accf += (i+1)*value*value;
    accp += value*value;
  }
  *mean = acc / sz;
  *centr = accf / accp;
  acc = 0;
  for(i=0;i<sz;i++) {
    value = sqrt(from[i][0]*from[i][0] + from[i][1]*from[i][1]);
    value -= *mean;
    acc += value*value;
  }
  *var = acc / sz;
}

int fetch_audio_sample(audio_system* sys,double* vec) {
  int err;
#if NATIVE_AUDIO_FORMAT!=DOUBLE
  if((err = snd_pcm_readi(sys->handle, sys->buf, AUDIO_SAMPLE_COUNT)) != AUDIO_SAMPLE_COUNT) {
    return err;
  }
  audio_buffer_to_double(sys->buf,sys->dbuf,AUDIO_SAMPLE_COUNT*2);
#else
  if((err = snd_pcm_readi(sys->handle, sys->dbuf, AUDIO_SAMPLE_COUNT)) != AUDIO_SAMPLE_COUNT) {
    return err;
  }
#endif
  fftw_execute(sys->plan);
  calculate_statistics(sys->tbuf,AUDIO_SAMPLE_COUNT/2,&vec[0],&vec[1],&vec[2]);
  calculate_statistics(&(sys->tbuf[AUDIO_SAMPLE_COUNT/2+1]),AUDIO_SAMPLE_COUNT/2,&vec[3],&vec[4],&vec[5]);
  return 0;
}

int init_audio_system(audio_system* sys) {
  int err;
  snd_pcm_hw_params_t* hw_params;
  if((err = snd_pcm_open(&(sys->handle),AUDIO_DEVICE, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    return err;
  }
  if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
    goto error_open;
  }
  if((err = snd_pcm_hw_params_any(sys->handle, hw_params)) < 0) {
    goto error_malloc;
  }
  if((err = snd_pcm_hw_params_set_access(sys->handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    goto error_malloc;
  }
  if((err = snd_pcm_hw_params_set_format(sys->handle, hw_params, PCM_FORMAT)) < 0) {
    goto error_malloc;
  }
  if((err = snd_pcm_hw_params_set_rate(sys->handle, hw_params, AUDIO_CAPTURE_RATE, 0)) < 0) {
    goto error_malloc;
  }
  if((err = snd_pcm_hw_params_set_channels(sys->handle, hw_params, 2)) < 0) {
    goto error_malloc;
  }
  if((err = snd_pcm_hw_params(sys->handle, hw_params)) < 0) {
    goto error_malloc;
  }
  snd_pcm_hw_params_free(hw_params);
  if((err = snd_pcm_prepare(sys->handle)) < 0) {
    goto error_open;
  }
  sys->plan = fftw_plan_dft_r2c_1d(AUDIO_SAMPLE_COUNT*2,sys->dbuf,sys->tbuf,0);
  return 0;
 error_malloc:
  snd_pcm_hw_params_free(hw_params);
 error_open:
  snd_pcm_close(sys->handle);
  return err;
}

void destroy_audio_system(audio_system* sys) {
  snd_pcm_close(sys->handle);
  fftw_destroy_plan(sys->plan);
}
