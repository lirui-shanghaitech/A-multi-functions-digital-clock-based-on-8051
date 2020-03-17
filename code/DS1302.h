#ifndef _DS1302_H
#define _DS1302_H

//Struct to store the time of date of the whole program.
struct time_and_date {
	unsigned int year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char week;
	unsigned char DateString[9];
	unsigned char TimeString[9];
	
};

#ifndef _DS1302_C
#endif

//Function define in the DS1302.C, API of DS1302.
void ds1302_write_byte(unsigned char reg, unsigned char dat);
unsigned char ds1302_read_byte(unsigned char reg);
void ds1302_burst_write(unsigned char *dat);
void ds1302_burst_read(unsigned char *dat);
void ds1302_get_time(struct time_and_date *tim);
void ds1302_set_time(struct time_and_date *tim);
void ds1302_init(void);
void ds1302_date_to_str (struct time_and_date *Time);
void ds1302_time_to_str(struct time_and_date *Time);
#endif
