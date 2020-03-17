#define _EEPROM_C
#include "pins.h"
#include "EEPROM.h"

//The delay function.
#define I2Cdelay() {_nop_();_nop_();_nop_();_nop_();}

//Procedure to initialize I2C
void startConfig(){
	SDA = 1;
	SCL = 1;
	I2Cdelay();
	SDA = 0;
	I2Cdelay();
	SCL = 0;
}

//Procedure to stop I2C
void stopConfig(){
	SCL = 0;
	SDA = 0;
	I2Cdelay();
	SCL = 1;
	I2Cdelay();
	SDA = 1;
	I2Cdelay();
}

bit writeI2C(unsigned char dat){
	bit a; //return the acknolegement.
	unsigned char mask = 0x80; //mask = b1000,0000
	while(mask !=0) {
		if ((mask&dat) == 0){ //Judge the present bit is 1
			SDA = 0;
		} else {
			SDA = 1;
		}
		mask >>= 1;
		I2Cdelay();
		SCL = 1;
		I2Cdelay(); //Transfer the data
		SCL = 0;
	}
	SDA = 1;
	I2Cdelay();
	SCL = 1;
	a = SDA;
	I2Cdelay();
	SCL = 0;
	return (~a);
}

//Read I2C from the bus
unsigned char readI2C(){
	unsigned char mask = 0x80; //masl value
	unsigned char d; //return data
	SDA = 1; //Make sure to release the bus.
	while(mask !=0) {
	    I2Cdelay();
		SCL = 1;
		if (SDA == 1){ //Judge the present bit is 1
			d = d | mask;
		} else {
			d &= ~mask;
		}
		mask >>= 1;
		I2Cdelay();
		SCL = 0;
	}
	SDA = 0; //Begin the ack
	I2Cdelay();
	SCL = 1;
	I2Cdelay();
	SCL = 0; //Finishing the ack.
	return d;
}

//Read the I2C without the ack
unsigned char readI2CnoACK(){
	unsigned char mask = 0x80; //masl value
	unsigned char d; //return data
	SDA = 1; //Make sure to release the bus.
	while(mask !=0) {
	    I2Cdelay();
		SCL = 1;
		if (SDA == 1){ //Judge the present bit is 1
			d = d | mask;
		} else {
			d &= ~mask;
		}
		mask >>= 1;
		I2Cdelay();
		SCL = 0;
	}
	SDA = 1; //SDA is high
	I2Cdelay();
	SCL = 1;
	I2Cdelay();
	SCL = 0; //So there is no ack.
	return d;
}

//Write one byte to EEPROM. Using I2C protocal.
void eeprom_write_byte(unsigned char addr, unsigned char dat) {
	startConfig();
	writeI2C(0xa0);
	writeI2C(addr);
	writeI2C(dat);
	stopConfig();
}

//Read one byte from EEPROM. Using I2C protocal.
unsigned char eeprom_read_byte(unsigned char addr) {
	unsigned char dat;
	startConfig();
	writeI2C(0xa0);
	writeI2C(addr);
	startConfig();
	writeI2C(0xa1);
	dat = readI2CnoACK();
	stopConfig();
	return dat;
}

//Read multi bytes from EEPROM. Using I2C protocal.
void eeprom_read_multi(unsigned char *buffer, unsigned char addr, unsigned char len) {
	do {
		startConfig();
		if (writeI2C(0xa0)) {
			break;
		}
		stopConfig();
	} while(1);
	writeI2C(addr);
	startConfig();
	writeI2C(0xa1);
	while(len > 1) {
		*buffer++ = readI2C();
		len--;
	
	}
	*buffer = readI2CnoACK();
	stopConfig();
}

//Write multi bytes to EEPROM, Using the page write method, this method is quiker. Using I2C protocal.
void eeprom_write_multi(unsigned char *buffer, unsigned char addr, unsigned char len) {
	while(len > 0) {
		do {
			startConfig();
			if (writeI2C(0xa0)) {
				break;
			}
			stopConfig();
		} while(1);
		writeI2C(addr);
		while(len > 0) {
			writeI2C(*buffer++);
			len--;
			addr++;
			if((addr&0x07) == 0) {
				break;
			}
		}
		stopConfig();
	}
}



