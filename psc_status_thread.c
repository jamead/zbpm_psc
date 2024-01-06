/********************************************************************
*  PSC Status Thread 
*  version 2
*  1-17-24
*
*  This thread is responsible for sending all slow data (10Hz) to the IOC.   It does
*  this over to message ID's (30 = slow status, 31 = 10Hz data)
*
*  It starts a listening server on
*  port 600.  Upon establishing a connection with a client, it begins to send out
*  packets containing all 10Hz updated data.
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "zbpm_regs.h"
#include "zbpm_msg.h"
#include "zbpm_defs.h"


void Host2NetworkConvStatus(char *inbuf, char *outbuf) {

	int i;

	for (i=0;i<8;i++) 
	    outbuf[i] = inbuf[i];
	for (i=8;i<MSGID30LEN;i=i+4) {
	    //printf("In %d: %d %d %d %d\n",i,inbuf[i],inbuf[i+1],inbuf[i+2],inbuf[i+3]);
	    outbuf[i] = inbuf[i+3];
	    outbuf[i+1] = inbuf[i+2];
	    outbuf[i+2] = inbuf[i+1];
	    outbuf[i+3] = inbuf[i];
	    //printf("Out %d: %d %d %d %d\n\n",i,outbuf[i],outbuf[i+1],outbuf[i+2],outbuf[i+3]);
        }	
}



void ReadBrdTemp(volatile unsigned int *fpgabase, int regaddr, char *msg) { 

    int tempraw; 
    float temp; 

    tempraw = fpgabase[regaddr];
    if (tempraw >= 0x8000)
	tempraw = tempraw | 0xFFFF0000;
    temp = tempraw * 0.0078125;
    //printf("Temp = %f\n",temp);
    memcpy(msg,&temp,sizeof(int));
    //printf("Size of int: %d\n",sizeof(int)); 
}


void ReadReg(volatile unsigned int *fpgabase, int regaddr, char *msg) {

    int rawreg;

    rawreg = fpgabase[regaddr];
    memcpy(msg,&rawreg,sizeof(int));
    //printf("Reg %d = %d\n",regaddr,rawreg);
}  



void ReadFpgaRegs(volatile unsigned int *fpgabase, char *msgid30, char *msgid31) {

    //Slow Control Data - PSC Message ID 30
    ReadReg(fpgabase,KX_REG,&msgid30[POS_KX_MSG30]);
    ReadReg(fpgabase,KY_REG,&msgid30[POS_KY_MSG30]);

    ReadReg(fpgabase,CHA_GAIN_REG,&msgid30[CHA_GAIN_MSG30]);
    ReadReg(fpgabase,CHB_GAIN_REG,&msgid30[CHB_GAIN_MSG30]);
    ReadReg(fpgabase,CHC_GAIN_REG,&msgid30[CHC_GAIN_MSG30]);
    ReadReg(fpgabase,CHD_GAIN_REG,&msgid30[CHD_GAIN_MSG30]);

    ReadReg(fpgabase,FPGA_VER_REG,&msgid30[FPGA_VER_MSG30]);
     

    ReadBrdTemp(fpgabase,TEMP_DFE0_REG,&msgid30[TEMP_DFESENSE0_MSG30]);
    ReadBrdTemp(fpgabase,TEMP_DFE1_REG,&msgid30[TEMP_DFESENSE1_MSG30]);
    ReadBrdTemp(fpgabase,TEMP_DFE2_REG,&msgid30[TEMP_DFESENSE2_MSG30]);
    ReadBrdTemp(fpgabase,TEMP_DFE3_REG,&msgid30[TEMP_DFESENSE3_MSG30]);
    ReadBrdTemp(fpgabase,TEMP_AFE0_REG,&msgid30[TEMP_AFESENSE0_MSG30]);
    ReadBrdTemp(fpgabase,TEMP_AFE1_REG,&msgid30[TEMP_AFESENSE1_MSG30]);

    //10Hz Slow Data - PSC Message ID 31
    ReadReg(fpgabase, EVR_TS_NS_REG, &msgid31[TS_NS_MSG31]);
    ReadReg(fpgabase, EVR_TS_S_REG, &msgid31[TS_MSG31]);
   
    ReadReg(fpgabase, SA_CHA_REG, &msgid31[AMPL_ASA_MSG31]);
    ReadReg(fpgabase, SA_CHB_REG, &msgid31[AMPL_BSA_MSG31]);
    ReadReg(fpgabase, SA_CHC_REG, &msgid31[AMPL_CSA_MSG31]);
    ReadReg(fpgabase, SA_CHD_REG, &msgid31[AMPL_DSA_MSG31]);
    ReadReg(fpgabase, SA_XPOS_REG, &msgid31[POS_X_MSG31]);
    ReadReg(fpgabase, SA_YPOS_REG, &msgid31[POS_Y_MSG31]);
    ReadReg(fpgabase, SA_SUM_REG, &msgid31[AMPL_SUM_MSG31]);
 
    ReadReg(fpgabase, EVR_DMA_TRIGNUM_REG, &msgid31[TRIG_EVENTNO_MSG31]);
    ReadReg(fpgabase, SA_TRIGNUM_REG, &msgid31[CNT_TRIG_MSG31]);



}







void *psc_status_thread(void *arg)
{
    int sockfd, newsockfd, portno, clilen;
    char msgid30_buf[1024];
    char msgid30_bufntoh[1024];
    int *msgid30_bufptr;
    char msgid31_buf[1024];
    char msgid31_bufntoh[1024];
    int *msgid31_bufptr;

    struct sockaddr_in serv_addr, cli_addr;
    int  n, loop=0;
    int sa_cnt, sa_cnt_prev;
    int fd;
    volatile unsigned int *fpgabase;


    signal(SIGPIPE, SIG_IGN);
    printf("Starting Tx 10Hz Server...\n");
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 600;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) 
       printf("setsockopt(SO_REUSEADDR) failed\n");


    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
                          sizeof(serv_addr)) < 0)
    {
         printf("ERROR on binding\n");
         exit(1);
    }

    /* Now start listening for the clients, here process will
    * go in sleep mode and will wait for the incoming connection */
    printf("10Hz socket: Listening for Connection...\r\n");
    listen(sockfd,5);


    // Open /dev/mem for writing to FPGA register 
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (fd < 0)  {
      printf("Can't open /dev/mem\n");
      exit(1);
    }
    //printf("Opened /dev/mem\r\n");

    fpgabase = (unsigned int *) mmap(0,getpagesize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,IOBUS_BASEADDR);

    if (fpgabase == NULL) {
       printf("Can't mmap\n");
       exit(1);
    }
    else
       printf("FPGA mmap'd\n");



reconnect:
    clilen = sizeof(cli_addr);
    printf("10Hz socket: Waiting to accept connection...\n");
    /* Accept actual connection from the client */
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                                &clilen);
    if (newsockfd < 0) {
        printf("10Hz socket: ERROR on accept");
        exit(1);
    }
    /* If connection is established then start communicating */
    printf("10Hz socket: Connected Accepted...\n");

    bzero(msgid30_buf,sizeof(msgid30_buf));
    msgid30_bufptr = (int *)msgid30_buf; 
    msgid30_buf[0] = 'P';
    msgid30_buf[1] = 'S';
    msgid30_buf[2] = 0;
    msgid30_buf[3] = (short int) MSGID30;
    *++msgid30_bufptr = htonl(MSGID30LEN);  //body length
    
    bzero(msgid31_buf,sizeof(msgid31_buf));
    msgid31_bufptr = (int *)msgid31_buf; 
    msgid31_buf[0] = 'P';
    msgid31_buf[1] = 'S';
    msgid31_buf[2] = 0;
    msgid31_buf[3] = (short int) MSGID31;
    *++msgid31_bufptr = htonl(MSGID31LEN);  //body length
	


    sa_cnt_prev = sa_cnt = 0;
    while (1) {
        //printf("In main loop...\n");
 	usleep(1000);
	do {
	   sa_cnt = fpgabase[SA_TRIGNUM_REG];
	   //printf("SA CNT: %d\n",sa_cnt);
	   usleep(100);
	}
        while (sa_cnt_prev == sa_cnt);
        sa_cnt_prev = sa_cnt;

        ReadFpgaRegs(fpgabase,&msgid30_buf[MSGHDRLEN],&msgid31_buf[MSGHDRLEN]);
	//printf("%8d:  Reading FPGA Registers...\n",sa_cnt);
        //for (i=0;i<60;i++)
	//    printf("%d:  %d\n",i*4-8,bufptr[i]);

        /*for (i=0;i<232;i=i+4) {
	    printf("%d:  %d  %d  %d  %d\n",i,buffer[i],buffer[i+1],buffer[i+2],buffer[i+3]);
	    printf("%d:  %d  %d  %d  %d\n",i,bufferntoh[i],bufferntoh[i+1],bufferntoh[i+2],bufferntoh[i+3]);
        }
        */
    

       /*print msg31 packet
       for (i=0;i<44;i=i+4) {
	    printf("%d:  %d  %d  %d  %d\n",i,msgid31_buf[i],msgid31_buf[i+1],msgid31_buf[i+2],msgid31_buf[i+3]);
	    //printf("%d:  %d  %d  %d  %d\n",i,bufferntoh[i],bufferntoh[i+1],bufferntoh[i+2],bufferntoh[i+3]);
        }
        */


	Host2NetworkConvStatus(msgid30_buf,msgid30_bufntoh);	
	n = write(newsockfd,msgid30_bufntoh,MSGID30LEN+MSGHDRLEN);
        if (n < 0) {
            printf("10Hz socket: ERROR writing to socket\n");
            close(newsockfd);
            goto reconnect;
            //exit(1);
        }


       //write msg31 packet
        Host2NetworkConvStatus(msgid31_buf,msgid31_bufntoh);	
	n = write(newsockfd,msgid31_bufntoh,MSGID31LEN+MSGHDRLEN);
        if (n < 0) {
            printf("10Hz socket: ERROR writing to socket\n");
            close(newsockfd);
            goto reconnect;
            //exit(1);
        }



	loop++;

        }

    return 0;





}

