#include "ini_parser.h"

#define INI_STRING_MAX 4096

int parse_ini_file(const ini_callbacks_t* cb,FILE* stream,void* data) {
  int ch,i,j,res;
  char buf[INI_STRING_MAX];
  while((ch = fgetc(stream)) != EOF) {
    switch(ch) {
    case '[': // A section opening
      i = 0;
      // Read the name
      while(1) {
	ch = fgetc(stream);
	if(ch == EOF) return -1;
	else if(ch == ']') {
	  buf[i] = '\0';
	  res = cb->section_opened(buf,data);
	  if(res != 0) {
	    return res;
	  }
	  break;
	} else if(ch == '\n') {
	  return -1;
	} else {
	  buf[i] = (char)ch;
	}
	i++;
	if(i == INI_STRING_MAX) {
	  return -2;
	}
      }
      // Skip trailing whitespace
      while(1) {
	ch = fgetc(stream);
	if(ch == EOF) return 0;
	else if(ch == ' ' || ch == '\t') {
	  // Skip it
	} else if(ch == '\n') {
	  break;
	} else {
	  // Illegal char
	  return -1;
	}
      }
      break;
    case '\n': // An empty line, skip it
    case ' ': // A leading whitespace, skip it
      break;
    case ';': // A comment, skip till line break
      while(1) {
	ch = fgetc(stream);
	if(ch == EOF) {
	  return 0;
	} else if(ch == '\n') {
	  break;
	}
      }
      break;
    default:
      // Read the key
      i = 1;
      buf[0] = (char)ch;
      while(1) {
	ch = fgetc(stream);
	if(ch == EOF || ch == '\n') {
	  return -1;
	} else if(ch == ' ' || ch == '\t') { // Trailing whitespace
	  buf[i] = '\0';
	  while(1) {
	    ch = fgetc(stream);
	    if(ch == EOF) {
	      return -1;
	    } else if(ch == ' ' || ch == '\t') {
	      // Skip whitespace
	    } else if(ch == '=') {
	      break;
	    } else { // other garbage
	      return -1;
	    }
	  }
	  break;
	} else if(ch == '=') {
	  buf[i] = '\0';
	  break;
	}
	if(i == INI_STRING_MAX) {
	  return -2;
	}
	buf[i] = (char)ch;
	i++;
	if(i >= INI_STRING_MAX) {
	  return -2;
	}
      }
      // The key is read, now read the value
      i++;
      if(i >= INI_STRING_MAX) {
	return -2;
      }
      j=i;
      while(1) {
	ch = fgetc(stream);
	if(ch == EOF) {
	  buf[i] = '\0';
	  res = cb->pair_read(buf,&buf[j],data);
	  if(res != 0) {
	    return res;
	  }
	  return 0;
	} else if(ch == '\n') {
	  ch = fgetc(stream);
	  ungetc(ch,stream);
	  if(ch != ' ') {
	    buf[i] = '\0';
	    res = cb->pair_read(buf,&buf[j],data);
	    if(res != 0) {
	      return res;
	    }
	    break;
	  } else {
	    buf[i] = '\n';
	  }
	} else {
	  buf[i] = (char)ch;
	}
	i++;
	if(i >= INI_STRING_MAX) {
	  return -2;
	}
      }
      break;
    }
  }
  return 0;
}

int read_doubles(const char* str,double* arr,int n) {
  int pos;
  int i;
  for(i=0;i<n;i++) {
    if(sscanf(str," %lf%n",&(arr[i]),&pos) != 1) {
      printf("FAIL %s\n",str);
      return -1;
    }
    str+=pos;
  }
  return 0;
}
