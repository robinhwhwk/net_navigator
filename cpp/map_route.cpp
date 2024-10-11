#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>     // For close
#include <chrono>

#define SEND_PORT 8080
#define RECV_PORT 9000


static char* getIpAddrByName(const char* hostname) {
    struct addrinfo hints, *res;
    int status;
    static char ipstr[INET6_ADDRSTRLEN];
    // Prepare hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets

    // Get address information
    if ((status = getaddrinfo(hostname, nullptr, &hints, &res))) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    }
    void *addr;
    if (res->ai_family == AF_INET) { //IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
        addr = &(ipv4->sin_addr);
    } else { // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*) res->ai_addr;
        addr = &(ipv6->sin6_addr);
    }
    // Convert the IP to a string
    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    freeaddrinfo(res);
    return ipstr;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./map_route <hostname>" << std::endl;
        return 1;
    }
    const char* hostname = argv[1];
    char buffer[1500]; // buffer to store response

    // Now res is a linked list of IP addresses for the hostname

    /*
    Steps:
    1. Gets the destination IP of the example destination, say “example.com”
    2. Creates a socket that allows it to send UDP packets
    3. Creates a socket on which it’ll listen for ICMP messages
    4. Initializes loop variables for keeping track of the current hop
    5. Inside a loop while we haven’t reached the destination:
        Sends a packet with an incrementing TTL
        Receives an incoming ICMP packet
        Keeps track of the current hop and the address of the received ICMP message
    */
   // convert hostname to IP address
    char* destination_ip;
    destination_ip = getIpAddrByName(hostname);
    // printf("Tracing route to: %s", destination_ip);
    std::cout << "Tracing route to: " << destination_ip << std::endl;
    // Create a socket that allows it to send UDP packets
    int send_sockfd;
    struct sockaddr_in sendaddr;
    socklen_t sendaddr_len = sizeof(sendaddr);
    if ((send_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "send_sockfd: " << send_sockfd << std::endl;
    // Convert the IP address from text to binary form
    if (inet_pton(AF_INET, destination_ip, &sendaddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(send_sockfd);
        exit(EXIT_FAILURE);
    }

    int option = 1;
    setsockopt(send_sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    memset(&sendaddr, 0, sizeof(sendaddr));
    sendaddr.sin_family = AF_INET;
    sendaddr.sin_port = htons(SEND_PORT);
    sendaddr.sin_addr.s_addr = inet_addr(destination_ip);

    // Create a socket that allows it to receive ICMP packets
    int recv_sockfd;
    struct sockaddr_in recvaddr;
    socklen_t recvaddr_len = sizeof(recvaddr);
    if ((recv_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&recvaddr, 0, sizeof(recvaddr));
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_port = htons(RECV_PORT);
    recvaddr.sin_addr.s_addr = INADDR_ANY;

    // Show ICMP Header, since that's where the router address is.
    setsockopt(recv_sockfd, IPPROTO_IP, IP_HDRINCL, &option, sizeof(option));

    char received_ip[INET6_ADDRSTRLEN];
    int current_hop = 1;
    const char* message = "message";
    while (strcmp(received_ip, destination_ip) != 0) {
        // Set socket TTL to current hop
        setsockopt(send_sockfd, IPPROTO_IP, IP_TTL, &current_hop, sizeof(current_hop));
        // Send the packet
        setsockopt(send_sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        int total_time_ms = 0;
        int latency_list[3];
        for (int i = 0; i < 3; ++i) {
            auto start_time = std::chrono::high_resolution_clock::now();
            if ((sendto(send_sockfd, message, strlen(message), 0, (struct sockaddr*)&sendaddr, sizeof(sendaddr))) < 0) {
                perror("sendto failed");
                close(send_sockfd);
                exit(EXIT_FAILURE);
            }
            // Receive any incoming ICMP packet.
            ssize_t bytes_received = recvfrom(recv_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&recvaddr, &recvaddr_len);
            if (bytes_received < 0) {
                perror("recvfrom failed");
                close(recv_sockfd);
                exit(EXIT_FAILURE);
            }
            auto end_time = std::chrono::high_resolution_clock::now();
            auto time = end_time - start_time;
            total_time_ms += time/std::chrono::milliseconds(1);

            latency_list[i] = time/std::chrono::milliseconds(1);
            
            strcpy(received_ip, inet_ntoa(recvaddr.sin_addr));
        }
        
        std::cout << "Current hop " << current_hop << ": ICMP message received from " << received_ip << " ";
        for (int j = 0; j < 3; j++) {
            std::cout << latency_list[j] << "ms ";
        }
        std::cout << ", Avg. latency: " << total_time_ms / 3 << "ms" << std::endl;
        current_hop += 1;
    }

}