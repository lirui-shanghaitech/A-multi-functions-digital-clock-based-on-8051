#ifndef _EEPROM_H
#define _EEPROM_H

#ifndef _EEPROM_C
#endif
//I2C and EEPROM Operate function.
unsigned char eeprom_read_byte(unsigned char addr);
void eeprom_write_byte(unsigned char addr, unsigned char dat);
void eeprom_read_multi(unsigned char *buffer, unsigned char addr, unsigned char len);
void eeprom_write_multi(unsigned char *buffer, unsigned char addr, unsigned char len);
#endif