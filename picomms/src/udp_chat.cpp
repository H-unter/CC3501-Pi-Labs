#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <poll.h>
#include <iostream>

int main(int argc, char *argv[])
{
    char *input_function_name = argv[0];  // Points to the program name
    char *input_host_name = argv[1];      // Points to the host argument
    char *input_port_number = argv[2];    // Points to the port argument

    std::string client_username = "Hunter";

    bool is_all_arguments_present = argc == 3;
    if (!is_all_arguments_present) {
        printf("Usage: %s host port message\tTransmit a UDP packet to specified host and port containing the message\n", input_function_name); //arg0 is the program name, so there are 3 inputs and the function name
        return 1;
    }

    /*
    Use getaddrinfo to generate an address structure corresponding to the host
    to connect to.
    */
    struct addrinfo socket_info_hints; // structure is used to provide hints about the type of socket you are requesting
    struct addrinfo *address; // pointer to a linked list of one or more addrinfo structures
    memset(&socket_info_hints, 0, sizeof(struct addrinfo)); // initializes the structure to 0
    socket_info_hints.ai_family = AF_INET; // IPv4
    socket_info_hints.ai_socktype = SOCK_DGRAM; // UDP

    // Check that the address is resolved
    int address_info = getaddrinfo(input_host_name, input_port_number, &socket_info_hints, &address);
    bool is_address_resolved = address_info == 0;
    if (!is_address_resolved) {
        fprintf(stderr, "Failed to resolve address: %s\n", gai_strerror(address_info));
        return 1;
    }

    // Open the socket
    int my_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol); // create new socket
    bool is_socket_creation_successful = my_socket != -1;
    if (!is_socket_creation_successful) {
        perror("Failed to create socket");
        return 1;
    }

    // Set SO_BROADCAST to 1
    int broadcast_enable = 1;
    bool is_broadcast_enable_set = setsockopt(my_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) != -1;
    if (!is_broadcast_enable_set) {
        perror("Failed to set SO_BROADCAST");
        return 1;
    }

    // Set SO_REUSEPORT to 1
    int reuseport_enable = 1;
    bool is_reuseport_enable_set = setsockopt(my_socket, SOL_SOCKET, SO_REUSEPORT, &reuseport_enable, sizeof(reuseport_enable)) != -1;
    if (!is_reuseport_enable_set) {
        perror("Failed to set SO_REUSEPORT");
        return 1;
    }

    // Bind the socket to the address 
    if (bind(my_socket, address->ai_addr, address->ai_addrlen) == -1) {
        perror("Failed to bind socket");
        close(my_socket);
        return 1;
    }

    // Prepare a ‘pollfd’ array containing standard input (STDIN_FILENO) and the socket.
    struct pollfd pfds[] = {
        {
            // monitor the terminal for standard input
            .fd = STDIN_FILENO,
            .events = POLLIN,
        },
        {
            // Monitor the UDP socket
            .fd = my_socket,
            .events = POLLIN | POLLERR,
        },
    };

    int max_username_length = 32;
    int max_message_length = 240;

    int input_buffer_size = 1024;
    char input_buffer[input_buffer_size]; // make it extra big in case someone messes up lol

    int output_buffer_length = max_username_length + 1 + max_message_length + 1;
    char output_buffer[output_buffer_length]; // Username + null + message + null

    while (true){
        // Call poll() to wait for events
        int poll_result = poll(pfds, 2, -1); // -1 means wait indefinitely
        
        bool is_poll_successful = poll_result != -1;
        if (!is_poll_successful) {
            perror("poll failed");
            close(my_socket);
            return 1;
        }

        bool is_event_on_stdin = pfds[0].revents & POLLIN;
        bool is_event_on_socket = pfds[1].revents & POLLIN;

        if (is_event_on_stdin) { // send message from terminal input
            std::string output_message;
            bool is_read_error = !std::getline(std::cin, output_message);
            if (is_read_error){
                fprintf(stderr, "Failed to read input\n");
            }

            if (output_message.length() > max_message_length) {
                fprintf(stderr, "Message length exceeds the maximum allowed length of %d bytes.\n", max_message_length);
                return 1;
            }

            // Copy the username into the buffer
            strncpy(output_buffer, client_username.c_str(), max_username_length);
            output_buffer[client_username.length()] = '\0'; // Null terminator between username and message

            // Copy the message into the buffer, after the null terminator
            strncpy(output_buffer + client_username.length() + 1, output_message.c_str(), max_message_length);

            // Send the message
            ssize_t bytes_sent = sendto(my_socket, output_buffer, client_username.length() + 1 + output_message.length(), 0, address->ai_addr, address->ai_addrlen);
            bool is_message_sent = bytes_sent != -1;
            if (!is_message_sent) {
                perror("Failed to send");
                close(my_socket);
                freeaddrinfo(address);
                return 1;
            }
        }

        
        if (is_event_on_socket) { // recieve message from the socket
            size_t recieved_bytes = read(my_socket, input_buffer, sizeof(input_buffer) - 1); // with room for a trailing null
            bool is_message_received = recieved_bytes != -1;
            if (!is_message_received) {
                perror("Failed to receive");
                close(my_socket);
                freeaddrinfo(address);
                return 1;
            }
            
            size_t message_content_index = 0;
            
            // Iterate through the string until the null terminator is found
            // message: username'\0'message'\0'
            while (message_content_index < recieved_bytes && input_buffer[message_content_index] != '\0') { 
                message_content_index++;
            }

            size_t end_message_index = message_content_index + 1;
            while (end_message_index < recieved_bytes && input_buffer[end_message_index] != '\0') { 
                end_message_index++;
            }

            input_buffer[end_message_index] = '\0'; // Null terminate the message



            bool is_message_format_valid = message_content_index < input_buffer_size - 1;
            if (!is_message_format_valid) {
                fprintf(stderr, "Error: Malformed message received.\n");
                close(my_socket);
                freeaddrinfo(address);
                return 1;
            }


            char* message_username = input_buffer; // username is the first part of the message
            char* message_content = input_buffer + message_content_index + 1; // message is the second part of the message, after a \0

            // Print the message
            printf("[%s] %s\n", message_username, message_content);
        }
    }

     // Clean up
    close(my_socket);
    freeaddrinfo(address);
    return 0;

}
