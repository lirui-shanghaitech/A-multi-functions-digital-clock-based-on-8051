#define _DS1302_C
#include "pins.h"
#include "DS1302.h"


//Write one byte to DS1302.
void ds1302_write_byte(unsigned char reg, unsigned char dat){
	unsigned char i, addr; 	//Temp value.
	DS_CE = 0;
	_nop_();
	DS_CK = 0;
	_nop_();
	DS_CE = 1; 				//Select the DS1302
	_nop_();
	addr = reg<<1|0x80;     //Change the registor number to the real address.

	for (i=0;i<8;i++) {     //Send one byte address.
		DS_IO = addr&0x01;
		addr >>= 1;
		DS_CK = 1;
		_nop_();
		DS_CK = 0;
		_nop_();
	}

	for (i=0;i<8;i++) {     //Send one byte data
		DS_IO = dat&0x01;
		dat >>= 1;
		DS_CK = 1;
		_nop_();
		DS_CK = 0;
		_nop_();
	}
	DS_CE = 0;              //Release the DS1302.
	_nop_();
}

//Read one byte from DS1302.
unsigned char ds1302_read_byte(unsigned char reg){
	unsigned char addr, i;
	unsigned char dat1, dat = 0;
	
	DS_CE = 0;
	_nop_();
	DS_CK = 0;
	_nop_();
	DS_CE = 1; 				//Select the DS1302
	_nop_();
	addr = reg<<1|0x81;     //Change the registor number to the real address.
	
	for (i=0;i<8;i++) {     //First send one byte address.
		DS_IO = addr&0x01;
		addr >>= 1;
		DS_CK = 1;
		_nop_();
		DS_CK = 0;
		_nop_();
		
	}
	//DS_IO = 1;              //Release the data path.
	_nop_();
	for(i=0;i<8;i++){
		dat1 = DS_IO;
		dat = (dat>>1) | (dat1<<7);
		DS_CK = 1;
		_nop_();
		DS_CK = 0;
		_nop_();
	}
	DS_CE = 0;              //Release the DS1302.
	_nop_();
	DS_CK = 1;
	_nop_();
	DS_IO = 0;
	_nop_();
	DS_IO = 1;
	_nop_();
	//DS_IO = 1;
	return dat;             //Return the data.
}

//Burst write, since there are some special condition 
//the usual method may have mistake, this function write all data once for all.
void ds1302_burst_write(unsigned char *dat) {
	unsigned char i, j = 0;
	unsigned com = 0xBE;
	unsigned tmp = 0;
	
	DS_CE = 0;
	_nop_();
	DS_CK = 0;
	_nop_();
	DS_CE = 1;              //Select the DS1302.
	_nop_();
	for (i=0;i<8;i++) {     //Send one byte data, Write the burst write command.
		DS_IO = com&0x01;
		com >>= 1;
		DS_CK = 1;
		_nop_();
		DS_CK = 0;
		_nop_();
	}
	//DS_IO = 1;              //Release the data path.

	for(j = 0;j<8;j++) {
		tmp = dat[j];
		for (i=0;i<8;i++) { //Send one byte data
			DS_IO = tmp&0x01;
			tmp >>= 1;
			DS_CK = 1;
			_nop_();
			DS_CK = 0;
			_nop_();
		}
		_nop_();
		//DS_IO = 1;          //Release the data path.
	}
	//DS_CK = 0;
	//DS_CK = 1;
	DS_CE = 0;              //Release the DS1302.
	_nop_();
}

//Burst read, since there are some special condition 
//the usual method may have mistake, this function read data once for all.
void ds1302_burst_read(unsigned char *dat) {
	unsigned char i,j, temp = 0;
	unsigned char com = 0xBF;
	DS_CE = 0;
	_nop_();
	DS_CK = 0;
	_nop_();
	DS_CE = 1;              //Select the DS1302.
	_nop_();

	for (i=0;i<8;i++) {     //Send one byte data, Write the burst write command.
		DS_IO = com&0x01;
		com >>= 1;
		DS_CK = 1;
		_nop_();
		DS_CK = 0;
		_nop_();
	}
	//DS_IO = 1;              //Release the data path.
	_nop_();
	for(j=0;j<8;j++){
		for(i=0x01;i!=0;i<<=1){
			if(DS_IO != 0){
				temp |= i;
			}
			DS_CK = 1;
			_nop_();
			DS_CK = 0;
		}
		dat[j] = temp;
		temp = 0;
		_nop_();
	}
	
	DS_CE = 0;              //Release the DS1302.
	_nop_();
	DS_CK = 1;
	_nop_();
	DS_IO = 0;
	_nop_();
	DS_IO = 1;
	_nop_();
}

//Get the real time from DS1302. Return the result using tim pointer.
void ds1302_get_time(struct time_and_date *tim) {
	unsigned char buffer[8];
	ds1302_burst_read(buffer);
	tim->year   = buffer[6] + 0x2000;
	tim->month  = buffer[4];
	tim->day    = buffer[3];
	tim->hour   = buffer[2];
	tim->minute = buffer[1];
	tim->second = buffer[0];
	tim->week   = buffer[5];
	
	//tim->year   = ds1302_read_byte(6) + 0x2000;
	//tim->month  = ds1302_read_byte(4);
	//tim->day    = ds1302_read_byte(3);
	//tim->hour   = ds1302_read_byte(2);
	//tim->minute = ds1302_read_byte(1);
	//tim->second = ds1302_read_byte(0);
	//tim->week   = ds1302_read_byte(5);
}

//Set the real time to DS1302.
void ds1302_set_time(struct time_and_date *tim) {
	unsigned char buffer[8];
	buffer[0] = tim->second;
	buffer[1] = tim->minute;
	buffer[2] = tim->hour;
	buffer[3] = tim->day;
	buffer[4] = tim->month;
	buffer[5] = tim->week;
	buffer[6] = tim->year;
	buffer[7] = 0;
	ds1302_burst_write(buffer);
}

//DS1302 init function. Set the time at 2018,11,27, 12:00, week 2
void ds1302_init(void) {
	struct time_and_date code first_time[] = 
	{
		0x2018, 0x11, 0x27, 0x12, 0x00, 0x00, 0x02 
	};
	DS_CE = 0;
	DS_CK = 0;
	if((ds1302_read_byte(0) & 0x80)) {
		ds1302_write_byte(7, 0x00);
		ds1302_set_time(&first_time);
		//ds1302_write_byte(7, 0x80);
	}
}

//
void ds1302_date_to_str (struct time_and_date *Time)
{
	Time->DateString[0] = ((Time->year>>4) & 0xF) + '0'; 
	Time->DateString[1] = (Time->year & 0x0F) + '0';
	Time->DateString[2] = '-';
	Time->DateString[3] = (Time->month >> 4) + '0';   
	Time->DateString[4] = (Time->month & 0x0F) + '0';
	Time->DateString[5] = '-';
	Time->DateString[6] = (Time->day >> 4) + '0';   
	Time->DateString[7] = (Time->day & 0x0F) + '0';
	Time->DateString[8] = '\0';
}

//
void ds1302_time_to_str(struct time_and_date *Time)
{
	Time->TimeString[0] = (Time->hour >> 4) + '0';  
	Time->TimeString[1] =(Time->hour & 0x0F) + '0';
	Time->TimeString[2] = ':';
	Time->TimeString[3] = (Time->minute >> 4) + '0';
	Time->TimeString[4] = (Time->minute & 0x0F) + '0';
	Time->TimeString[5] = ':';
	Time->TimeString[6] = (Time->second>> 4) + '0';   
	Time->TimeString[7] = (Time->second & 0x0F) + '0';
	Time->TimeString[8] = '\0';
}