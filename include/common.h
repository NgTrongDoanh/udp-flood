#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define MTU_DEFAULT 1500
#define MAX_IP_HEADER_SIZE 60
#define UDP_HEADER_SIZE 8    
#define MAX_HEADER_SIZE (MAX_IP_HEADER_SIZE + UDP_HEADER_SIZE) 
#define IP_MF 0x2000 
#define IP_DF 0x4000 

extern volatile long long total_packets_sent;
extern pthread_mutex_t packet_count_mutex;
extern volatile sig_atomic_t keep_running;

void sigint_handler(int signo);

#endif // COMMON_H