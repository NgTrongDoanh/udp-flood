#ifndef UDP_UTILS_H
#define UDP_UTILS_H

#include "common.h" 
#include "utils.h"  

struct udp_header {
    u_int16_t source_port;
    u_int16_t dest_port;
    u_int16_t length;
    u_int16_t checksum;
};

struct udp_datagram {
    struct udp_header header;
    u_int8_t payload[];
};

struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t zero;
    u_int8_t protocol;
    u_int16_t udp_length;
};

u_int16_t udp_checksum(u_int32_t source_address, u_int32_t dest_address, 
                       const struct udphdr *udp_hdr_ptr, 
                       const u_int8_t *payload_ptr,      
                       int payload_len);                 
#endif // UDP_UTILS_H