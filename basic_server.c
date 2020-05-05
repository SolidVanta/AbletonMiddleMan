#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define MY_PORT "3490"
#define BACKLOG 10
#define MAX_SIZE 28

struct addrinfo settings;
struct addrinfo *res, *p; // will point to the result
struct sockaddr_storage client_addr;
int sockfd, newfd, yes=1, status, fdmax;
socklen_t addrlen;
char hostname[128];
char str[MAX_SIZE];
char msg[MAX_SIZE];
size_t bytes_rec;
fd_set read_fds, master; //file descriptor list
char remoteIP[INET6_ADDRSTRLEN];


void setup(struct addrinfo *settings)
{
  memset(settings, 0, sizeof settings); // make sure the struct is empty
  settings->ai_family = AF_UNSPEC; // don't care if IPv4 or IPv6
  settings->ai_flags = AI_PASSIVE; // use my IP address
  settings->ai_socktype = SOCK_STREAM; // use TCP stream sockets
}

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
  {
    if(sa->sa_family == AF_INET) {
      return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }
                                                                                                                                                                   
int main()
{
  setup(&settings);
  if ((status = getaddrinfo(NULL, "3490", &settings, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }

  FD_ZERO(&master);
  for (p = res; p != NULL; p = p->ai_next) {
    if ( ( sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) ) == -1){
      perror("server:socket");
      continue;
    }
      /* Set this for re-use of port*/
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
      perror("setsockopt");
      exit(1);
      }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      fprintf(stderr, "Could not bind to %s", MY_PORT);
      continue;
    }
      break;
  }
  if (p == NULL) {
    fprintf(stderr, "selectserver: failed to bind\n");
    exit(2);
  }
  
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(3);
  }

  FD_SET(sockfd, &master);
  fdmax = sockfd;
  freeaddrinfo(res);
  //Refactor into method.
  struct sockaddr_in *sa = (struct sockaddr_in *)res->ai_addr;
  printf("PORT: %d \n", htons(sa->sin_port) );
  inet_ntop(res->ai_family,&(sa->sin_addr), str, res->ai_addrlen);
  printf("HOST IP: %s \n", str);
  printf("AF_FAMILY: %d \n", p->ai_family);
  gethostname(hostname, sizeof(hostname));
  printf("hostname: %s \n", hostname);
  printf("server: waiting for connections... \n");
  // ---end

  
  for(;;) {
    read_fds = master;
    if(select(fdmax+1, &master, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(4);
    }
    // run through the existing connections looking for data to read
    for(int i = 0; i <= fdmax; i++) {
      if(FD_ISSET(i, &read_fds) ) {// we got one!!
        if(i == sockfd) {
        // handle new connections
          addrlen = sizeof client_addr;
          newfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
          
          if(newfd == -1) {
            perror("accept");
          } else {
            FD_SET(newfd, &master);// add to master set
            if(newfd > fdmax) {// keep track of the max
              fdmax = newfd;
            }
            printf("selectserver: new connection from %s on "
                   "socket %d\n", inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr*)&client_addr),remoteIP, INET6_ADDRSTRLEN), newfd);
          }
        } else {
          // handle data from a client
          if((bytes_rec = recv(i, msg, sizeof msg, 0)) <=0) {
            // got error or connection closed by client
            if(bytes_rec == 0) { // connection closed
              printf("selectserver: socket %d hung up\n", i);
            } else {
              perror("recv");
            }
            close(i); // bye!
            FD_CLR(i, &master);// remove from master set
          } else {
            // we got some data from a client
            printf("Message received: %s, bytes received: %lu \n", msg, bytes_rec); //We can also do something else with the data, like send a MIDI message
          }
        }
      }
    }
  }


    /*
    addr_size = sizeof client_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
    if (new_fd == -1){
      perror("accept");
      continue;
    }
    struct sockaddr_in *sa = (struct sockaddr_in *)&client_addr;
    inet_ntop(client_addr.ss_family, &(sa->sin_addr), str, res->ai_addrlen);
    printf("server: got connection from %s \n", str);
    if (n
    while ((bytes_rec = recv(new_fd, msg, 128, 0)) != 0) {
        printf("Message received: %s, bytes received: %lu \n", msg, bytes_rec);
    }
  }  
  freeaddrinfo(res);
    */
  return 0;
}
