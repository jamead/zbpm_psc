/********************************************************************
*  PSC Waveform
*
*  This thread is responsible for sending all waveform data to the IOC
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




void Host2NetworkConvWvfm(char *inbuf, char *outbuf) {

    int i;

    for (i=0;i<8;i++) 
        outbuf[i] = inbuf[i];
    for (i=8;i<MSGID51LEN;i=i+4) {
        //printf("In %d: %d %d %d %d\n",i,inbuf[i],inbuf[i+1],inbuf[i+2],inbuf[i+3]);
        outbuf[i] = inbuf[i+3];
        outbuf[i+1] = inbuf[i+2];
        outbuf[i+2] = inbuf[i+1];
        outbuf[i+3] = inbuf[i];
       //printf("Out %d: %d %d %d %d\n\n",i,outbuf[i],outbuf[i+1],outbuf[i+2],outbuf[i+3]);
        }	
}





void ReadADCWvfm(volatile unsigned int *fpgabase, char *msg) {

    int i;
    short int *msgptr;
    int adcdatabuf[20000];
    int adcval_cha,adcval_chb,adcval_chc,adcval_chd;
    int wordCnt, wordsRead, samp_cnt, regVal;
     
    msgptr = (short int *) msg;

    //printf("Resetting FIFO...\n");
    fpgabase[ADCFIFO_RST_REG] = 0x1;
    usleep(1);
    fpgabase[ADCFIFO_RST_REG] = 0x0;
    usleep(10);
    //printf("Words in FIFO = %d\n",fpgabase[ADCFIFO_CNT_REG]);
    //printf("Starting ADC FIFO write burt (8K samples)\n");
    fpgabase[ADCFIFO_STREAMENB_REG] = 1;
    fpgabase[ADCFIFO_STREAMENB_REG] = 0;
    //printf("Waiting for ADC Data to Start...\n");
    while (fpgabase[ADCFIFO_CNT_REG] == 0)
         usleep(10000);

    printf("Running...\n");
    usleep(500000);
    wordCnt = fpgabase[ADCFIFO_CNT_REG];
    wordsRead = 0;
    //printf("FIFO Word Count: %d\n",wordCnt);

    while (wordCnt != 0) {
       wordCnt = fpgabase[ADCFIFO_CNT_REG];
       regVal = fpgabase[ADCFIFO_DATA_REG];
       adcdatabuf[wordsRead] = regVal;
       wordsRead++;
       }

    printf("ADC FIFO Read Complete, words read = %d\n",wordsRead);
    //printf("Remaining ADC Word Count : %d\n",fpgabase[ADCFIFO_CNT_REG]);
    //printf("Results...\n");
    samp_cnt = 0;
    //printf("ADC raw data\n");
    for (i=0;i<wordsRead;i=i+2) {
         adcval_cha = (short int) ((adcdatabuf[i] & 0xFFFF0000) >> 16);
         adcval_chb = (short int) (adcdatabuf[i] & 0xFFFF);
         adcval_chc = (short int) ((adcdatabuf[i+1] & 0xFFFF0000) >> 16);
         adcval_chd = (short int) (adcdatabuf[i+1] & 0xFFFF);
         //printf("%d\t%d\t%d\t%d\t%d\t\n",i,adcval_cha,adcval_chb,adcval_chc,adcval_chd);
         *msgptr++ = adcval_cha;
	 *msgptr++ = adcval_chb;
	 *msgptr++ = adcval_chc;
	 *msgptr++ = adcval_chd; 
	 samp_cnt++;
       }
    //printf("Samples = %d\n",samp_cnt);



}







void *psc_wvfm_thread(void *arg)
{
    int sockfd, newsockfd, portno, clilen;
    char msgid51_buf[MSGID51LEN];
    char msgid51_bufntoh[MSGID51LEN];
    int *msgid51_bufptr;

    struct sockaddr_in serv_addr, cli_addr;
    int  i, n, loop=0;
    int fd;
    volatile unsigned int *fpgabase;
    int prevtrignum, newtrignum;

    signal(SIGPIPE, SIG_IGN);
    printf("Starting Tx Waveform Server... \n");
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 20;
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
    printf("Waveform socket: Listening for Connection...\r\n");
    listen(sockfd,5);


    // Open /dev/mem for writing to FPGA register 
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (fd < 0)  {
      printf("Can't open /dev/mem\n");
      exit(1);
    }
    //printf("Opened /dev/mem\r\n");

    fpgabase = (unsigned int *) mmap(0,getpagesize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,LIVEBUS_BASEADDR);

    if (fpgabase == NULL) {
       printf("Can't mmap\n");
       exit(1);
    }
    else
       printf("FPGA mmap'd\n");



reconnect:
    clilen = sizeof(cli_addr);
    printf("Waveform socket: Waiting to accept connection...\n");
    /* Accept actual connection from the client */
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                                &clilen);
    if (newsockfd < 0) {
        printf("Waveform socket: ERROR on accept");
        exit(1);
    }
    /* If connection is established then start communicating */
    printf("Waveform socket: Connected Accepted...\n");

   
    bzero(msgid51_buf,sizeof(msgid51_buf));
    msgid51_bufptr = (int *)msgid51_buf; 
    msgid51_buf[0] = 'P';
    msgid51_buf[1] = 'S';
    msgid51_buf[2] = 0;
    msgid51_buf[3] = (short int) MSGID51;
    *++msgid51_bufptr = htonl(MSGID51LEN);  //body length
	

    prevtrignum = 0;
    newtrignum = 0;

    while (1) {
        //wait for a trigger
        while (newtrignum == prevtrignum) { 
	    newtrignum = fpgabase[EVR_DMA_TRIGNUM_REG];
            usleep(10000);	
	}	    
        prevtrignum = newtrignum; 

	ReadADCWvfm(fpgabase,&msgid51_buf[MSGHDRLEN]);
	//printf("%8d:  Reading ADC Waveform...\n",loop);
        //for (i=0;i<60;i++)
	//    printf("%d:  %d\n",i*4-8,bufptr[i]);


        //print msg51 packet
        //for (i=0;i<44;i=i+4) 
	//        printf("%d:  %d  %d  %d  %d\n",i,msgid51_buf[i],msgid51_buf[i+1],
	//    					   msgid51_buf[i+2],msgid51_buf[i+3]);
        


	Host2NetworkConvWvfm(msgid51_buf,msgid51_bufntoh);	
	n = write(newsockfd,msgid51_bufntoh,MSGID51LEN+MSGHDRLEN);
        if (n < 0) {
            printf("Waveform socket: ERROR writing to socket\n");
            close(newsockfd);
            goto reconnect;
            //exit(1);
        }



	loop++;

    }

   return NULL; 





}

