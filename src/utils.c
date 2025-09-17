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
    FILE *fp = fopen("/proc/net/route", "r");
    if (!fp) return 0;

    char line[256];
    char iface[64];
    unsigned long dest, gateway;
    
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%63s %lx %lx", iface, &dest, &gateway) == 3) {
            if (dest == 0) { 
                struct in_addr gw_addr;
                gw_addr.s_addr = gateway;
                fclose(fp);

                char *ip = malloc(INET_ADDRSTRLEN);
                if (ip) {
                    strcpy(ip, inet_ntoa(gw_addr));
                }
                u_int32_t ip_u32 = inet_addr(ip);
                free(ip);
                return ip_u32;
            }
        }
    }

    fclose(fp);
    return 0;
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