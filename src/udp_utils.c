#include "udp_utils.h"

u_int16_t udp_checksum(u_int32_t source_address, u_int32_t dest_address, 
                       const struct udphdr *udp_hdr_ptr, 
                       const u_int8_t *payload_ptr, 
                       int payload_len) {
    struct pseudo_header psh = {
        source_address,
        dest_address, 
        0, IPPROTO_UDP, // zero, protocol (UDP = 17)
        htons(sizeof(struct udphdr) + payload_len) 
    };
    
    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + payload_len;
    char *pseudogram = malloc(psize);
    if (!pseudogram) {
        perror("malloc for pseudogram failed in udp_checksum");
        return 0; 
    }

    memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), (char*)udp_hdr_ptr, sizeof(struct udphdr));
    memcpy(pseudogram + sizeof(struct pseudo_header) + sizeof(struct udphdr), (char*)payload_ptr, payload_len);

    u_int16_t result = checksum((u_int16_t*)pseudogram, psize);
    free(pseudogram);

    return result;
}