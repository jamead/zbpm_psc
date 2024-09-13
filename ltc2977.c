/********************************************************************
*  Power Manager Readbacks for ZYNQ-based RF BPM
*  6-27-2018
*  By: Anthony Caracappa
*  This subroutine is called by an IOC for the 1 Hz Readbacks... 
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <math.h>
#include <linux/i2c-dev.h>
#include <sys/stat.h>
#include <errno.h>
#include <i2c/smbus.h>

#include "zbpm_defs.h"
#include "zbpm_msg.h"



float L16_to_float(int input_val)
{
    // fixed exponent of -13 on LTC2977
    int exponent = -13;
    int mantissa = input_val;
    // compute value as mantissa * 2^(exponent)
    return mantissa * pow(2,exponent);
}

float L11_to_float(int input_val)
{
    // extract exponent as MS 5 bits
    int exponent = input_val >> 11;
    // extract mantissa as LS 11 bits
    int mantissa = input_val & 0x7ff;
    // sign extend exponent from 5 to 8 bits
    if( exponent > 0x0F ) exponent = (exponent|0xE0)-256; 
    // ints are 32 bits so using subtraction to adjust for negative numbers
    // sign extend mantissa from 11 to 16 bits
    if( mantissa > 0x03FF ) mantissa = (mantissa|0xF800)-65536;
    // compute value as mantissa * 2^(exponent)
    return mantissa * pow(2,exponent);
}



void GetPwrManagement(char *msg)
{
   int file;
   char filename[20];
  
   snprintf(filename, 19, "/dev/i2c-0");
   file = open(filename, O_RDWR);
   if (file < 0){
      printf("Could not open /dev/i2c-0)");
   }
   int addr = 0x5b; /* The I2C address */

   if (ioctl(file, I2C_SLAVE, addr) < 0){
      printf("Could not access i2c bus"); 
   }
   
   int res;
   short stats;
   float temp;

   //Read Controller Temperature
   res = i2c_smbus_read_word_data(file, 0x8d);
   temp = L11_to_float(res);
   printf("Temperature  = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   //Read Vin
   res = i2c_smbus_read_word_data(file, 0x88);
   temp = L11_to_float(res);
   printf("Vin = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   // Page 0 - 1.0V 
   i2c_smbus_write_byte_data(file, 0x00, 0 );
   res = i2c_smbus_read_word_data(file, 0x8b);
   stats = i2c_smbus_read_word_data(file, 0x79);
   if(stats<0) stats = i2c_smbus_read_word_data(file, 0x79);
   temp = L16_to_float(res);
   if(temp>7.9){
      usleep(20000);
      res = i2c_smbus_read_word_data(file, 0x8b);
      temp = L16_to_float(res);
   }
   printf("1.0V = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   // Page 1 - 1.0v current
   i2c_smbus_write_byte_data(file, 0x00, 1 );
   res = i2c_smbus_read_word_data(file, 0x8b);
   stats = i2c_smbus_read_word_data(file, 0x79);
   if(stats<0) stats = i2c_smbus_read_word_data(file, 0x79);
   temp = 100*L11_to_float(res);
   if(temp<0){
      usleep(20000);
      res = i2c_smbus_read_word_data(file, 0x8b);
      temp = 100*L11_to_float(res);
   }
   printf("1.0V Current = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   // Page 2  1.8V
   i2c_smbus_write_byte_data(file, 0x00, 2 );
   res = i2c_smbus_read_word_data(file, 0x8b);
   stats = i2c_smbus_read_word_data(file, 0x79);
   if(stats<0) stats = i2c_smbus_read_word_data(file, 0x79);
   temp = L16_to_float(res);
   if(temp>7.9){
      usleep(20000);
      res = i2c_smbus_read_word_data(file, 0x8b);
      temp = L16_to_float(res);
   }
   printf("1.8V = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   // Page 3  3.3V
   i2c_smbus_write_byte_data(file, 0x00, 3 );
   res = i2c_smbus_read_word_data(file, 0x8b);
   stats = i2c_smbus_read_word_data(file, 0x79);
   if(stats<0) stats = i2c_smbus_read_word_data(file, 0x79);
   temp = L16_to_float(res);
   if(temp>7.9){
      usleep(20000);
      res = i2c_smbus_read_word_data(file, 0x8b);
      temp = L16_to_float(res);
   }
   printf("3.3V = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   // Page 4  5V
   i2c_smbus_write_byte_data(file, 0x00, 4 );
   res = i2c_smbus_read_word_data(file, 0x8b);
   stats = i2c_smbus_read_word_data(file, 0x79);
   if(stats<0) stats = i2c_smbus_read_word_data(file, 0x79);
   temp = L16_to_float(res);
   if(temp>7.9){
      usleep(20000);
      res = i2c_smbus_read_word_data(file, 0x8b);
      temp = L16_to_float(res);
   }
   printf("5V = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   // Page 5 MGT - AVTT 
   i2c_smbus_write_byte_data(file, 0x00, 5 );
   res = i2c_smbus_read_word_data(file, 0x8b);
   stats = i2c_smbus_read_word_data(file, 0x79);
   if(stats<0) stats = i2c_smbus_read_word_data(file, 0x79);
   temp = L16_to_float(res);
   if(temp>7.9){
      usleep(20000);
      res = i2c_smbus_read_word_data(file, 0x8b);
      temp = L16_to_float(res);
   }
   printf("MGT AVTT = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   // Page 6 MGT AVCC
   i2c_smbus_write_byte_data(file, 0x00, 6 );
   res = i2c_smbus_read_word_data(file, 0x8b);
   stats = i2c_smbus_read_word_data(file, 0x79);
   if(stats<0) stats = i2c_smbus_read_word_data(file, 0x79);
   temp = L16_to_float(res);
   if(temp>7.9){
      usleep(20000);
      res = i2c_smbus_read_word_data(file, 0x8b);
      temp = L16_to_float(res);
   }
   printf("MGT AVCC = %f\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;


   // Page 7 Board Current
   i2c_smbus_write_byte_data(file, 0x00, 7 );
   res = i2c_smbus_read_word_data(file, 0x8b);
   stats = i2c_smbus_read_word_data(file, 0x79);
   if(stats<0) stats = i2c_smbus_read_word_data(file, 0x79);
   temp = 100*L11_to_float(res);
   if(temp<0){
      usleep(20000);
      res = i2c_smbus_read_word_data(file, 0x8b);
      temp = 100*L11_to_float(res);
   }
   printf("Brd Current  = %f\n\n",temp);
   memcpy(msg,&temp,sizeof(int));
   msg = msg + 4;



   close(file);

}

