#include "classifier.h"

void usage() {
  printf("Usage: classifier [-hcb] [-f FORMAT] [--help] [--cpu-runtime] [-udp-broadcast] [--format FORMAT] (PLUGIN CLASSIFIER)*\n");
  printf("\n");
  printf(" -h,--help\t\tShow this help\n");
  printf(" -c,--cpu-runtime\tPrint runtime statistics upon exit\n");
  printf(" -b,--udp-broadcast\tBroadcast classification results via UDP\n");
  printf(" -f,--format\tOutput the classification result using a format string\n");
  printf("\n");
  printf("FORMAT is a string that can contain the following letters:\n");
  printf(" 's' The current time in seconds\n");
  printf(" 'u' The microseconds of the current time\n");
  printf(" 'c' The classification result (a string)\n");
  printf(" 'r' The raw value of the classification\n");
  printf(" 'g' The expected classification result (ground truth)\n");
  printf(" 'b' The current battery level (in percent)\n");
  printf("\n");
  printf("PLUGIN is a string seperated by ':' describing a plugin and its options. Available plugins:\n");
  printf("  acceleration\t\t\tThe acceleration sensors of the OpenMoko Freerunner phone\n");
  printf("  replay:acceleration:FILE\tReplay acceleration sensor data from FILE\n");
  printf("\n");
  printf("CLASSIFIER is a json file describing a classifier.\n");
}

plugin_t* load_plugin(char* options) {
  if(strncmp("acceleration",options,12) == 0) {
    return get_acceleration_plugin();
  }
  if(strncmp("replay",options,6) == 0) {
    switch(options[6]) {
    case '\0':
      fprintf(stderr,"replay plugin requires two parameters (none given)\n");
      return NULL;
    case ':':
      if(strncmp("acceleration",&options[7],12) == 0) {
        switch(options[19]) {
        case '\0':
          fprintf(stderr,"replay plugin requires two parameters (one given)\n");
          return NULL;
        case ':':
          return get_acceleration_replayer_plugin(&options[20]);
        }
      }
      break;
    }
  }
  fprintf(stderr,"Failed to load plugin for string %s\n",options);
  return NULL;
}

int check_format_string(const char* fmt) {
  int i;
  for(i=0;fmt[i]!='\0';i++) {
    switch(fmt[i]) {
    case 's':
    case 'u':
    case 'c':
    case 'r':
    case 'b':
    case 'g':
      break;
    default:
      fprintf(stderr,"Invalid letter in format string: '%c'\n",fmt[i]);
      return -1;
    }
  }
  return 0;
}

int parse_options(int argc,char** argv,classifier_options* opts) {
  static struct option long_options[] = {
    { "cpu-runtime",0,NULL,0 },
    { "udp-broadcast",0,NULL,0 },
    { "format",required_argument,NULL,0 },
    { "help",0,NULL,0 },
    { NULL,0,NULL,0 }
  };
  int option_index;
  int c;

  //Set defaults
  opts->cpu_runtime = 0;
  opts->udp_broadcast = 0;
  opts->show_help = 0;
  opts->format_string = "suc";

  while( (c = getopt_long(argc,argv,"chbf:",long_options,&option_index)) != -1) {
    switch(c) {
    case 0:
      switch(option_index) {
      case 0:
        opts->cpu_runtime = 1;
        break;
      case 1:
        opts->udp_broadcast = 1;
        break;
      case 2:
        opts->format_string = optarg;
        break;
      case 3:
        opts->show_help = 1;
        break;
      }
      break;
    case 'c':
      opts->cpu_runtime = 1;
      break;
    case 'h':
      opts->show_help = 1;
      break;
    case 'b':
      opts->udp_broadcast = 1;
      break;
    case 'f':
      opts->format_string = optarg;
      break;
    default:
      return -1;
    }
  }
  opts->length = (argc - optind) / 2;
  if(opts->length > 0) {
    opts->plugins = calloc(opts->length,sizeof(char*));
    opts->classifiers = calloc(opts->length,sizeof(char*));
    for(c = 0; c<opts->length; c++) {
      opts->plugins[c] = strdup(argv[optind+c*2]);
      opts->classifiers[c] = strdup(argv[optind+c*2+1]);
    }
  }
  return 0;
}

int main(int argc,char** argv) {
  classifier_options opts;
  if(parse_options(argc,argv,&opts) != 0) {
    usage();
    exit(-1);
  }
  if(check_format_string(opts.format_string) != 0) {
    usage();
    exit(-1);
  }
  if(opts.show_help) {
    usage();
    exit(0);
  }
  int running = 1;
  struct tms time_start,time_end;
  struct timespec real_time_start,real_time_end;
  int sock;
  if(opts.cpu_runtime) {
    times(&time_start);
    clock_gettime(CLOCK_MONOTONIC,&real_time_start);
  }


  void exit_handler(int signal) {
    running = 0;
  }

  signal(SIGINT,exit_handler);

  if(opts.udp_broadcast) {
    sock = create_broadcast_socket();
  }

  void callback(char* class,double raw,char* ground_truth,void* user_data) {
    int i;
    struct timeval tv;
    gettimeofday(&tv,NULL);
    for(i=0;opts.format_string[i]!='\0';i++) {
      if(i>0) printf("\t");
      switch(opts.format_string[i]) {
      case 's':
        printf("%ld",tv.tv_sec);
        break;
      case 'u':
        printf("%ld",tv.tv_usec);
        break;
      case 'c':
        printf("%s",class);
        break;
      case 'r':
        printf("%f",raw);
        break;
      case 'b':
        printf("%d",read_battery_level());
        break;
      case 'g':
        if(ground_truth == NULL) {
          printf("NONE");
        } else {
          printf("%s",ground_truth);
        }
        break;
      }
    }
    if(i>0) {
      printf("\n");
    }
    if(opts.udp_broadcast) {
      send_broadcast_packet(sock,tv.tv_sec,tv.tv_usec,class,(int)(raw*255.0),9);
    }
  }

  int i;
  for(i=0;i<opts.length;i++) {
    plugin_t* plugin = load_plugin(opts.plugins[i]);
    classifier_set_t cls;
    if(parse_classifier_set(opts.classifiers[i],&cls,plugin) != 0) {
      fprintf(stderr,"Failed to parse classifier %s\n",opts.classifiers[i]);
      exit(-1);
    }
    
  }

  pid_t* pids = calloc(opts.length,sizeof(pid_t));
  for(i=0;i<opts.length;i++) {
    plugin_t* plugin = load_plugin(opts.plugins[i]);
    classifier_set_t cls;
    if(parse_classifier_set(opts.classifiers[i],&cls,plugin) != 0) {
      fprintf(stderr,"Failed to parse classifier %s\n",opts.classifiers[i]);
      exit(-1);
    }
    pids[i] = dispatch_plugin(plugin,cls.rules,callback,NULL,&running,cls.semantics);
  }

  for(i=0;i<opts.length;i++) {
    waitpid(pids[i],NULL,0);
  }

  if(opts.cpu_runtime) {
    times(&time_end);
    clock_gettime(CLOCK_MONOTONIC,&real_time_end);
    double rtime = 0;
    rtime += time_end.tms_utime - time_start.tms_utime;
    rtime += time_end.tms_stime - time_start.tms_stime;
    rtime += time_end.tms_cutime - time_start.tms_cutime;
    rtime += time_end.tms_cstime - time_start.tms_cstime;
    rtime /= sysconf(_SC_CLK_TCK);
    printf("%f seconds in CPU time\n",rtime);
    double rtime2 = 0;
    rtime2 += real_time_end.tv_sec - real_time_start.tv_sec;
    rtime2 += ((double)(real_time_end.tv_nsec - real_time_start.tv_nsec)) / 1000000000.0;
    printf("%f seconds in real time\n",rtime2);
    printf("%f%% load\n",100.0*rtime/rtime2);
  }
  return 0;
}
