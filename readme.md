# UDP Flood Sender

## Introduction

`udp_flood_sender` is a powerful tool designed to simulate Denial of Service (DoS) attacks specifically using the UDP protocol. This tool crafts and sends a large volume of spoofed UDP packets to a specified target IP address and port. It leverages raw sockets and multithreading to enhance attack intensity

The primary objective of this project is to provide a fundamental tool for researching how UDP Flood attacks operate (and how modern operating systems respond to them at the kernel level, but i was too lazy to do it :))))

**Disclaimer:** This tool is intended solely for research, educational, and experimental purposes within a controlled and isolated environment. Using this tool to attack any system without explicit authorization is illegal and unethical. The developer assumes no responsibility for any misuse.

## Principles of Operation

`udp_flood_sender` operates by constructing custom IP packets that encapsulate UDP data, bypassing the standard TCP/IP (or OSI) stack for these packets.

*   **Raw Sockets:** The program manually builds entire IP and UDP headers, allowing for customization of fields such as the source IP address (IP spoofing).
*   **Multithreading:** Multiple threads operate in parallel to create and send packets, increasing the rate and pressure on the target.
*   **Source IP Spoofing Modes**
    *   **Router Loopback Flood:** Spoofs the source IP address as the IP address of the Default Gateway (Router). This forces the Router to process response packets destined for itself, potentially overwhelming the Router's resources.
    *   **Else:** `192.168.1.1` - default
*   **Resource Optimization:** The program is designed to allocate memory for the packet once and only update dynamic fields (IP ID, checksums, payload) within the main loop, minimizing overhead on the attacking machine.

## System Requirements

*   Linux-based operating system (e.g., ParrotOS, Ubuntu).
*   GCC compiler.
*   `pthread` library.
*   `root` privileges to run the program (due to the use of raw sockets).

## Project Structure

```
udp-flood/
├── build/                 
├── include/               
│   ├── common.h           
│   ├── ip_utils.h         
│   ├── udp_utils.h        
│   └── utils.h            
└── src/                   
    ├── main.c             
    ├── ip_utils.c         
    ├── udp_utils.c        
    └── utils.c            
└── Makefile               
```

## Usage Guide

### 1. Environment Setup

*   **Clone Repository:**
    ```bash
    git clone https://github.com/NgTrongDoanh/udp-flood.git
    cd udp-flood
    ```

### 2. Building the Program

Open a terminal in the root directory of your project (`udp-flood/`) and run:

```bash
make
```
The executable will be created as `udp_flood_sender`.

### 3. Running the Program

The program requires `root` privileges to run.

**General Syntax:**
```bash
sudo ./build/udp_flood_sender -d <destination IP> [OPTIONS]
```

**Options:**

*   `-d <destination IP>`: Target IP address. (REQUIRED)
*   `-p <destination port>`: Target port for UDP flood (default: random)
*   `-m <mtu>`: MTU of the sending interface (default: 1500).
*   `-t <num_threads>`: Number of threads to use (default: 1, max 100).
*   `-h`: Display this usage guide.

**Examples:**

1.  **UDP Flood to `192.168.56.101` on port `8888` with 14 threads:**
    ```bash
    sudo ./build/udp_flood_sender -d 192.168.56.101 -p 8888 -t 14
    ```

2.  **UDP Flood to `192.168.56.101` on port random port with 10 threads, mtu = 1000:**
    ```bash
    sudo ./build/udp_flood_sender -d 192.168.56.101 -P 8888 -a 1 -n 14 -l 100
    ```


**To stop the attack, press `Ctrl+C` in the terminal where the program is running.**

## Cleanup

To remove compiled object files and the executable:

```bash
make clean
```
