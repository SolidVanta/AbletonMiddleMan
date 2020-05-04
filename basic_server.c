#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define MY_PORT "3490"
#define BACKLOG 10
#define MAX_SIZE 128

struct addrinfo settings;
struct addrinfo *res, *p; // will point to the result
struct sockaddr_storage client_addr;
int sockfd, new_fd, yes=1, status;
socklen_t addr_size;
char hostname[128];
size_t size;
char str[MAX_SIZE];
char msg[MAX_SIZE];
int len;
size_t bytes_rec;

void setup(struct addrinfo *settings) {

  memset(settings, 0, sizeof settings); // make sure the struct is empty
  settings->ai_family = AF_UNSPEC; // don't care if IPv4 or IPv6
  settings->ai_flags = AI_PASSIVE; // use my IP address
  settings->ai_socktype = SOCK_STREAM; // use TCP stream sockets
 
}

int main()
{

  setup(&settings);
  if ((status = getaddrinfo(NULL, "3490", &settings, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }


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
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  
  struct sockaddr_in *sa = (struct sockaddr_in *)res->ai_addr;
  printf("PORT: %d \n", htons(sa->sin_port) );
  inet_ntop(res->ai_family,&(sa->sin_addr), str, res->ai_addrlen);
  printf("HOST IP: %s \n", str);
  printf("AF_FAMILY: %d \n", p->ai_family);
  gethostname(hostname, sizeof(hostname));
  printf("hostname: %s \n", hostname);
  printf("server: waiting for connections... \n");

  while(1){
    addr_size = sizeof client_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
    if (new_fd == -1){
      perror("accept");
      continue;
    }
    struct sockaddr_in *sa = (struct sockaddr_in *)&client_addr;
    inet_ntop(client_addr.ss_family, &(sa->sin_addr), str, res->ai_addrlen);
    printf("server: got connection from %s \n", str);
    
    while ((bytes_rec = recv(new_fd, msg, 128, 0)) != 0) {
        printf("Message received: %s, bytes received: %lu \n", msg, bytes_rec);
    }
  }


  
  freeaddrinfo(res);
  return 0;
}
