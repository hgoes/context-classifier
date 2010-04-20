#include "broadcaster.h"

int create_broadcast_socket() {
  int sock;
  const int broadcast = 1;
  struct sockaddr_in bind_addr;
  if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    return -1;
  }
  if((setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof broadcast)) < 0) {
    return -1;
  }

  memset(&bind_addr,0,sizeof(bind_addr));
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind_addr.sin_port = htons(PORT);
  
  if(bind(sock,(struct sockaddr*)&bind_addr,sizeof(bind_addr)) < 0) {
    return -1;
  }

  return sock;
}

int send_broadcast_packet(int sock,
                          uint32_t tstamp_sec,
                          uint32_t tstamp_usec,
                          const char* act,
                          uint8_t rel,
                          uint8_t inter_rel) {
  
  char buf[sizeof(tstamp_sec)+sizeof(tstamp_usec)+3+sizeof(rel)+sizeof(inter_rel)];
  
  memcpy(buf,&tstamp_sec,sizeof(tstamp_sec));
  memcpy(&buf[sizeof(tstamp_sec)],&tstamp_usec,sizeof(tstamp_usec));
  memcpy(&buf[sizeof(tstamp_sec)+sizeof(tstamp_usec)],act,3);
  memcpy(&buf[sizeof(tstamp_sec)+sizeof(tstamp_usec)+3],&rel,sizeof(rel));
  memcpy(&buf[sizeof(tstamp_sec)+sizeof(tstamp_usec)+3+sizeof(rel)],&inter_rel,sizeof(inter_rel));
  
  struct sockaddr_in recv;
  recv.sin_family = AF_INET;
  recv.sin_port = htons(PORT);
  recv.sin_addr.s_addr = INADDR_BROADCAST;

  sendto(sock,buf,sizeof(tstamp_sec)+sizeof(tstamp_usec)+3+sizeof(rel)+sizeof(inter_rel),0,(struct sockaddr*)&recv,sizeof(recv));

  return 0;
}
