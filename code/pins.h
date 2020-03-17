#ifndef _PINS_H
#define _PINS_H

#include <reg52.h>
#include <intrins.h>
#include <ctype.h>
#include <stdio.h>

//Define pins, used in this clock project.

typedef  signed    char    int8;    
typedef  signed    int     int16;   
typedef  signed    long    int32;  
typedef  unsigned  char    uint8;   
typedef  unsigned  int     uint16;  
typedef  unsigned  long    uint32;  


//Define the DS1302
sbit DS_CK = P3^6; //The clock pin of DS1302
sbit DS_IO = P3^4;   //The input/output pin of DS1302(data pin)
sbit DS_CE = P3^5;   //The selected pin of DS1302

//Define the LCD 1602
sbit LCD_RS=P2^6;     //Select the data register(=1) or command register(=0)
sbit LCD_E=P2^7;    //Enable signal
sbit LCD_RW=P2^5;     //Read and write signal.
sbit LCD_BUSY=P0^7;      //Busy condition bit.
#define LCD_DATA P0  //Define the data port of LCD1602.

//Define the DS18B20
sbit DS18B20_IO = P3^7; //Define the data path of DS18B20.

//Define the EEPROM 24C02.
sbit SCL = P2^1;			//Clock siganl of I2C.
sbit SDA = P2^0;			//Data signal of I2C.

//Define the infrared IC. Using the NEC protocal.
sbit IR_DATA = P3^2;

//Define the loudspeaker.
sbit BEEM = P1^5;

#endif