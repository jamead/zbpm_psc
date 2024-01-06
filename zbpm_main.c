#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "zbpm_regs.h"
#include "zbpm_msg.h"
#include "zbpm_defs.h"

int main() {
   
    pthread_t thread_cntrl, thread_status, thread_wvfm;

    printf("Starting...\n");




    if (pthread_create(&thread_cntrl, NULL, psc_cntrl_thread, NULL) != 0) {
       perror("Thread creation failed");
       exit(1);
    }


    if (pthread_create(&thread_status, NULL, psc_status_thread, NULL) != 0) {
       perror("Thread creation failed");
       exit(1);
    }

    if (pthread_create(&thread_wvfm, NULL, psc_wvfm_thread, NULL) != 0) {
       perror("Thread creation failed");
       exit(1);
    }
    
     
    while (1) 
       sleep(1);  
   
	
    exit(0);

}
