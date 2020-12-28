#include <stdio.h>
#include <string.h>
#include <sys/select.h>

int main(void) {
  fd_set rfds;
  struct timeval tv;
  int retval;
  char ch1,ch2;

  while(1){
    tv.tv_sec = 5; tv.tv_usec = 0; /* Wait up to five seconds. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    retval = select(1, &rfds, NULL, NULL, &tv);  

    if (FD_ISSET(0, &rfds)){
      scanf("%c",&ch1);  scanf("%c",&ch2);
      printf("Retval: %d; Data is available now: %c %c %d %d \n",retval,ch1,ch2,tv.tv_sec,tv.tv_usec);
    }
    else{
      printf("Retval: %d; No data within five seconds.\n",retval);
    }
  }
}

