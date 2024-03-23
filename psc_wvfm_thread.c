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




void Host2NetworkConvWvfm(char *inbuf, char *outbuf, int len) {

    int i;

    for (i=0;i<8;i++) 
        outbuf[i] = inbuf[i];
    for (i=8;i<len;i=i+4) {
        //printf("In %d: %d %d %d %d\n",i,inbuf[i],inbuf[i+1],inbuf[i+2],inbuf[i+3]);
        outbuf[i] = inbuf[i+3];
        outbuf[i+1] = inbuf[i+2];
        outbuf[i+2] = inbuf[i+1];
        outbuf[i+3] = inbuf[i];
       //printf("Out %d: %d %d %d %d\n\n",i,outbuf[i],outbuf[i+1],outbuf[i+2],outbuf[i+3]);
        }	
}


void ReadTbTWvfm(volatile unsigned int *fpgabase, char *msg) {

    int i;
    int *msgptr;
    int databuf[30000];  //1024pts * 7val * 4bytes/val
    //int cha, chb, chc, chd, x, y, sum;
    int wordCnt, wordsRead, samp_cnt, regVal;
    int ts_s, ts_ns;
     
    msgptr = (int *) msg;

    printf("Reading TbT FIFO...\n");
    printf("\tWords in TbT FIFO = %d\n",fpgabase[TBTFIFO_CNT_REG]);
  
    // first word is a 0x80000000
    printf("\tTbT header: %x\n",fpgabase[TBTFIFO_DATA_REG]); 
    // next 2 words are the timestamp trigger 
    ts_s = fpgabase[TBTFIFO_DATA_REG];
    ts_ns = fpgabase[TBTFIFO_DATA_REG];
    printf("\tTrigger Timestamp: %d   %d\n",ts_s,ts_ns);

    // Get TbT Waveform
    for (i=0;i<1024*7;i++) 
	*msgptr++ = fpgabase[TBTFIFO_DATA_REG];
        
    //printf("TbT FIFO Read Complete...\n");
    //printf("Resetting FIFO...\n");
    fpgabase[TBTFIFO_RST_REG] = 0x1;
    usleep(1);
    fpgabase[TBTFIFO_RST_REG] = 0x0;
    usleep(10);
  
}




void ReadADCWvfm(volatile unsigned int *fpgabase, char *msg) {

    int i;
    short int *msgptr;
    int adcdatabuf[20000];
    int adcval_cha,adcval_chb,adcval_chc,adcval_chd;
    int wordCnt, wordsRead, samp_cnt, regVal;
    int hdr, ts_s, ts_ns;
 

    msgptr = (short int *) msg;
    printf("Reading ADC FIFO...\n");
    printf("\tWords in ADC FIFO = %d\n",fpgabase[ADCFIFO_CNT_REG]);
    // first 2 words is 0x80000000 and 0x00000000
    printf("\tADC header: %x\n",fpgabase[ADCFIFO_DATA_REG]); 
    hdr = fpgabase[ADCFIFO_DATA_REG]; 
    // next 2 words are the timestamp trigger 
    ts_s = fpgabase[ADCFIFO_DATA_REG];
    ts_ns = fpgabase[ADCFIFO_DATA_REG];
    printf("\tTrigger Timestamp: %d   %d\n",ts_s,ts_ns);


    for (i=0;i<7900;i=i+2) {
        //chA and chB are in a single 32 bit word 
	regVal = fpgabase[ADCFIFO_DATA_REG];
        *msgptr++ = (short int) ((regVal & 0xFFFF0000) >> 16);
        *msgptr++ = (short int) (regVal & 0xFFFF);
        //chC and chD are in a single 32 bit word 
        regVal = fpgabase[ADCFIFO_DATA_REG];
        *msgptr++ = (short int) ((regVal & 0xFFFF0000) >> 16);
        *msgptr++ = (short int) (regVal & 0xFFFF);
    } 

    //printf("Resetting FIFO...\n");
    fpgabase[ADCFIFO_RST_REG] = 0x1;
    usleep(1);
    fpgabase[ADCFIFO_RST_REG] = 0x0;
    usleep(10);
 

}



void *psc_wvfm_thread(void *arg)
{
    int sockfd, newsockfd, portno, clilen;
    char msgid51_buf[MSGID51LEN];
    char msgid51_bufntoh[MSGID51LEN];
    int *msgid51_bufptr;
    char msgid52_buf[MSGID52LEN];
    char msgid52_bufntoh[MSGID52LEN];
    int *msgid52_bufptr;
    struct sockaddr_in serv_addr, cli_addr;
    int  i, n, loop=0;
    int fd;
    volatile unsigned int *fpgaiobase, *fpgalivebase;
    int prevtrignum, newtrignum, test;

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

    fpgalivebase = (unsigned int *) mmap(0,getpagesize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,LIVEBUS_BASEADDR);
    fpgaiobase   = (unsigned int *) mmap(0,getpagesize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,IOBUS_BASEADDR);


    if (fpgaiobase == NULL || fpgalivebase == NULL) {
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

    // ADC Data waveform
    bzero(msgid51_buf,sizeof(msgid51_buf));
    msgid51_bufptr = (int *)msgid51_buf; 
    msgid51_buf[0] = 'P';
    msgid51_buf[1] = 'S';
    msgid51_buf[2] = 0;
    msgid51_buf[3] = (short int) MSGID51;
    *++msgid51_bufptr = htonl(MSGID51LEN);  //body length
	
    // TbT Data waveform
    bzero(msgid52_buf,sizeof(msgid52_buf));
    msgid52_bufptr = (int *)msgid52_buf; 
    msgid52_buf[0] = 'P';
    msgid52_buf[1] = 'S';
    msgid52_buf[2] = 0;
    msgid52_buf[3] = (short int) MSGID52;
    *++msgid52_bufptr = htonl(MSGID52LEN);  //body length
    
    prevtrignum = 0;
    newtrignum = 0;
    
    while (1) {
        //wait for a trigger
        while (newtrignum == prevtrignum) { 
	    newtrignum = fpgaiobase[DMA_TRIGCNT_REG];
	    usleep(1000);	
	}	    
   	printf("\nTrig Num: %d  \n",newtrignum);     
	
	prevtrignum = newtrignum; 

	ReadADCWvfm(fpgalivebase,&msgid51_buf[MSGHDRLEN]);
	//printf("%8d:  Reading ADC Waveform...\n",loop);

	ReadTbTWvfm(fpgalivebase,&msgid52_buf[MSGHDRLEN]);
	//printf("%8d:  Reading TbT Waveform...\n",loop);


	//for (i=0;i<60;i++)
	//    printf("%d:  %d\n",i*4-8,bufptr[i]);


        //print msg51 packet
        //for (i=0;i<44;i=i+4) 
	//        printf("%d:  %d  %d  %d  %d\n",i,msgid51_buf[i],msgid51_buf[i+1],
	//    					   msgid51_buf[i+2],msgid51_buf[i+3]);
        


	//write out msg51
	Host2NetworkConvWvfm(msgid51_buf,msgid51_bufntoh,sizeof(msgid51_buf));	
	n = write(newsockfd,msgid51_bufntoh,MSGID51LEN+MSGHDRLEN);
        if (n < 0) {
            printf("Waveform socket: ERROR writing MSG 51 - ADC Waveform\n");
            close(newsockfd);
            goto reconnect;
        }

	//write out msg52
	Host2NetworkConvWvfm(msgid52_buf,msgid52_bufntoh,sizeof(msgid52_buf));	
	n = write(newsockfd,msgid52_bufntoh,MSGID52LEN+MSGHDRLEN);
        if (n < 0) {
            printf("Waveform socket: ERROR writing MSG 52 - TbT Waveform\n");
            close(newsockfd);
            goto reconnect;
        }





	loop++;

    }

   return NULL; 





}

