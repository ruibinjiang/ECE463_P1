#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 500

int open_clientfd(char *hostname, int port) 
{ 
  int clientfd; 
  struct hostent *hp; 
  struct sockaddr_in serveraddr; 
 
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    return -1; /* check errno for cause of error */ 
 
  /* Fill in the server's IP address and port */ 
  if ((hp = gethostbyname(hostname)) == NULL) 
    return -2; /* check h_errno for cause of error */ 
  bzero((char *) &serveraddr, sizeof(serveraddr)); 
  serveraddr.sin_family = AF_INET; 
  bcopy((char *)hp->h_addr,  
        (char *)&serveraddr.sin_addr.s_addr, hp->h_length); 
  serveraddr.sin_port = htons(port); 
 
  /* Establish a connection with the server */ 
  if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
    return -1; 
  return clientfd; 
} 


/* usage: ./echoclient host port */
int main(int argc, char **argv)
{ 
  int clientfd, port; 
  char *host, *filepath, *check_response, buf[MAXLINE]; 
  check_response = NULL;

  host = argv[1]; 
  port = atoi(argv[2]); 
  filepath = argv[3];

  //make the GET request string
  memset(buf, '\0', MAXLINE);
  strcat(buf, "GET ");
  strcat(buf, filepath);
  strcat(buf, " HTTP/1.0\r\n\r\n");
  //printf("%s", buf);

  //connect to server
  clientfd = open_clientfd(host, port);

  if (clientfd < 0){
    printf("Error opening connection \n");
    exit(0);
  }
  
  //send GET request and receive response
  write(clientfd, buf, strlen(buf));
  read(clientfd, buf, MAXLINE); 
  fputs(buf, stdout); 
  close(clientfd); 
  
  //check server response
  check_response = strstr(buf, "HTTP/1.1 200 OK");

  if(!check_response){
    printf("Error bad server response \n");
    exit(0);
  }
  
  //retrieve file path
  check_response = strstr(buf, "\r\n\r\n");
  if(!check_response){
    printf("Error bad server response \n");
    exit(0);
  }
  char*newpath = (char*)malloc(strlen(check_response)-3);
  strcpy(newpath,check_response+4);
  newpath[strlen(newpath)-1] = '\0';
  //printf("%s", newpath);

  //make the GET request string
  memset(buf, '\0', MAXLINE);
  strcat(buf, "GET ");
  strcat(buf, newpath);
  strcat(buf, " HTTP/1.0\r\n\r\n");
  //printf("%s", buf);
  free(newpath);

  //connect to server
  clientfd = open_clientfd(host, port);

  if (clientfd < 0){
    printf("Error opening connection \n");
    exit(0);
  }

  //send GET request and receive response
  write(clientfd, buf, strlen(buf));
  int n = 1;
  while(n!=0){
    n = read(clientfd, buf, MAXLINE); 
    buf[n] = '\0';
    printf("%s", buf);
  } 
  
  
  close(clientfd); 
  exit(0); 
} 



