/********************************************************************
*  PSC Control Thread 
*  1-21-24
*  
*  This thread is responsible for setting all control registers on the
*  fpga that are sent from the IOC.  It starts a listening server on 
*  port 7.  Upon receiving a packet from the IOC it then does a write
*  to the appropriate FPGA register.
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
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




void soft_trig(volatile unsigned int *fpgabase, int msgVal) {
    fpgabase[SOFT_DMA_TRIG_REG] = msgVal;
}

void set_atten(volatile unsigned int *fpgabase, int whichatten, int msgVal) {
    	
    if (whichatten == RFATTEN)	{
       printf("Setting RF attenuator to %d dB\n",msgVal);
       fpgabase[RF_DSA_REG] = msgVal*4;
    }
    else { //pilot tone attenuator {
       printf("Setting PT attenuator to %d dB\n",msgVal);
       fpgabase[PT_DSA_REG] = msgVal*4;
    }

}

void set_geo_dly(volatile unsigned int *fpgabase, int msgVal) {
    // the Geo delay is the same as tbt_gate delay, set them both for now
    fpgabase[FINE_TRIG_DLY_REG] = msgVal; 
    fpgabase[TBT_GATEDLY_REG] = msgVal;
}


void set_coarse_dly(volatile unsigned int *fpgabase, int msgVal) {
    fpgabase[COARSE_TRIG_DLY_REG] = msgVal; 
}


void prog_lmx2541(volatile unsigned int *fpgabase, int msgVal) {

   printf("Programming LMX2451...\n");
   //register R7 : Resets all registers
   fpgabase[PT_SPI_REG] = 0x00000017;
   usleep(1000);

   // R13 
   fpgabase[PT_SPI_REG] = 0x0000008d;
   usleep(1000);

   // R12 : external VCO
   fpgabase[PT_SPI_REG] = 0x0000001c;
   usleep(1000);

   // R9  : program as shown
   fpgabase[PT_SPI_REG] = 0x28001409;
   usleep(1000);

   // R8 : 
   fpgabase[PT_SPI_REG] = 0x0111ce58;
   usleep(1000);

   // R6
   fpgabase[PT_SPI_REG] = 0x001f3336;
   usleep(1000);

   // R5
   fpgabase[PT_SPI_REG] = 0xA0000005;
   usleep(1000);

   // R4
   fpgabase[PT_SPI_REG] = 0x88084754;
   usleep(1000);

   // R3
   fpgabase[PT_SPI_REG] = 0x00387f03;
   usleep(1000);

   // R2
   fpgabase[PT_SPI_REG] = 0x04000002;
   usleep(1000);

   // R1
   fpgabase[PT_SPI_REG] = 0x000009b1;
   usleep(1000);

   // R0
   fpgabase[PT_SPI_REG] = 0x0001a4f0;
   usleep(1000);
   printf("Finished programming LMX2451\n"); 
}




void set_pilottone_rfenb(volatile unsigned int *fpgabase, int msgVal) {

   if (msgVal == 0) 
       // disable PT 
       fpgabase[PT_SPI_REG] = 0x00000017;
   else
       // program LMK chip 
       prog_lmx2541(fpgabase,MsgData); 
	
   fpgabase[PT_RFENB_REG] = msgVal; 
 	    
   
   }


void set_trigtobeam_thresh(volatile unsigned int *fpgabase, int msgVal) {
    fpgabase[TRIGTOBEAM_THRESH_REG] = msgVal; 
}




void set_machineloc(volatile unsigned int *fpgabase, int msgVal) {
// 0,1,2,4 = single pass mode
// 3 = Booster 
// 5 = Storage Ring	
    switch (msgVal) {
        case 0:
        case 1:
        case 2:
        case 4: 
            fpgabase[MACHINE_LOC_REG] = 0;
            break;	    
	case 3:   
	    fpgabase[MACHINE_LOC_REG] = 3;
	    break; 
	case 5:
            fpgabase[MACHINE_LOC_REG] = 5;
	    break;
    }
}



void set_eventno(volatile unsigned int *fpgabase, int msgVal) {
    fpgabase[EVR_DMA_TRIGNUM_REG] = msgVal; 
}

void set_trigsrc(volatile unsigned int *fpgabase, int msgVal) {
    if (msgVal == 0) {
        printf("Setting Trigger Source to EVR\n");
        fpgabase[TRIG_EVRINT_SEL_REG] = msgVal; 
    }
    else if (msgVal == 1) {
	printf("Setting Trigger Source to INT (soft)\n");
        fpgabase[TRIG_EVRINT_SEL_REG] = msgVal;
    } 
    else 
        printf("Invalid Trigger Source\n");

}



void set_kxky(volatile unsigned int *fpgabase, int axis, int msgVal) {
   
    if (axis == HOR)	{
       printf("Setting Kx to %d nm\n",msgVal);
       fpgabase[KX_REG] = msgVal;
    }
    else {
       printf("Setting Ky to %d nm\n",msgVal);
       fpgabase[KY_REG] = msgVal;
    }
}


void set_bbaoffset(volatile unsigned int *fpgabase, int axis, int msgVal) {
   
    if (axis == HOR)	{
       printf("Setting BBA X to %d nm\n",msgVal);
       fpgabase[BBA_XOFF_REG] = msgVal;
    }
    else { 
       printf("Setting BBA Y to %d nm\n",msgVal);
       fpgabase[BBA_YOFF_REG] = msgVal;
    }
}




void set_gain(volatile unsigned int *fpgabase, int channel, float msgValflt) {

   int msgVal;
   msgVal = 0x7FFF*msgValflt;

   switch(channel) {
        case CHA:
	   fpgabase[CHA_GAIN_REG] = msgVal;
           printf("Setting ChA gain to %d 0x%x %f\n",msgVal, msgVal, msgValflt);
	   break;
	case CHB:
	   fpgabase[CHB_GAIN_REG] = msgVal;
           printf("Setting ChB gain to %d\n",msgVal);
	   break;
         case CHC:
	   fpgabase[CHC_GAIN_REG] = msgVal;
           printf("Setting ChC gain to %d\n",msgVal);
	   break;
	case CHD:
	   fpgabase[CHD_GAIN_REG] = msgVal;
           printf("Setting ChD gain to %d\n",msgVal);
	   break;
        default:
           printf("Invalid gain channel number\n");	   
	   break;
    }
}




void *psc_cntrl_thread(void *arg)
{
    int clilen;
    char buffer[256];
    char tempbuf[4];
    struct sockaddr_in serv_addr, cli_addr;
    int  n, *bufptr, numpackets;
    int MsgAddr, MsgData;
    float *tempptr, MsgDataflt;
    int fd;
    volatile unsigned int *fpgabase;
    int sockfd, newsockfd;


    printf("Starting Rx Server\r\n");
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        printf("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

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
    * go in sleep mode and will wait for the incoming connection  */
    printf("Listening for Connection\r\n");
    if (listen(sockfd,5) == -1) {
	printf("Listen Failed\n");
	exit(1);
    }

    printf("Server listening on port %d...\n",PORT);

    /* Open /dev/mem for writing to FPGA register */
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (fd < 0)  {
      printf("Can't open /dev/mem\n");
      exit(1);
   }
   printf("Opened /dev/mem\r\n");

   fpgabase = (unsigned int *) mmap(0,255,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x43C00000);


   if (fpgabase == NULL) {
      printf("Can't mmap\n");
      exit(1);
   }
   else 
      printf("FPGA mmap'd\n");


reconnect:

    clilen = sizeof(cli_addr);
    printf("Waiting to accept connection...\n");
    /* Accept actual connection from the client */
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,&clilen);
    if (newsockfd < 0) { 
        printf("ERROR on accept");
         exit(1);
    }
    /* If connection is established then start communicating */
    printf("Connected Accepted...\n");

    while (1) {
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n <= 0) {
            printf("ERROR reading from socket\n");
            close(newsockfd);
	    goto reconnect; 
        }
        bufptr = (int *) buffer;
        printf("\nPacket %d Received : NumBytes = %d\n",++numpackets,n);
        printf("Header: %c%c \t",buffer[0],buffer[1]);
        printf("Message ID : %d\t",(ntohl(*bufptr++)&0xFFFF));
        printf("Body Length : %d\t",ntohl(*bufptr++));
        MsgAddr = ntohl(*bufptr++);
        printf("Msg Addr : %d\t",MsgAddr);
	MsgData = ntohl(*bufptr);
        printf("Data : %d\n",MsgData);
         
	//printf("Input0: %d %d %d %d\n",buffer[0],buffer[1],buffer[2],buffer[3]);
       	//printf("Input1: %d %d %d %d\n",buffer[4],buffer[5],buffer[6],buffer[7]);
       	//printf("Input2: %d %d %d %d\n",buffer[8],buffer[9],buffer[10],buffer[11]);
	//printf("Input3: %d %d %d %d\n",buffer[12],buffer[13],buffer[14],buffer[15]);
        tempbuf[0] = buffer[15];
	tempbuf[1] = buffer[14];
	tempbuf[2] = buffer[13];
	tempbuf[3] = buffer[12];
	tempptr = (float *) tempbuf;
        MsgDataflt = *tempptr; 

        switch(MsgAddr) {
		case SOFT_TRIG_MSG1:
		    printf("Soft Trigger Message:   Value=%d\n",MsgData);
                    soft_trig(fpgabase,MsgData);
		    break;

		case DMA_TRIG_SRC_MSG1:
		    printf("Set Trigger Source Message:   Value=%d\n",MsgData);
                    set_trigsrc(fpgabase,MsgData);
		    break;

		case PILOT_TONE_ENB_MSG1:
		    printf("Pilot Tone Enb Message:   Value=%d\n",MsgData);
                    set_pilottone_rfenb(fpgabase,MsgData);
		    break;

		case PILOT_TONE_SPI_MSG1:
		    printf("Pilot Tone SPI Message  Deprecated:   Value=%d\n",MsgData);
                    //prog_lmx2541(fpgabase,MsgData); 
		    break;

		case RF_ATTEN_MSG1:
		    printf("RF Attenuator Message:   Value=%d\n",MsgData);
                    set_atten(fpgabase,RFATTEN,MsgData); 
		    break;

		case PT_ATTEN_MSG1:
		    printf("PT Attenuator Message:   Value=%d\n",MsgData);
                    set_atten(fpgabase,PTATTEN,MsgData); 
		    break;

		case KX_MSG1:
		    printf("Kx Message:   Value=%d\n",MsgData);
                    set_kxky(fpgabase,HOR,MsgData); 
		    break;

		case KY_MSG1:
		    printf("Ky Message:   Value=%d\n",MsgData);
                    set_kxky(fpgabase,VERT,MsgData); 
		    break;

                case BBA_XOFF_MSG1:
		    printf("BBA X Offset:   Value=%d\n",MsgData);
	 	    set_bbaoffset(fpgabase,HOR,MsgData);
                    break;

                case BBA_YOFF_MSG1:
		    printf("BBA Y Offset:   Value=%d\n",MsgData);
		    set_bbaoffset(fpgabase,VERT,MsgData);
                    break; 

		case CHA_GAIN_MSG1:
		    printf("ChA Gain Message:   Value=%f\n",MsgDataflt);
                    set_gain(fpgabase,CHA,MsgDataflt); 
		    break;

		case CHB_GAIN_MSG1:
		    printf("ChA Gain Message:   Value=%f\n",MsgDataflt);
                    set_gain(fpgabase,CHB,MsgDataflt); 
		    break;

		case CHC_GAIN_MSG1:
		    printf("ChA Gain Message:   Value=%f\n",MsgDataflt);
                    set_gain(fpgabase,CHC,MsgDataflt); 
		    break;

		case CHD_GAIN_MSG1:
		    printf("ChA Gain Message:   Value=%f\n",MsgDataflt);
                    set_gain(fpgabase,CHD,MsgDataflt); 
		    break;

		case FINE_TRIG_DLY_MSG1:
		    printf("Fine Trig Delay Message:   Value=%d\n",MsgData);
                    set_geo_dly(fpgabase,MsgData); 
		    break;

		case COARSE_TRIG_DLY_MSG1:
		    printf("Coarse Trig Delay Message:   Value=%d\n",MsgData);
		    set_coarse_dly(fpgabase,MsgData);
		    break;

		case TRIGTOBEAM_THRESH_MSG1:
		    printf("Trigger to Beam Threshold Message:   Value=%d\n",MsgData);
		    set_trigtobeam_thresh(fpgabase,MsgData);
		    break;




		case EVENT_NO_MSG1:
		    printf("TbT Gate Width Message:   Value=%d\n",MsgData);
                    set_eventno(fpgabase,MsgData); 
		    break;

		case MACHINE_SEL_MSG1:
		    printf("Machine Location Message:   Value=%d\n",MsgData);
                    set_machineloc(fpgabase,MsgData); 
		    break;




                default:
		    printf("Msg not supported yet...\n");
		    break;
	    }

    
        }


    close(newsockfd);
    return NULL; 

}

