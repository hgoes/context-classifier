project(context-classifier)

#set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)

add_executable(test classifier audio vec rule ini_parser)

target_link_libraries(test asound)
target_link_libraries(test fftw3)

set(CMAKE_C_FLAGS "-g -Wall")