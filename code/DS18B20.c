#define _DS18B20_C
#include "pins.h"
#include "DS18B20.h"

//Delay 10us, input is how many times of 10us for instance x=5, delay 50us.
void delay10us(unsigned char x) {
	unsigned char i = 0;
	for(i=0;i<x;i++) {
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
	}
}

//Write one byte to DS18B20
void ds18b20_write_byte(unsigned char dat) {
	unsigned char i = 0;
	unsigned char tmp = 0;
	tmp = dat;
	EA = 0;										//Shut down the global interrupt, since DS18B20 need a strict time sequential.
	for (i=0;i<8;i++){
		DS18B20_IO = 0;							//Pull down IO 2us first.
		_nop_();
		_nop_();
		DS18B20_IO = tmp&0x01;					//Send one bit data.
		delay10us(6);
		DS18B20_IO = 1;							//Delay 60us for DS18B20 to read.
		tmp >>= 1;								//Send next bit.
	}
	EA = 1;
}


//Read one byte from DS18B20.
unsigned char ds18b20_read_byte() {
	unsigned char dat, i;
	EA = 0;										//Shut down the global interrupt, since DS18B20 need a strict time sequential.

	for (i = 0x01; i != 0; i<<=1) {
		DS18B20_IO = 0;							//Pull down IO 2us first.
		_nop_();
		_nop_();
		DS18B20_IO = 1;							//Pull up IO 2us first.
		_nop_();
		_nop_();
		if(DS18B20_IO)							//Read the value of DS18B20_IO
			dat = dat | i;						//If value equal to 1 set the corresponding bit of dat to 1.
		else
			dat = dat & (~i);					//If value equal to 0 set the corresponding bit of dat to 0.
		delay10us(6);
	}
	EA = 1;										//Open the global interrupt.
	return dat;
}

//Get the real time temperature from DS18B20, input is a pointer.
bit ds18b20_get_temp(int *temperature) {
	unsigned char low, high;
	bit acknowlegement = 1;

	//Get the acknowlegement of DS18B20, if there exist, we will get 0, if not we will get 1.
	EA = 0;
	DS18B20_IO = 0;
	delay10us(50);
	DS18B20_IO = 1;
	delay10us(6);
	acknowlegement = DS18B20_IO;
	while(!DS18B20_IO);
	EA = 1;

	if(acknowlegement == 0) {
		ds18b20_write_byte(0xCC);					//Write 0xCC to skip the rom operation.
		ds18b20_write_byte(0xBE);					//Write 0xBE to send the read operation.
		low = ds18b20_read_byte();					//Read the lsb of the temperature.
		high = ds18b20_read_byte();					//Read the msb of the temperature.
		*temperature = ((int)high << 8) + low;		//Return the value using the pointer, total 16 bits.
	}
	return ~acknowlegement;							//If get the value successfully, return 1, else return 0.
}

//Start the transition of DS18B20.
bit ds18b20_start_transition() {
	bit acknowlegement = 1;

	//Get the acknowlegement of DS18B20, if there exist, we will get 0, if not we will get 1.
	EA = 0;
	DS18B20_IO = 0;
	delay10us(50);
	DS18B20_IO = 1;
	delay10us(6);
	acknowlegement = DS18B20_IO;
	while(!DS18B20_IO);
	EA = 1;

	if(acknowlegement == 0) {
		ds18b20_write_byte(0xCC);					//Write 0xCC to skip the rom operation.
		ds18b20_write_byte(0x44);					//Start the temperature transition.
	}

	return ~acknowlegement;							//If get the value successfully, return 1, else return 0.
}
