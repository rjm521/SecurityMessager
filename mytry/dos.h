//  ddos.h


#ifndef dos_h
#define dos_h

#include "message.h"
#include "socket.h"
#include "util.h"

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MODE_EMPTY 0 //not initiallized mode
#define MODE_TCP 1

/*extern是计算机语言中的一个关键字，可置于变量或者函数前，以表示变量或者函数的定义在别的文件中。*/
extern double psent;
extern uint8_t metrics;
extern bool status;//is status shown?
extern int tcount;
extern char* __host;
extern int __port;
extern pthread_mutex_t mutex;
extern bool use_dos_sleep;
extern int dos_sleep;
extern bool __run;

typedef struct {
    char* packet;
    char* host;
    int port;
    uint8_t mode;
} _dos_param;

void dos(char* host, int port, char* packet, int _tcount, int mode);

#endif /* dos_h */
