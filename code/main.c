#include "pins.h"
#include "LCD1602.h"
#include "DS1302.h"
#include "DS18B20.h"
#include "EEPROM.h"


struct time_and_date SetTime;
//Infrared.
//--------------------------------------------------------------------------------------
bit ir_finish = 0; //Ir flag if recieved data, set to 1, else set to 0.
unsigned char ir_data[4]; //Data buffer since there are 4 bytes need to be recieved.
const uint8 code ir_map[][2] = {  //???????PC??????
    {0x45,0x00}, {0x46,0x00}, {0x47,0x1B}, //??->?  Mode->?   ??->ESC
    {0x44,0x00}, {0x40,0x25}, {0x43,0x27}, //??->?  ??->?? ??->??
    {0x07,0x00}, {0x15,0x28}, {0x09,0x26}, // EQ->?   ??->?? ??->??
    {0x16, '0'}, {0x19,0x1B}, {0x0D,0x0D}, //'0'->'0'  ??->ESC  U/SD->??
    {0x0C, '1'}, {0x18, '2'}, {0x5E, '3'}, //'1'->'1'  '2'->'2'   '3'->'3'
    {0x08, '4'}, {0x1C, '5'}, {0x5A, '6'}, //'4'->'4'  '5'->'5'   '6'->'6'
    {0x42, '7'}, {0x52, '8'}, {0x4A, '9'}, //'7'->'7'  '6'->'8'   '9'->'9'
};
//---------------------------------------------------------------------------------------

//Song
//---------------------------------------------------------------------------------------
code unsigned char FH[] = {0xF2,0xF3,0xF5,0xF5,0xF6,0xF7,0xF8,
0xF9,0xF9,0xFA,0xFA,0xFB,0xFB,0xFC,
0xFC,0xFC,0xFD,0xFD,0xFD,0xFD,0xFE,
0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFF};

code unsigned char FL[] = {
0x42, 0xC1, 0x17, 0xB6, 0xD0, 0xD1, 0xB6,
0x21, 0xE1, 0x8C, 0xD8, 0x68, 0xE9, 0x5B, 
0x8F,0xEE, 0x44, 0x6B, 0xB4, 0xF4, 0x2D,
0x47, 0x77, 0xA2, 0xB6, 0xDA, 0xFA, 0x16}; 

code unsigned char song_code[] = {6, 2, 3, 5, 2, 1, 3, 2, 2, 5, 2, 2, 1, 3, 2, 6, 2, 1, 5, 2, 1,6, 2, 4, 3, 2, 2, 5, 2, 1, 6, 2, 1, 5, 2, 2, 3, 2, 2, 1, 2, 1,6, 1, 1, 5, 2, 1, 3, 2, 1, 2, 2, 4, 2, 2, 3, 3, 2, 1, 5, 2, 2,
5, 2, 1, 6, 2, 1, 3, 2, 2, 2, 2, 2, 1, 2, 4, 5, 2, 3, 3, 2, 1,2, 2, 1, 1, 2, 1, 6, 1, 1, 1, 2, 1, 5, 1, 6, 0, 0, 0};

unsigned char song_timer1_high = 0;		//High eight bits for timer1.
unsigned char song_timer1_low = 0;		//Low eight bits for timer1.
unsigned char song_time = 1;

//Function defined at here.
void song_begin(void);
void song_delay(unsigned char t);
//---------------------------------------------------------------------------------------
	
void delayms(unsigned int itime);
void show_time(struct time_and_date *tim);
void show_temp();
void eeprom_store_time(struct time_and_date *tim);
void ir_key_serve(void);
void ir_init(void);

void delay200ms(void)
{
  	unsigned char i,j,k;
  	for(i=201;i>0;i--)
  	for(j=32;j>0;j--)
  	for(k=14;k>0;k--);
}


unsigned char psec;
void main(void) {
	//unsigned char buf[6];
	unsigned char i = 0;
	IT0=1;
	EX0=1;
	EA=1;	

	IR_DATA=1;
	lcd_init();
	ds1302_init();
	ds18b20_start_transition();
	ds1302_get_time(&SetTime);
	eeprom_store_time(&SetTime);
	delayms(30);
	RCAP2H=0x02; 
	RCAP2L=0x18;

	ET2=1;
	EA=1;
	TR2=1;
	ir_init();
	while(1){
		song_begin();
	}
}

void delayms(unsigned int itime){
	unsigned int i,j;
	for (i = 0;i<itime; i++)
		for (j = 0;j<1275;j++);
}

void show_time(struct time_and_date *tim)
{
    unsigned char str[18];
    
    str[0]  = ((tim->year>>4) & 0xF) + '0';  
    str[1]  = (tim->year & 0x0F) + '0';
    str[2]  = '-';
    str[3]  = (tim->month >> 4) + '0';   
    str[4]  = (tim->month & 0x0F) + '0';
    
    str[5]  = (tim->day >> 4) + '0';   
    str[6]  = (tim->day & 0x0F) + '0';
    str[7]  = ' ';
    str[8] = (tim->hour >> 4) + '0';  
    str[9] = (tim->hour & 0x0F) + '0';
    str[10] = ':';
    str[11] = (tim->minute >> 4) + '0';   
    str[12] = (tim->minute & 0x0F) + '0';
	  str[13] = ':';
	  str[14] = (tim->second>> 4) + '0';   
    str[15] = (tim->second & 0x0F) + '0';
    str[16] = '\0';
		
	  lcd_show_string(0, 0, "Temp:");  
    lcd_show_string(0, 1, str);
		
}

void show_temp() {
	int temper = 0;
	unsigned char str[5];
	unsigned char a,b;
	a = ds18b20_get_temp(&temper);
	b = ds18b20_start_transition();
	
	temper >>= 4;
	str[0] = (temper/10) + '0';
	str[1] = (temper%10) + '0';
	str[2] = '\'';
	str[3] = 'C';
	str[4] = '\0';
	lcd_show_string(0,0,"Temp:");
	lcd_show_string(8,0,str);
}

void Timer0Interrupt(void) interrupt 1
{
    //TH0 = 0x10;
    //TL0 = 0x00;
	  //ds1302_get_time(&SetTime);
		//if(psec != SetTime.second){
		//show_time();
    //add your code here!
}

void eeprom_store_time(struct time_and_date *tim) {
		unsigned char buffer[6];
		buffer[0] = 0x2018;
		buffer[1] = 0x11;
		buffer[2] = 0x27;
		buffer[3] = 0x12;
		buffer[4] = 0x00;
		buffer[5] = tim->second;
		eeprom_write_multi(buffer, 0x30, 6);
}

void Timer2_Server(void) interrupt 5 
{ 
	static unsigned char Timer2_Server_Count;
	RCAP2H=0x02; 
	RCAP2L=0x18;
	
	Timer2_Server_Count++; 
	if(Timer2_Server_Count==10)
	{ 
		Timer2_Server_Count=0; 
		TF2=0;
		ds1302_get_time(&SetTime);
	 ds1302_date_to_str (&SetTime);
	 ds1302_time_to_str(&SetTime);
	 lcd_show_string(0, 1, SetTime.DateString);
	 lcd_show_string(8, 1, SetTime.TimeString);
	 ir_key_serve();
	 show_temp();
		//lcd_show_string(14,0,"md");
	} 
}


//----------------------------------------------------------------------------------------------------------------------------------------
void ir_delay(unsigned int i)
{
	while(i--);	
}

void ir_key_serve(void){
		unsigned char i = 0;
		if(ir_finish){
			ir_finish = 0;
			for (i = 0; i < 21; i++) {
				if(ir_data[2] == ir_map[i][0]) {
					lcd_set_cursor(13,0);
					lcd_write_data(ir_map[i][1]);
					//lcd_show_string(0,13, ir_map[i][1]);
					break;
				}
			}
		}
}

//Init the infrared equipment.
void ir_init(void) {
	IR_DATA = 1;						//To prepare read the value.
	TMOD = 0x11;
	TH0 = 0x00;
  TL0 = 0x00;
  EA = 1;									//Open the global interrupt
  ET0 = 0;								//Forbid the timer0 interrupt
  TR0 = 0;								//Shut down the timer0.
	IT0 = 1;								//Open the external interrupt0
	EX0 = 1;								//Using the edge triger.
}

//Get the low voltage time.
unsigned int ir_get_high_time(){
	TH0 = 0;
	TL0 = 0;
	TR0 = 1;								//Open the timer 0
	while(IR_DATA) {				//Get the low time
		if (TH0 > 0X40) 			//To prevent the wrong condition break if takes too long.
			break;
	}
	TR0 = 0;								//Shut down the timer0.
	return (TH0*256 + TL0);
}

//Get the high voltage time.
unsigned int ir_get_low_time() {
	TH0 = 0;
	TL0 = 0;
	TR0 = 1;
	while(!IR_DATA){
		if (TH0 > 0x40)
			break;
	}
	TR0 = 0;
	return (TH0*256 + TL0);
}

//Interrupt0 serve function. Get the data.
void ir_get_data() interrupt 0 {
	unsigned char i, j = 0;
	unsigned char recieved_byte;
	unsigned int tmp;
	tmp = ir_get_low_time();
	if((tmp < 7000) || (tmp > 10000)) {						//Get the first 9ms
		IE0 = 0;
		return;
	}
	
	tmp = ir_get_high_time();											//Get the low 4.5ms
	if((tmp < 3500) || (tmp > 5000)) {
		IE0 = 0;
		return;
	}
	
	for(i = 0; i < 4; i++){
		for(j = 0;j < 8; j++) {
			tmp = ir_get_low_time();									//Get the 560us low voltage.
			if((tmp < 310) || (tmp > 720)) {
				IE0 = 0;
				return;
			}
			
			tmp = ir_get_high_time();
			if ((tmp > 310) && (tmp < 720)) {					//If the time between 1460us to 1900us. set 1 else set 0.
				recieved_byte >>= 1;
			} else if ((tmp > 1300)&&(tmp < 1800)) {
				recieved_byte >>= 1;
				recieved_byte |= 0x80;
			} else {																	//If not at that scope exist.
				IE0 = 0;
				return;
			}
		}
		ir_data[i] = recieved_byte;
	}
	ir_finish = 1;																//Finish recieve.
	IE0 = 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------------------------------------
void song_delay(unsigned char t) {
	unsigned char i;
	unsigned long j;
	for(i=0;i<t;i++)
		for(j=0;j<8000;j++);
	TR1 = 0;
}


void song_begin(void){
	  unsigned char i,m = 0;
	  i = 0;
	  m = 0;
		song_time = 1;
		TMOD = 0x11;
		ET1 = 1;
    EA = 1;
	  //BEEM = 0;
		while(song_time) {
			m = song_code[i] + 7*song_code[i+1] - 1;
			song_timer1_high = FH[m];
			song_timer1_low  = FL[m];
			song_time = song_code[i+2];
			i+=3;
			TH1 = song_timer1_high;
			TL1 = song_timer1_low;
			TR1 = 1;
			song_delay(song_time);
		}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------
void Timer1Interrupt(void) interrupt 3 //Timer1's interrupt.
{		
		TR1 = 0;
    BEEM = !BEEM;
		TH1 = song_timer1_high;
    TL1 = song_timer1_low;
		TR1 = 1;
}
