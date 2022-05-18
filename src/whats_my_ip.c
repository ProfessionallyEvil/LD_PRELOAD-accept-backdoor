/***********************************************************************
*
* Project: whats_my_ip
*
* Author: Travis Phillips
*
* Date: 05/18/2022
*
* Project Repo:
*  https://github.com/ProfessionallyEvil/LD_PRELOAD-accept-backdoor
*
* Purpose: This is a simple target application for the accept_backdoor
*          LD_PRELOAD example.  This is a simple daemon that will accept
*          connections on port 1337 and respond to the client with the
*          IP and port they connected from and then disconnect.
*
* Compile: gcc whats_my_ip.c -o whats_my_ip
*
***********************************************************************/
#include <stdio.h>        // Standard include
#include <sys/types.h>    // Socket Stuff
#include <sys/socket.h>   // Socket Stuff
#include <netinet/in.h>   // Socket Stuff
#include <netdb.h>        // Socket Stuff
#include <arpa/inet.h>    // Socket Stuff
#include <signal.h>       // signal() - to catch ctrl+C and close clean
#include <string.h>       // strlen()
#include <unistd.h>       // close(), write()
#include <errno.h>        // error()
#include <stdlib.h>       // exit()


// Global for use of storing socket descriptors so signal can
// close them on Ctrl+C
int sock_fd = 0;
int client_sock_fd = 0;
static const int PORT = 1337;

// Signal handler function.
void shutdown_server(int signal_num) {
    if (signal_num == SIGINT) {
        printf("\n [*]Caught Ctrl+C, Shutting down the server...\n");
        // Attempt to close sockets.
        if (client_sock_fd != 0)
            close(client_sock_fd);
        if (sock_fd != 0)
            close(sock_fd);
        exit(0);
    }
}

int main(int argc, char **argv) {
    struct sockaddr_in addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    //char *client_ip = NULL;
    //uint16_t client_port = NULL;
    char buffer[64];
    
    // Create a signal handler for Ctrl+C
    signal(SIGINT, shutdown_server);
    
    // Print a banner
    printf("\n\t---===[ What's my IP Service ]===---\n\n");
    
    // Create a socket.
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!sock_fd){
        perror("ERROR opening socket");
        return 1;
    }

    // Zero out the sockaddr_in struct memeory.
    memset(&addr, 0, sizeof(addr));
    
    // Populate the addr struct.
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    // Bind and listen the socket.
    if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("ERROR on binding");
        return 1;
    }
    listen(sock_fd, 1);
    printf(" [*] Server started on 0.0.0.0:%d\n", PORT);

    // Start the infinite accept loop.
    while (1) {
        // housekeeping
        memset(&cli_addr, 0, sizeof(cli_addr));
        memset(&buffer, 0, 64);
        client_sock_fd = 0;
        
        // Run accept() to get a connection.
        client_sock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &cli_len);
        
        // Check that the client socket is good.
        if (client_sock_fd > 1) {

            // Report in the console we got a connection.
            printf(" [*] Client Connected: %s:%d\n", 
                   inet_ntoa(cli_addr.sin_addr),
                   ntohs(cli_addr.sin_port));
            
            // Extract the IP address and Port and send it to the client.
            snprintf(buffer, 63,
                     "\n [*] IP Address: %s\n [*] Source Port: %d\n", 
                     inet_ntoa(cli_addr.sin_addr),
                     ntohs(cli_addr.sin_port));
            
            // Send that information to the client.
            write(client_sock_fd, buffer, strlen(buffer));
            
            // Close the client socket.
            close(client_sock_fd);
        }
    }
    
    return 0;
}
