#include "ip_utils.h"

u_int16_t ip_checksum(struct ip_header *hdr) {
    hdr->header_checksum = 0; 
    return checksum((u_int16_t*)hdr, hdr->ihl * 4);
}