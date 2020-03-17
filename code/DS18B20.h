#ifndef _DS18B20_H
#define _DS18B20_H

#ifndef _DS18B20_C
#endif

//Function define.(API)
bit ds18b20_get_temp(int *temperature);
bit ds18b20_start_transition();
void ds18b20_write_byte(unsigned char dat);
unsigned char ds18b20_read_byte();

#endif