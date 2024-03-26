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
#include <sys/sysinfo.h>
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

int get_machineloc(volatile unsigned int *fpgabase) {
    
    int machine_loc;

    machine_loc = fpgabase[MACHINE_LOC_REG];
    switch (machine_loc) {
        case 0:
	    return STRAIGHT;
	    break;
	case 3:
	case 5:
	    return RING;
	    break;
    }
}




void ReadSFPStats(volatile unsigned int *fpgabase, char *msg) {
   
    int i, rawval; 
    float temp;

    //read SFP Temperatures
    fpgabase[SFP_I2C_CNTRL_REG] = 0x160;	
    usleep(5000);
    for (i=0;i<=5;i++) { 
       rawval = fpgabase[SFP0_RDBK+i];
       if (rawval != 0xFFFF) 
          temp = rawval / 256.0;
       else
          temp = 0;    
       memcpy(msg,&temp,sizeof(int));
       msg = msg + 4;
       //printf("SFP%d Temp = %f\n",i,temp);
       }

    //read SFP Voltages 
    fpgabase[SFP_I2C_CNTRL_REG] = 0x162;	
    usleep(5000);
    for (i=0;i<=5;i++) { 
       rawval = fpgabase[SFP0_RDBK+i];
       if (rawval != 0xFFFF) 
          temp = rawval / 10000.0;
       else
          temp = 0;    
       memcpy(msg,&temp,sizeof(int));
       msg = msg + 4;
       //printf("SFP%d Voltage = %f\n",i,temp);
       }

    //read SFP Laser Bias Currnt (mA) 
    fpgabase[SFP_I2C_CNTRL_REG] = 0x164;	
    usleep(5000);
    for (i=0;i<=5;i++) { 
       rawval = fpgabase[SFP0_RDBK+i];
       if (rawval != 0xFFFF) 
          temp = rawval / 2000.0;
       else
          temp = 0;    
       memcpy(msg,&temp,sizeof(int));
       msg = msg + 4;
       //printf("SFP%d TxBias = %f\n",i,temp);
       }

    //read SFP Transmit Power (mA) 
    fpgabase[SFP_I2C_CNTRL_REG] = 0x166;	
    usleep(5000);
    for (i=0;i<=5;i++) { 
       rawval = fpgabase[SFP0_RDBK+i];
       if (rawval != 0xFFFF) 
          temp = rawval / 10000.0;
       else
          temp = 0;    
       memcpy(msg,&temp,sizeof(int));
       msg = msg + 4;
       //printf("SFP%d TxPower = %f\n",i,temp);
       }

    //read SFP Receive Power (mA) 
    fpgabase[SFP_I2C_CNTRL_REG] = 0x168;	
    usleep(5000);
    for (i=0;i<=5;i++) { 
       rawval = fpgabase[SFP0_RDBK+i];
       if (rawval != 0xFFFF) 
          temp = rawval / 10000.0;
       else
          temp = 0;    
       memcpy(msg,&temp,sizeof(int));
       msg = msg + 4;
       //printf("SFP%d RxPower = %f\n",i,temp);
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


void ReadAtten(volatile unsigned int *fpgabase, int regaddr, char *msg) { 

    int attenraw; 

    attenraw = fpgabase[regaddr];
    attenraw = attenraw >> 2;  //atten register is 0.25dB / bit, but interface uses 1dB / bit
    memcpy(msg,&attenraw,sizeof(int));
}




void ReadReg(volatile unsigned int *fpgabase, int regaddr, char *msg) {

    int rawreg;

    rawreg = fpgabase[regaddr];
    memcpy(msg,&rawreg,sizeof(int));
    //if (regaddr == PLL_LOCKED_REG) 
    //  printf("PLL Reg %d = %d\n",regaddr,rawreg);
}  


void GetSysUptime(char *msg) {
    struct sysinfo info;
    int uptime;

    sysinfo(&info);
    uptime = (int) info.uptime;
    printf("System Uptime : %d\n",uptime);
    //uptime = 25262728;
    memcpy(msg,&uptime,sizeof(int));
}


void GetFPGADieTemp(char *msg) {

   FILE *fp; 
   char resp[50]; 
   float offset, raw, scale, temp; 

   fp = popen("cat /sys/bus/platform/drivers/xadc/f8007100.adc/iio\\:device0/in_temp0_offset", "r");
   sscanf(fgets(resp, sizeof(resp), fp),"%f",&offset);
   pclose(fp);
   fp = popen("cat /sys/bus/platform/drivers/xadc/f8007100.adc/iio\\:device0/in_temp0_raw", "r");
   sscanf(fgets(resp, sizeof(resp), fp),"%f",&raw);
   pclose(fp);
   fp = popen("cat /sys/bus/platform/drivers/xadc/f8007100.adc/iio\\:device0/in_temp0_scale", "r");
   sscanf(fgets(resp, sizeof(resp), fp),"%f",&scale);
   pclose(fp);
   temp = scale*(raw + offset)/1000.0;
   printf("FPGA Die Temp = %f\n",temp);

   memcpy(msg,&temp,sizeof(int));

}



void ReadSysInfo(volatile unsigned int *fpgabase, char *msg) {

    ReadReg(fpgabase,FPGA_VER_REG,&msg[FPGA_VER_MSG32]);
    ReadBrdTemp(fpgabase,TEMP_DFE0_REG,&msg[TEMP_DFESENSE0_MSG32]);
    ReadBrdTemp(fpgabase,TEMP_DFE1_REG,&msg[TEMP_DFESENSE1_MSG32]);
    ReadBrdTemp(fpgabase,TEMP_DFE2_REG,&msg[TEMP_DFESENSE2_MSG32]);
    ReadBrdTemp(fpgabase,TEMP_DFE3_REG,&msg[TEMP_DFESENSE3_MSG32]);
    ReadBrdTemp(fpgabase,TEMP_AFE0_REG,&msg[TEMP_AFESENSE0_MSG32]);
    ReadBrdTemp(fpgabase,TEMP_AFE1_REG,&msg[TEMP_AFESENSE1_MSG32]);

    ReadSFPStats(fpgabase,&msg[SFP_TEMP_MSG32]);
    GetSysUptime(&msg[SYS_UPTIME_MSG32]);
    GetPwrManagement(&msg[PWR_MON_MSG32]);
    GetFPGADieTemp(&msg[FPGA_DIETEMP_MSG32]);
   
}






void ReadGenRegs(volatile unsigned int *fpgabase, char *msg) {

    //Slow Control Data - PSC Message ID 30
    ReadReg(fpgabase,PLL_LOCKED_REG,&msg[PLL_LOCKED_MSG30]);
	
    ReadReg(fpgabase,KX_REG,&msg[POS_KX_MSG30]);
    ReadReg(fpgabase,KY_REG,&msg[POS_KY_MSG30]);

    ReadReg(fpgabase,BBA_XOFF_REG,&msg[BBA_XOFF_MSG30]);
    ReadReg(fpgabase,BBA_YOFF_REG,&msg[BBA_YOFF_MSG30]);
 
    ReadAtten(fpgabase,RF_DSA_REG,&msg[RF_ATTEN_MSG30]);
    ReadAtten(fpgabase,PT_DSA_REG,&msg[PT_ATTEN_MSG30]);

    ReadReg(fpgabase,PT_RFENB_REG,&msg[PT_RFENB_MSG30]);

    ReadReg(fpgabase,CHA_GAIN_REG,&msg[CHA_GAIN_MSG30]);
    ReadReg(fpgabase,CHB_GAIN_REG,&msg[CHB_GAIN_MSG30]);
    ReadReg(fpgabase,CHC_GAIN_REG,&msg[CHC_GAIN_MSG30]);
    ReadReg(fpgabase,CHD_GAIN_REG,&msg[CHD_GAIN_MSG30]);
    ReadReg(fpgabase,FINE_TRIG_DLY_REG,&msg[TBT_GATE_DLY_MSG30]);
    ReadReg(fpgabase,COARSE_TRIG_DLY_REG,&msg[COARSE_TRIG_DLY_MSG30]);

    ReadReg(fpgabase,DMA_TRIGCNT_REG,&msg[TRIG_DMA_CNT_MSG30]);

    ReadReg(fpgabase,EVR_DMA_TRIGNUM_REG,&msg[TRIG_EVENTNO_MSG30]);

    ReadReg(fpgabase, EVR_TS_NS_LAT_REG, &msg[TS_NS_LAT_MSG30]);
    ReadReg(fpgabase, EVR_TS_S_LAT_REG, &msg[TS_S_LAT_MSG30]);
    ReadReg(fpgabase, EVR_TS_NS_REG, &msg[TS_NS_MSG30]);
    ReadReg(fpgabase, EVR_TS_S_REG, &msg[TS_S_MSG30]);

    ReadReg(fpgabase, SA_TRIGNUM_REG, &msg[TRIG_SA_CNT_MSG30]);

    ReadReg(fpgabase, TRIGTOBEAM_THRESH_REG, &msg[TRIGTOBEAM_THRESH_MSG30]);
    ReadReg(fpgabase, TRIGTOBEAM_DLY_REG, &msg[TRIGTOBEAM_DLY_MSG30]);





}


void ReadPosRegs(volatile unsigned int *fpgabase, char *msg) {


    ReadReg(fpgabase, SA_CHA_REG, &msg[AMPL_ASA_MSG31]);
    ReadReg(fpgabase, SA_CHB_REG, &msg[AMPL_BSA_MSG31]);
    ReadReg(fpgabase, SA_CHC_REG, &msg[AMPL_CSA_MSG31]);
    ReadReg(fpgabase, SA_CHD_REG, &msg[AMPL_DSA_MSG31]);
    ReadReg(fpgabase, SA_XPOS_REG, &msg[POS_X_MSG31]);
    ReadReg(fpgabase, SA_YPOS_REG, &msg[POS_Y_MSG31]);
    ReadReg(fpgabase, SA_SUM_REG, &msg[AMPL_SUM_MSG31]);

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
    char msgid32_buf[1024];
    char msgid32_bufntoh[1024];
    int *msgid32_bufptr;
    
    struct sockaddr_in serv_addr, cli_addr;
    int i, n, loop=0, loc, satrigwait;
    int sa_cnt, sa_cnt_prev, trig_cnt, trig_cnt_prev;
    int update_posdata=0;
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

    //initialize the 10z status registers
    bzero(msgid30_buf,sizeof(msgid30_buf));
    msgid30_bufptr = (int *)msgid30_buf; 
    msgid30_buf[0] = 'P';
    msgid30_buf[1] = 'S';
    msgid30_buf[2] = 0;
    msgid30_buf[3] = (short int) MSGID30;
    *++msgid30_bufptr = htonl(MSGID30LEN);  //body length
   
    //initialize the Postion Val status registers
    bzero(msgid31_buf,sizeof(msgid31_buf));
    msgid31_bufptr = (int *)msgid31_buf; 
    msgid31_buf[0] = 'P';
    msgid31_buf[1] = 'S';
    msgid31_buf[2] = 0;
    msgid31_buf[3] = (short int) MSGID31;
    *++msgid31_bufptr = htonl(MSGID31LEN);  //body length

    //initialize the system info    
    bzero(msgid32_buf,sizeof(msgid32_buf));
    msgid32_bufptr = (int *)msgid32_buf;
    msgid32_buf[0] = 'P';
    msgid32_buf[1] = 'S';
    msgid32_buf[2] = 0;
    msgid32_buf[3] = (short int) MSGID32;
    *++msgid32_bufptr = htonl(MSGID32LEN);  //body length




    sa_cnt_prev = sa_cnt = 0;
    trig_cnt_prev = trig_cnt = 0;
    satrigwait = 0;   

    while (1) {
        printf("In main loop...\n");
 	usleep(100);
        	
	do {
	   satrigwait++;
	   sa_cnt = fpgabase[SA_TRIGNUM_REG];
	   //printf("SA CNT: %d    %d\n",sa_cnt, satrigwait);
	   usleep(1000);
	}
        while (sa_cnt_prev == sa_cnt); 
	//while ((sa_cnt_prev == sa_cnt) && (satrigwait < 500));
	satrigwait = 0;
        sa_cnt_prev = sa_cnt;
        
	//if straight section, update SA data on trigger only
	//otherwise update at 10Hz.
	loc = get_machineloc(fpgabase);
        //printf("Loc = %d\n",loc);
	if (loc == STRAIGHT) {
	  trig_cnt = fpgabase[DMA_TRIGCNT_REG];
 	  if (trig_cnt != trig_cnt_prev) {
            update_posdata = 1;
	    trig_cnt_prev = trig_cnt;
	  }
	  else 
	    update_posdata = 0;
        } 
	else
	    update_posdata = 1;
        
        //printf("Update Posdata = %d\n",update_posdata);
	if (update_posdata == 1) {
          //Read the 10Hz FPGA registers 	
          ReadPosRegs(fpgabase,&msgid31_buf[MSGHDRLEN]);
    
          //write 10Hz msg31 packet
          Host2NetworkConvStatus(msgid31_buf,msgid31_bufntoh);	
	  n = write(newsockfd,msgid31_bufntoh,MSGID31LEN+MSGHDRLEN);
          if (n < 0) {
            printf("Status socket: ERROR writing MSG 31 - Pos Info\n");
            close(newsockfd);
            goto reconnect;
          }

        }

        ReadGenRegs(fpgabase,&msgid30_buf[MSGHDRLEN]);
        //printf("Reading Gen Registers\n");
        //write 1Hz msg30 packet
	Host2NetworkConvStatus(msgid30_buf,msgid30_bufntoh);	
	n = write(newsockfd,msgid30_bufntoh,MSGID30LEN+MSGHDRLEN);
        if (n < 0) {
           printf("Status socket: ERROR writing MSG 30 - 1Hz Info\n");
           close(newsockfd);
           goto reconnect;
        }


	if ((loop % 10) == 0) {
          //printf("Reading Sys Info\n");
          ReadSysInfo(fpgabase,&msgid32_buf[MSGHDRLEN]);
	  Host2NetworkConvStatus(msgid32_buf,msgid32_bufntoh);	
	  //for(i=0;i<160;i=i+4) 
          //    printf("%d: %d  %d  %d  %d\n",i-8,msgid32_buf[i], msgid32_buf[i+1], 
          //		                msgid32_buf[i+2], msgid32_buf[i+3]);
          n = write(newsockfd,msgid32_bufntoh,MSGID32LEN+MSGHDRLEN);
          if (n < 0) {
            printf("Status socket: ERROR writing MSG 32 - System Info\n");
            close(newsockfd);
            goto reconnect;
          }
        }


     loop++;

    } 

    return 0;





}

