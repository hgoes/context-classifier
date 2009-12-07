#ifndef __INI_PARSER_H__
#define __INI_PARSER_H__

#include <stdio.h>

typedef struct {
  int (*section_opened)(const char* name,void* data);
  int (*pair_read)(const char* key,const char* value,void* data);
} ini_callbacks_t;

int parse_ini_file(const ini_callbacks_t* cb,FILE* stream,void* data);

int read_doubles(const char* str,double* arr,int n);

#endif
