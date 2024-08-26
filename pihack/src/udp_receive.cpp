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
#include <poll.h>

//call it like ./client host port msg

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s port\tListen on the specified port\n", argv[0]);
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
    hints.ai_flags = AI_PASSIVE; // interpret a NULL hostname as a wildcard (to accept data from anywhere)
    int s = getaddrinfo(NULL, argv[1], &hints, &address); 
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

    // Bind it to the address and port 
    if (0 != bind(socket_fd, address->ai_addr, address->ai_addrlen)) {
        perror("Failed to bind");
        return 1;
    }

    // Prepare the pollfd array with the list of file handles to monitor
    struct pollfd pfds [] = {
        {
            // monitor the socket
            .fd = socket_fd,
            .events = POLLIN | POLLERR,
        },
        // add here if there are other files/sockets to monitor
    };

    // Event loop
    static char buf [512];
    for (;;) {
        // Wait for events
        poll(pfds, sizeof(pfds)/sizeof(struct pollfd), -1);

        // Check if a packet arrived
        if (pfds[0].revents) {
            // Read the incoming packet
            ssize_t bytes_read = read(socket_fd, buf, sizeof(buf) - 1); // with room for a trailing null
            if (bytes_read < 0) {
                return 0;
            }
            // Make the message null terminated
            buf[bytes_read] = 0;

            // Print it out
            printf("Received: %s\n", buf);
        }
    }
    
    // Free the memory returned by getaddrinfo
    freeaddrinfo(address);

    // Close the socket
    close(socket_fd);
}
