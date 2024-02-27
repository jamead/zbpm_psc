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
   fpgabase[2] = 0x80000010;
   fpgabase[2] = 0x0010;
   usleep(5000);

   fpgabase[2] = 0x5800;
   fpgabase[2] = 0x80005800;
   fpgabase[2] = 0x5800;
   usleep(5000);

   fpgabase[2] = 0x4500;
   fpgabase[2] = 0x80004500;
   fpgabase[2] = 0x4500;
   usleep(5000);

   fpgabase[2] = 0x3c08;
   fpgabase[2] = 0x80003c08;
   fpgabase[2] = 0x3c08;
   usleep(5000);

   fpgabase[2] = 0x4b80;
   fpgabase[2] = 0x80004b80;
   fpgabase[2] = 0x4b80;
   usleep(5000);

   fpgabase[2] = 0x3d08;
   fpgabase[2] = 0x80003d08;
   fpgabase[2] = 0x3d08;
   usleep(5000);

   fpgabase[2] = 0x4980;
   fpgabase[2] = 0x80004980;
   fpgabase[2] = 0x4980;
   usleep(5000);

   fpgabase[2] = 0x3e08;
   fpgabase[2] = 0x80003e08;
   fpgabase[2] = 0x3e08;
   usleep(5000);

   fpgabase[2] = 0x4d80;
   fpgabase[2] = 0x80004d80;
   fpgabase[2] = 0x4d80;
   usleep(5000);

   fpgabase[2] = 0x3f08;
   fpgabase[2] = 0x80003f08;
   fpgabase[2] = 0x3f08;
   usleep(5000);

   fpgabase[2] = 0x4f80;
   fpgabase[2] = 0x80004f80;
   fpgabase[2] = 0x4f80;
   usleep(5000);

   fpgabase[2] = 0x4002;
   fpgabase[2] = 0x80004002;
   fpgabase[2] = 0x4002;
   usleep(5000);

   fpgabase[2] = 0x5180;
   fpgabase[2] = 0x80005180;
   fpgabase[2] = 0x5180;
   usleep(5000);

   fpgabase[2] = 0x4102;
   fpgabase[2] = 0x80004102;
   fpgabase[2] = 0x4102;
   usleep(5000);

   fpgabase[2] = 0x5380;
   fpgabase[2] = 0x80005380;
   fpgabase[2] = 0x5380;
   usleep(5000);

   fpgabase[2] = 0x4202;
   fpgabase[2] = 0x80004202;
   fpgabase[2] = 0x4202;
   usleep(5000);

   fpgabase[2] = 0x5580;
   fpgabase[2] = 0x80005580;
   fpgabase[2] = 0x5580;
   usleep(5000);

   fpgabase[2] = 0x3800;
   fpgabase[2] = 0x80003800;
   fpgabase[2] = 0x3800;
   usleep(5000);

   fpgabase[2] = 0x3900;
   fpgabase[2] = 0x80003900;
   fpgabase[2] = 0x3900;
   usleep(5000);

   fpgabase[2] = 0x3a0a;
   fpgabase[2] = 0x80003a0a;
   fpgabase[2] = 0x3a0a;
   usleep(5000);

   fpgabase[2] = 0x4302;
   fpgabase[2] = 0x80004302;
   fpgabase[2] = 0x4302;
   usleep(5000);

   fpgabase[2] = 0x5780;
   fpgabase[2] = 0x80005780;
   fpgabase[2] = 0x5780;
   usleep(5000);

   fpgabase[2] = 0x0400;
   fpgabase[2] = 0x80000400;
   fpgabase[2] = 0x0400;
   usleep(5000);

   fpgabase[2] = 0x0501;
   fpgabase[2] = 0x80000501;
   fpgabase[2] = 0x0501;
   usleep(5000);

   fpgabase[2] = 0x0636;
   fpgabase[2] = 0x80000636;
   fpgabase[2] = 0x0636;
   usleep(5000);

   fpgabase[2] = 0x0877;
   fpgabase[2] = 0x80000877;
   fpgabase[2] = 0x0877;
   usleep(5000);

   fpgabase[2] = 0x0900;
   fpgabase[2] = 0x80000900;
   fpgabase[2] = 0x0900;
   usleep(5000);

   fpgabase[2] = 0x0a02;
   fpgabase[2] = 0x80000a02;
   fpgabase[2] = 0x0a02;
   usleep(5000);

   fpgabase[2] = 0x0b00;
   fpgabase[2] = 0x80000b00;
   fpgabase[2] = 0x0b00;
   usleep(5000);

   fpgabase[2] = 0x0c01;
   fpgabase[2] = 0x80000c01;
   fpgabase[2] = 0x0c01;
   usleep(5000);

   fpgabase[2] = 0x0d00;
   fpgabase[2] = 0x80000d00;
   fpgabase[2] = 0x0d00;
   usleep(5000);

   fpgabase[2] = 0x5a01;
   fpgabase[2] = 0x80005a01;
   fpgabase[2] = 0x5a01;
   usleep(5000);

   close(fd);

}
