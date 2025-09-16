#include "utils.h"

u_int16_t checksum(void *b, int len) {    
    u_int16_t *buf = b; 
    u_int32_t sum = 0; 
    u_int16_t result;

    for (sum = 0; len > 1; len -= 2) sum += *buf++; 
    if (len == 1) sum += *(u_int8_t*)buf; 
    
    sum = (sum >> 16) + (sum & 0xFFFF); 
    sum += (sum >> 16);

    result = ~sum; 
    return result; 
}

u_int32_t generate_random_ip_u32() {
    return htonl( (u_int32_t)rand() % 256 << 24 | 
                  (u_int32_t)rand() % 256 << 16 | 
                  (u_int32_t)rand() % 256 << 8  | 
                  (u_int32_t)rand() % 256 );
}

u_int32_t get_default_gateway_ip_u32() {
    FILE *fp;
    char line[256];
    char iface[16];
    u_int32_t dest_ip_hex, gateway_ip_hex;
    int flags;
    u_int32_t default_gateway_net_order = 0;

    fp = fopen("/proc/net/route", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/net/route");
        return 0;
    }

    fgets(line, sizeof(line), fp); 

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "%s %X %X %X", iface, &dest_ip_hex, &gateway_ip_hex, &flags) == 4) {
            if (dest_ip_hex == 0x0 && (flags & 0x0003) == 0x0003) {
                default_gateway_net_order = htonl(gateway_ip_hex); 
                default_gateway_net_order = ( (gateway_ip_hex & 0x000000FF) << 24 ) |
                                            ( (gateway_ip_hex & 0x0000FF00) << 8  ) |
                                            ( (gateway_ip_hex & 0x00FF0000) >> 8  ) |
                                            ( (gateway_ip_hex & 0xFF000000) >> 24 );

                break;
            }
        }
    }

    fclose(fp);
    return default_gateway_net_order;
}

int is_valid_ip(const char *ip_str) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_str, &(sa.sin_addr));
    return result != 0;
}

int is_valid_port_num(int port) { 
    return port > 0 && port <= 65535;
}

int is_valid_port_str(const char *port_str) {
    char *endptr;
    long port = strtol(port_str, &endptr, 10);
    if (*endptr != '\0' || endptr == port_str) {
        return 0; 
    }
    return is_valid_port_num((int)port);
}