#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>



#define PORT "3490"
#define BACKLOG 10
#define HOSTSIZE 64

int main()
{
  struct addrinfo settings;
  struct addrinfo *res, *p; // will point to the result
  int sockfd, new_fd, yes, status;
  socklen_t addr_size;
  char hostname[HOSTSIZE];
  struct sockaddr_in *sa;
  
  char str[HOSTSIZE];
  memset(&settings, 0, sizeof settings); // make sure the struct is empty
  settings.ai_family = AF_UNSPEC; // don't care if IPv4 or IPv6
  settings.ai_socktype = SOCK_STREAM; // use TCP stream sockets
  
  if (gethostname(hostname, sizeof hostname) == -1){
    fprintf(stderr, "Could not fetch hostname");
    exit(1);
  }

  if ((status = getaddrinfo("0.0.0.0", PORT, &settings, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }
  
  printf("HOST: %s\n", hostname);
  printf("PORT: %s \n", settings.ai_canonname);


  for(p = res; p != NULL; p = res->ai_next){
    if ( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }
    printf("Socket created!\n");
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      fprintf(stderr, "Could not connect to %s \n", PORT);
      continue;
    }
    sa = (struct sockaddr_in *) p->ai_addr;
    printf("PORT: %d \t AI_FAMILY: %d \t %d \n", htons(sa->sin_port), p->ai_family, AF_INET);
    inet_ntop(p->ai_family,&(sa->sin_addr), str, p->ai_addrlen);
    printf("IP: %s \n",str);
    printf("We made a connection \n");
    break;
  }
  char *m = "Hello, this is a client!";
  size_t bytes_sent;
  bytes_sent = send(sockfd, m, strlen(m)+1,0);
  printf("Sent bytes: %lu\n", bytes_sent);
  freeaddrinfo(res);
  return 0;
}
