#ifndef IP_UTILS_H
#define IP_UTILS_H

#include "common.h" 
#include "utils.h" 

struct ip_header {
    #if __BYTE_ORDER == __LITTLE_ENDIAN
        u_int8_t ihl:4;
        u_int8_t version:4;
    #elif __BYTE_ORDER == __BIG_ENDIAN
        u_int8_t version:4;
        u_int8_t ihl:4;
    #endif
    u_int8_t tos;
    u_int16_t total_length;
    u_int16_t identification;
    u_int16_t frag_off; // 3 bits flags + 13 bits offset
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t header_checksum;
    u_int32_t saddr;
    u_int32_t daddr;

    u_int8_t options[]; // optional
};

struct ip_packet {
    struct ip_header header;
    u_int8_t payload[];
};

u_int16_t ip_checksum(struct ip_header *hdr);

#endif // IP_UTILS_H