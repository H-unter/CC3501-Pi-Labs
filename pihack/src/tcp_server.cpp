#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

// call it like ./server port 

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s port\tListen on the specified port\n", argv[0]);
        return 1;
    }

    /*
    Use getaddrinfo to generate an address structure corresponding to
    a "wildcard" address. That means the server will accept connections
    from anywhere.
    */
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // Return a "wildcard" that will listen for connections from any address

    int s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "Failed to resolve address: %s\n", gai_strerror(s));
        return 1;
    }

    // Try to open a socket using each of the addresses returned by getaddrinfo in sequence
    int sock;
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        /*
        Create the socket using the address family and socket type returned by
        getaddrinfo.
        */
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) {
            // Failed. Try the next address in the sequence
            continue;
        }

        /*
        Allow bind to succeed even if old connections are in the TIME_WAIT state
        (i.e. waiting to close).
        */
        int optval = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        /*
        Bind to the address returned by getaddrinfo.
        This could be an IPv4 or an IPv6 address.
        */
        if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
            // Success.
            break;
        }

        // If we failed to bind the socket, close it.
        close(sock);
    }

    if (rp == NULL) {
        // None of the addresses returned by getaddrinfo worked.
        perror("Failed to bind socket");
        return 1;
    }

    // Free the memory returned by getaddrinfo
    freeaddrinfo(result);

    /*
    Listen for incoming connections. The second argument is the number of
    unhandled connections to enqueue.
    */
    if (-1 == listen(sock, 5)) {
        perror("Failed to listen");
        return 1;
    }

    for (;;) {
        /*
        Accept a connection from a client.
        The second and third arguments can be used to receive the address of the
        client that connected.
        */
        int conn = accept(sock, NULL, NULL);
        if (conn == -1) {
            perror("Failed to accept a connection");
            return 1;
        }

        /*
        Spawn a new child process to handle this connection.
        */
        if (fork() == 0) {
            // We are the child.
            printf("Connection established.\n");

            /*
            Receive data
            */
            char buf[101];
            int num_bytes = recv(conn, buf, 100, 0);
            if (num_bytes == 0) {
                printf("\nRemote side disconnected\n");
                break;
            } else if (num_bytes == -1) {
                perror("\nFailed to receive data");
            }
            buf[num_bytes] = 0;
            printf("Received: %s\n", buf);

            /*
            Echo it back
            */
            int s;
            if (-1 == (s = send(conn, buf, num_bytes, 0))) {
                perror("Failed to send data");
            }

            /*
            Close the connection
            */
            printf("Closing the connection.\n");
            close(conn);
        } else {
            // We are the parent.
            // Close the socket because it's being handled in the child process.
            close(conn);
        }
    }
}
