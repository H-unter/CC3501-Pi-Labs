#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/udp.h>

//call it like ./client host port msg

int main(int argc, char *argv[])
{
    if (argc < 4) {
        printf("Usage: %s host port message\tTransmit a UDP packet to specified host and port containing the message\n", argv[0]); //arg0 is the program name
        return 1;
    }

    /*
    Use getaddrinfo to generate an address structure corresponding to the host
    to connect to.
    */
    struct addrinfo hints;
    struct addrinfo *address;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    int s = getaddrinfo(argv[1], argv[2], &hints, &address); // arg1 is the host name and arg2 is the port number
    if (s != 0) {
        fprintf(stderr, "Failed to resolve address: %s\n", gai_strerror(s));
        return 1;
    }

    // Open the socket
    int socket_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (socket_fd == -1) {
        // Failed.
        perror("Failed to create socket");
        return 1;
    }

    // Allow multiple applications to use the same port (to run two versions of the app side by side for testing)
    int optval = true;
    if (0 != setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))) {
        perror("Failed to set SO_REUSEPORT");
        return 1;
    }

    // Send the message 
    char *buffer_to_send = argv[3]; // Use the third command line argument
    s = sendto(socket_fd, buffer_to_send, strlen(buffer_to_send), 0, address->ai_addr, address->ai_addrlen);
    if (s == -1) {
        perror("Failed to send.");
        return 1;
    }
    printf("Sent %i bytes\n", s);

    // Free the memory returned by getaddrinfo
    freeaddrinfo(address);

    // Close the socket
    close(socket_fd);
}
