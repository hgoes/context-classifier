#ifndef __BROADCASTER_H__
#define __BROADCASTER_H__

#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdint.h>

#define PORT 7343

int create_broadcast_socket();

int send_broadcast_packet(int sock,
                          uint32_t tstamp_sec,
                          uint32_t tstamp_usec,
                          const char* act,
                          uint8_t rel,
                          uint8_t inter_rel);

#endif
