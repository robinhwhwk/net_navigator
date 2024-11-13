#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <hostname>" << std::endl;
        return 1;
    }

    const char* hostname = argv[1];
    struct addrinfo hints, *res;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    // Prepare hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    // Get address information
    if ((status = getaddrinfo(hostname, nullptr, &hints, &res)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        return 2;
    }

    // Loop through all the results and get the first one
    for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
        void *addr;
        std::string ipver;

        // Get the pointer to the address itself,
        // different fields in IPv4 and IPv6
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // Convert the IP to a string and print it
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        std::cout << ipver << ": " << ipstr << std::endl;
    }

    freeaddrinfo(res); // Free the linked list

    return 0;
}
