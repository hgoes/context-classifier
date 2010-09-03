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

void calculate_statistics(complex_type* from,int sz,double* mean,double* var,double* centr) {
  int i;
  double value;
  double acc = 0;
  double accf = 0;
  double accp = 0;
  for(i=0;i<sz;i++) {
    value = sqrt((double)(COMPLEX_R(from[i])*COMPLEX_R(from[i]) + COMPLEX_I(from[i])*COMPLEX_I(from[i])));
    //printf("%d: %f %f %f\n",i,value,from[i][0],from[i][1]);
    acc += value;
    accf += (i+1)*value*value;
    accp += value*value;
  }
  *mean = acc / sz;
  *centr = accf / accp;
  acc = 0;
  for(i=0;i<sz;i++) {
    value = sqrt((double)(COMPLEX_R(from[i])*COMPLEX_R(from[i]) + COMPLEX_I(from[i])*COMPLEX_I(from[i])));
    value -= *mean;
    acc += value*value;
  }
  *var = acc / sz;
}

int fetch_audio_sample(audio_system* sys,double* vec,char** ground_truth,const int* semantics) {
  int err;
#ifdef INT_FFT
  if((err = snd_pcm_readi(sys->handle, sys->buf, AUDIO_SAMPLE_COUNT)) != AUDIO_SAMPLE_COUNT) {
    return err;
  }
#else
#ifndef NATIVE_AUDIO_FORMAT_DOUBLE
  if((err = snd_pcm_readi(sys->handle, sys->buf, AUDIO_SAMPLE_COUNT)) != AUDIO_SAMPLE_COUNT) {
    return err;
  }
  audio_buffer_to_double(sys->buf,sys->dbuf,AUDIO_SAMPLE_COUNT*2);
#else
  if((err = snd_pcm_readi(sys->handle, sys->dbuf, AUDIO_SAMPLE_COUNT)) != AUDIO_SAMPLE_COUNT) {
    return err;
  }
#endif
#endif

  MEASURED("audio preprocessing",{
#ifdef INT_FFT
      kiss_fftr(sys->plan,sys->buf,sys->tbuf);
#else
      fftw_execute(sys->plan);
#endif
      calculate_statistics(sys->tbuf,AUDIO_SAMPLE_COUNT/2,&vec[semantics[1]],&vec[semantics[3]],&vec[semantics[5]]);
      calculate_statistics(&(sys->tbuf[AUDIO_SAMPLE_COUNT/2+1]),AUDIO_SAMPLE_COUNT/2,&vec[semantics[2]],&vec[semantics[4]],&vec[semantics[6]]);
      //calculate_statistics(sys->tbuf,AUDIO_SAMPLE_COUNT,&vec[0],&vec[1],&vec[2]);
    });
  *ground_truth = NULL;
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
#ifdef INT_FFT
  sys->plan = kiss_fftr_alloc(AUDIO_SAMPLE_COUNT*2,0,NULL,NULL);
#else
  sys->plan = fftw_plan_dft_r2c_1d(AUDIO_SAMPLE_COUNT*2,sys->dbuf,sys->tbuf,0);
#endif
  return 0;
 error_malloc:
  snd_pcm_hw_params_free(hw_params);
 error_open:
  snd_pcm_close(sys->handle);
  return err;
}

void destroy_audio_system(audio_system* sys) {
  snd_pcm_close(sys->handle);
#ifdef INT_FFT
  kiss_fftr_free(sys->plan);
#else
  fftw_destroy_plan(sys->plan);
#endif
}

static void audio_plugin_destructor(audio_system* sys) {
  destroy_audio_system(sys);
  free(sys);
}

static int audio_semantics(const char* term) {
  if(strcmp(term,"mean_1")==0) return 1;
  if(strcmp(term,"mean_2")==0) return 2;
  if(strcmp(term,"var_1")==0) return 3;
  if(strcmp(term,"var_2")==0) return 4;
  if(strcmp(term,"centr_1")==0) return 5;
  if(strcmp(term,"centr_2")==0) return 6;
  return -1;
}

plugin_t* get_audio_plugin() {
  audio_system* sys = malloc(sizeof(audio_system));
  plugin_t* res = malloc(sizeof(plugin_t));
  init_audio_system(sys);
  res->name = "audio";
  res->user_data = sys;
  res->feature_vector_size = 6;
  res->callback = (feature_getter_t)fetch_audio_sample;
  res->destructor = (plugin_destructor_t)audio_plugin_destructor;
  res->semantic_mapper = audio_semantics;
  return res;
}
