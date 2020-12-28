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

int open_listenfd_udp(int port)  
{ 
  int listenfd, optval=1; 
  struct sockaddr_in serveraddr; 
   
  /* Create a socket descriptor */ 
  if ((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
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
 
  return listenfd; 
} 

int max(int x, int y)
{
    return (x > y ) ? x : y;
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
  char response[MAXLINE];

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
  memset(response, '\0', MAXLINE);
  file_check = access(fpath, F_OK);
  if(file_check==0){
    read_check = access(fpath, R_OK);
    if(read_check==0){
      fp =  fopen(fpath, "r");
      if(fp){
      strcpy(response, "HTTP/1.0 200 OK\r\n\r\n");
      write(connfd, response, strlen(response));}
    }
    else{
      strcpy(response, "HTTP/1.0 403 Forbidden\r\n\r\n");
      write(connfd, response, strlen(response));
      return;
    }
  }
  else{
    strcpy(response, "HTTP/1.0 404 Not Found\r\n\r\n");
    write(connfd, response, strlen(response));
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
  int listenfd, listenfd_udp, connfd, port, port_udp, clientlen, clientlen_udp;
  struct sockaddr_in clientaddr, clientaddr_udp;
  struct in_addr temp_addr;
  struct hostent *hp;
  char *haddrp;
  fd_set rfds;
  int maxfd, retval, rec_len, seq_num;
  char buf[MAXLINE];

  port = atoi(argv[1]); /* the server listens on a port passed 
			   on the command line */
  port_udp = atoi(argv[2]);
  listenfd = open_listenfd(port); 
  listenfd_udp = open_listenfd_udp(port_udp); 

  while (1) {
    FD_ZERO(&rfds);
		FD_SET(listenfd, &rfds);
		FD_SET(listenfd_udp, &rfds);
    
    maxfd=max(listenfd,listenfd_udp)+1;
    retval = select(maxfd, &rfds, NULL, NULL, NULL); 
    
    if (FD_ISSET(listenfd, &rfds)){
      clientlen = sizeof(clientaddr); 
      connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
      
      if(fork() == 0){
        hp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
              sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        haddrp = inet_ntoa(clientaddr.sin_addr);
        //close(listenfd);
        echo(connfd);
        exit(0);
      }
      close(connfd);
    }
    else if (FD_ISSET(listenfd_udp, &rfds)){
      memset(buf, '\0', MAXLINE);
      seq_num = 0;
      clientlen_udp = sizeof(clientaddr_udp); 

      rec_len = recvfrom(listenfd_udp, buf, MAXLINE, 0, (struct sockaddr *) &clientaddr_udp, &clientlen_udp);
      if(rec_len == -1){
        return -1;
      }

      memcpy(&seq_num, buf + rec_len - 4, 4);
      seq_num = ntohl(seq_num) + 1;
      seq_num = htonl(seq_num);

      memset(buf + rec_len - 4, '\0', 4);
      
      inet_aton(buf, &temp_addr);
      hp = gethostbyaddr((const char *)&temp_addr.s_addr, sizeof(temp_addr.s_addr), AF_INET);
      
      memset(buf, '\0', MAXLINE);
			memcpy(buf, hp->h_name, strlen(hp->h_name));
			memcpy(buf + strlen(hp->h_name), &seq_num, 4);

			sendto(listenfd_udp, buf, strlen(hp->h_name) + 4, 0, (struct sockaddr *) &clientaddr_udp, clientlen_udp);
		}
    
  }
}

