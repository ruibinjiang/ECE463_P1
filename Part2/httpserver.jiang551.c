#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXLINE 500

int open_listenfd(int port)  
{ 
  int listenfd, optval=1; 
  struct sockaddr_in serveraddr; 
   
  /* Create a socket descriptor */ 
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    return -1; 
  
  /* Eliminates "Address already in use" error from bind. */ 
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,  
		 (const void *)&optval , sizeof(int)) < 0) 
    return -1; 
 
  /* Listenfd will be an endpoint for all requests to port 
     on any IP address for this host */ 
  bzero((char *) &serveraddr, sizeof(serveraddr)); 
  serveraddr.sin_family = AF_INET;  
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  
  serveraddr.sin_port = htons((unsigned short)port);  
  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
    return -1; 
 
  /* Make it a listening socket ready to accept 
     connection requests */ 
  if (listen(listenfd, LISTENQ) < 0) 
    return -1; 
 
  return listenfd; 
} 

void echo(int connfd)  
{ 
  size_t n;  
  FILE* fp = NULL;
  int isGet = 0;
  int shift_num = 0;
  int file_check = 1;
  int read_check = 1;
  char buf[MAXLINE];  
  char fpath[MAXLINE];

  //check GET
  do{
    n = read(connfd, buf, MAXLINE);
    if(n!=0){
      if(strstr(buf, "GET") != NULL){
        isGet = 1;
      }
    }
  }while(isGet==0);
  
  //get file path
  memset(fpath, '\0', MAXLINE);
  if(n>19){
    if(buf[4]==47){
      strncpy(fpath, buf + 5, n - 20);
    }
    else{
      strncpy(fpath, buf + 4, n - 19);
    }
    shift_num = buf[n-14]-48;
  }
  
  //check file status
  file_check = access(fpath, F_OK);
  if(file_check==0){
    read_check = access(fpath, R_OK);
    if(read_check==0){
      fp =  fopen(fpath, "r");
      if(fp){
      write(connfd, "HTTP/1.0 200 OK\r\n\r\n", 23);}
    }
    else{
      write(connfd, "HTTP/1.0 403 Forbidden\r\n\r\n", 30);
      return;
    }
  }
  else{
    write(connfd, "HTTP/1.0 404 Not Found\r\n\r\n", 30);
    return;
  }
  
  //Caesar Shift
  isGet = 0;
  int i = 0;
  int letter;
  do{
    n = fread(buf, sizeof(char), MAXLINE, fp);
    if(n!=0){
      for(i = 0; i < n; i++){
        letter = buf[i];
        if(letter >= 'a' && letter <= 'z'){
          letter-=shift_num;
          if(letter < 'a'){
            letter+=26;
          }
          buf[i]=letter;
        }
        else if(letter >= 'A' && letter <= 'Z'){
          letter-=shift_num;
          if(letter < 'A'){
            letter+=26;
          }
          buf[i]=letter;
        }
      }
      write(connfd, buf, n);
      memset(buf, '\0', MAXLINE);
    }
    else{
      isGet = 1;
    }
  }while(isGet==0);
  
  fclose(fp);
  return;
} 


int main(int argc, char **argv) {
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  struct hostent *hp;
  char *haddrp;

  port = atoi(argv[1]); /* the server listens on a port passed 
			   on the command line */
  listenfd = open_listenfd(port); 

  while (1) {
    clientlen = sizeof(clientaddr); 
    connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
    hp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
		       sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    haddrp = inet_ntoa(clientaddr.sin_addr);

    echo(connfd);
    close(connfd);
  }
}

