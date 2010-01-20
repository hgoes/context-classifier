project(context-classifier)

set(INT_FFT true)
set(AUDIO_FORMAT "S16")

if(INT_FFT AND ${AUDIO_FORMAT} STREQUAL "DOUBLE")
  message(FATAL_ERROR "Integer FFT and double audio format not supported")
endif()

set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)

if(INT_FFT)
  set(FFT_SOURCES kiss_fft kiss_fftr)
else()
  set(FFT_LIB fftw3)
endif()

add_executable(test classifier audio vec rule ini_parser movement ${FFT_SOURCES})

target_link_libraries(test asound)
if(FFT_LIB)
  target_link_libraries(test ${FFT_LIB})
endif()

set(CMAKE_C_FLAGS "-g -Wall -DNATIVE_AUDIO_FORMAT_${AUDIO_FORMAT}")

if(INT_FFT)
  if(${AUDIO_FORMAT} STREQUAL "S16")
    set(FIXED_POINT_BITS 16)
  elseif(${AUDIO_FORMAT} STREQUAL "S32")
    set(FIXED_POINT_BITS 32)
  else()
    message(FATAL_ERROR "Fixed FFT can only be done with audio data types S16 or S32")
  endif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DINT_FFT -DFIXED_POINT=${FIXED_POINT_BITS}")
endif()