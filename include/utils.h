#ifndef UTILS_H
#define UTILS_H

#include "common.h"

u_int16_t checksum(void *b, int len);
u_int32_t generate_random_ip_u32(); 
u_int32_t get_default_gateway_ip_u32();

int is_valid_ip(const char *ip_str);
int is_valid_port_num(int port);    
int is_valid_port_str(const char *port_str);

#endif // UTILS_H