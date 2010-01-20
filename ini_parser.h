/**
 * @file
 * A minimalistic ini parser.
 */
#ifndef __INI_PARSER_H__
#define __INI_PARSER_H__

#include <stdio.h>

/**
 * Callback structure for the ini-parser
 */
typedef struct {
  /**
   * Called when a new section is opened
   * @param name The name of the section
   * @param data The user defined data pointer passed to the parse-call
   */
  int (*section_opened)(const char* name,void* data);
  /**
   * Called when a key-value pair is encountered
   * @param key The name of the key
   * @param value The value string
   * @param data The user defined data pointer passed to the parse-call
   */
  int (*pair_read)(const char* key,const char* value,void* data);
} ini_callbacks_t;

/**
 * Parse an ini file from a file stream.
 * @param cb The callbacks to use when encountering entities
 */
int parse_ini_file(const ini_callbacks_t* cb,FILE* stream,void* data);

/**
 * Helper function to parse a list of doubles from a string.
 * @param str The string to parse from
 * @param arr The array to store the read doubles in
 * @param n The number of elements to read
 * @return 0 if the required number of doubles could be read, <0 otherwise
 */
int read_doubles(const char* str,double* arr,int n);

#endif
