/********************************************************************
*  Mem-Map
*  8-17-2016
*  Tony Caracappa
*  This program reads the shared memory space
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>



void prog_ad9510(void) 
{
    volatile unsigned int *fpgabase;
    int fd;

    /* Open /dev/mem for writing to FPGA register */
    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (fd < 0)  {
      printf("Can't open /dev/mem\n");
      exit(1);
   }

   fpgabase = (unsigned int *) mmap(0,255,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x43C00000);

   if (fpgabase == NULL) {
      printf("Can't mmap\n");
      exit(1);
   }

   fpgabase[2] = 0x0010;
   usleep(5000);

   fpgabase[2] = 0x5800;
   usleep(5000);

   fpgabase[2] = 0x4500;
   usleep(5000);

   fpgabase[2] = 0x3c08;
   usleep(5000);

   fpgabase[2] = 0x4b80;
   usleep(5000);

   fpgabase[2] = 0x3d08;
   usleep(5000);

   fpgabase[2] = 0x4980;
   usleep(5000);

   fpgabase[2] = 0x3e08;
   usleep(5000);

   fpgabase[2] = 0x4d80;
   usleep(5000);

   fpgabase[2] = 0x3f08;
   usleep(5000);

   fpgabase[2] = 0x4f80;
   usleep(5000);

   fpgabase[2] = 0x4002;
   usleep(5000);

   fpgabase[2] = 0x5180;
   usleep(5000);

   fpgabase[2] = 0x4102;
   usleep(5000);

   fpgabase[2] = 0x5380;
   usleep(5000);

   fpgabase[2] = 0x4202;
   usleep(5000);

   fpgabase[2] = 0x5580;
   usleep(5000);

   fpgabase[2] = 0x3800;
   usleep(5000);

   fpgabase[2] = 0x3900;
   usleep(5000);

   fpgabase[2] = 0x3a0a;
   usleep(5000);

   fpgabase[2] = 0x4302;
   usleep(5000);

   fpgabase[2] = 0x5780;
   usleep(5000);

   fpgabase[2] = 0x0400;
   usleep(5000);

   fpgabase[2] = 0x0501;
   usleep(5000);

   fpgabase[2] = 0x0636;
   usleep(5000);

   fpgabase[2] = 0x0877;
   usleep(5000);

   fpgabase[2] = 0x0900;
   usleep(5000);

   fpgabase[2] = 0x0a02;
   usleep(5000);

   fpgabase[2] = 0x0b00;
   usleep(5000);

   fpgabase[2] = 0x0c01;
   usleep(5000);

   fpgabase[2] = 0x0d00;
   usleep(5000);

   fpgabase[2] = 0x5a01;
   usleep(5000);

   close(fd);

}
