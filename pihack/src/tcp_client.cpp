#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

//call it like ./client host port msg

int main(int argc, char *argv[])
{
    if (argc < 4) {
        printf("Usage: %s host port message\tConnect to host, port and send message\n", argv[0]); //arg0 is the program name
        return 1;
    }

    /*
    Use getaddrinfo to generate an address structure corresponding to the host
    to connect to.
    */
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    int s = getaddrinfo(argv[1], argv[2], &hints, &result); // arg1 is the host name and arg2 is the port number
    if (s != 0) {
        fprintf(stderr, "Failed to resolve address: %s\n", gai_strerror(s));
        return 1;
    }

    // Try to open a socket using each of the addresses returned by getaddrinfo in sequence
    int sock;
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        void *addr;
        const char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (rp->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it
        char ipstr[INET6_ADDRSTRLEN];
        inet_ntop(rp->ai_family, addr, ipstr, sizeof ipstr);
        printf("Connecting to %s address: %s\n", ipver, ipstr);

        /*
        Create the socket using the address family and socket type returned by
        getaddrinfo.
        */
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) {
            // Failed. Try the next address in the sequence
            perror("Failed to create socket");
            continue;
        }

        /*
        Connect to the server
        */
        if (-1 != connect(sock, rp->ai_addr, rp->ai_addrlen)) {
            // Success.
            break;
        }

        // Failed. Try the next address in the sequence.
        perror("Failed to connect");
        close(sock);
    }

    if (rp == NULL) {
        // None of the addresses returned by getaddrinfo worked.
        fprintf(stderr, "Failed to connect.\n");
        return 1;
    }

    // Free the memory returned by getaddrinfo
    freeaddrinfo(result);

    // Send the message
    if (-1 == (s = send(sock, argv[3], strlen(argv[3]), 0))) { //arg3 is the data to be sent
        perror("Failed to send data");
    }
    printf("Sent %i bytes\n", s);


    // Receive a message
    printf("Receiving:\n");
    char buf[101];
    int num_bytes;
    // Continue receiving while there is data to receive
    while ((num_bytes = recv(sock, buf, 100, 0)) > 0) {
        buf[num_bytes] = 0;
        printf("%s", buf);
    };

    // Close the socket
    close(sock);
}
