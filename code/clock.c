#include "pins.h"
#include "LCD1602.h"
#include "DS1302.h"
#include "DS18B20.h"
#include "EEPROM.h"

//Sytem variable, time, state, flag.
//---------------------------
enum system_state {DEFAULT, SETTING};		//Define the system state.
enum system_state present_state;
unsigned char cursor_index; 				//Define the current setting index.
struct time_and_date SetTime; 				//Store the set time.
struct time_and_date CurTime;				//Store the current time.
bit mute_flag = 0;							//Mute flag, if mute set to 1 else set to 0.
//unsigned char present_minute;				//Store the current second, to sense the hour.
//unsigned char present_second;				//Store the curretn second, to sense the alarm.
int present_temper = 0;						//Store the current temperature, to sense the alarm.
bit uart_flag = 0;							//Make sure if uart start,if start set to 1, else set to 0.
bit rd_flag = 1;							//Test if send temperature or date. 0 send data, 1 send temp.
bit real_time = 1;
unsigned char temp_counter = 0;
//bit  forbid_song = 0;						//Since song and uart both use timer1 so when send the data we can not have alarm.
unsigned char uart_i= 0;					//Index of UART.
//unsigned char buf[1];
//---------------------------


//Infrared variable, only used in infrared module.
//---------------------------
bit ir_finish = 0; 							//Ir flag if recieved data, set to 1, else set to 0.
unsigned char ir_data[4]; 					//Data buffer since there are 4 bytes need to be recieved.
const uint8 code ir_map[][2] = {
    {0x45,'s'}, {0x46,'o'}, {0x47,'m'}, 		//shutdown  Mode   Mute
    {0x44,'u'}, {0x40,'l'}, {0x43,'r'}, 		//unknown   left   Right
    {0x07,'e'}, {0x15,'d'}, {0x09,'i'}, 		// EQ       Inc    Dec
    {0x16,'0'}, {0x19,'x'}, {0x0D,'y'}, 		//'0'->'0'  RPT    U/SD
    {0x0C,'1'}, {0x18,'2'}, {0x5E,'3'}, 		//'1'->'1'  '2'->'2'   '3'->'3'
    {0x08,'4'}, {0x1C,'5'}, {0x5A,'6'}, 		//'4'->'4'  '5'->'5'   '6'->'6'
    {0x42,'7'}, {0x52,'8'}, {0x4A,'9'}, 		//'7'->'7'  '6'->'8'   '9'->'9'
};
//---------------------------


//Varaible use for song module only.
//---------------------------
code unsigned char FH[] = {						//Initila value for timer1 high eight bits
	0xF2,0xF3,0xF5,0xF5,0xF6,0xF7,0xF8,			//Low frequency
	0xF9,0xF9,0xFA,0xFA,0xFB,0xFB,0xFC,			//Medium freqency.
	0xFC,0xFC,0xFD,0xFD,0xFD,0xFD,0xFE,			//High frequency.
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFF			//Super high freqency.
};

code unsigned char FL[] = {						//Initila value for timer1 low eight bits
	0x42, 0xC1, 0x17, 0xB6, 0xD0, 0xD1, 0xB6,	//Low frequency
	0x21, 0xE1, 0x8C, 0xD8, 0x68, 0xE9, 0x5B, 	//Medium freqency.
	0x8F,0xEE, 0x44, 0x6B, 0xB4, 0xF4, 0x2D,	//High frequency.
	0x47, 0x77, 0xA2, 0xB6, 0xDA, 0xFA, 0x16	//Super high freqency.
}; 

code unsigned char song_code[] = {				//The code of shi shang zhi you mama hao.
	6, 2, 3, 5, 2, 1, 3, 2, 2, 5, 
	2, 2, 1, 3, 2, 6, 2, 1, 5, 2, 
	1,6, 2, 4, 3, 2, 2, 5, 2, 1, 
	6, 2, 1, 5, 2, 2, 3, 2, 2, 1, 
	2, 1,6, 1, 1, 5, 2, 1, 3, 2, 
	1, 2, 2, 4, 2, 2, 3, 3, 2, 1,
	5, 2, 2,5, 2, 1, 6, 2, 1, 3, 
	2, 2, 2, 2, 2, 1, 2, 4, 5, 2, 
	3, 3, 2, 1,2, 2, 1, 1, 2, 1, 6,
	1, 1, 1, 2, 1, 5, 1, 6, 0, 0, 0
};

unsigned char song_timer1_high = 0;				//High eight bits for timer1.
unsigned char song_timer1_low = 0;				//Low eight bits for timer1.
unsigned char song_time = 1;
//-------------------------------

//Function definition.
void song_begin(void);
void song_delay(unsigned char t);

void show_time(struct time_and_date *tim);
void show_temp();
void eeprom_store_time(struct time_and_date *tim);
void ir_key_serve(void);
void ir_init(void);

void number_setting(unsigned char opcode);
void cursor_shift_right();
void cursor_shift_left();
void cancel_set();
void state_transition();
void key_act(unsigned char key_op);
void timer_config(void);


/***
Infrared module. Using timer0 to calculate the delay.
Using the external interrupt 0 to sense the input signal.
Here we need to change the mode of TMOD so we may change the 
mode of timer1 need to cauciouse. 

Global variable: ir_flag: check if finished recieve if done set to 1.
				 ir_data: store the recieved data, the third value is what we want.
***/
//-------------------------------------------------------------------------------------
void ir_delay(unsigned int i)
{
	while(i--);	
}

//Use the key serve function to change the state of 
void ir_key_serve(void){
		unsigned char i = 0;
		if(ir_finish){
			ir_finish = 0;
			for (i = 0; i < 21; i++) {
				if(ir_data[2] == ir_map[i][0]) {
					//lcd_set_cursor(13,0);
					//lcd_write_data(ir_map[i][1]);
					//lcd_show_string(0,13, ir_map[i][1]);
					key_act(ir_map[i][1]);
					break;
				}
			}
		}
}

//Init the infrared equipment.
void ir_init(void) {
	IR_DATA = 1;							//To prepare read the value.
	TMOD = 0x11;
	TH0 = 0x00;
  	TL0 = 0x00;
  	ET1 = 1;
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
	while(IR_DATA) {						//Get the low time
		if (TH0 > 0X40) 					//To prevent the wrong condition break if takes too long.
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
	if((tmp < 7000) || (tmp > 10000)) {					//Get the first 9ms
		IE0 = 0;
		return;
	}
	
	tmp = ir_get_high_time();							//Get the low 4.5ms
	if((tmp < 3500) || (tmp > 5000)) {
		IE0 = 0;
		return;
	}
	
	for(i = 0; i < 4; i++){
		for(j = 0;j < 8; j++) {
			tmp = ir_get_low_time();					//Get the 560us low voltage.
			if((tmp < 310) || (tmp > 720)) {
				IE0 = 0;
				return;
			}
			
			tmp = ir_get_high_time();
			if ((tmp > 310) && (tmp < 720)) {			//If the time between 1460us to 1900us. set 1 else set 0.
				recieved_byte >>= 1;
			} else if ((tmp > 1300)&&(tmp < 1800)) {
				recieved_byte >>= 1;
				recieved_byte |= 0x80;
			} else {									//If not at that scope exist.
				IE0 = 0;
				return;
			}
		}
		ir_data[i] = recieved_byte;
	}
	ir_finish = 1;										//Finish recieve.
	IE0 = 0;
}
//-------------------------------------------------------------------------------------

//Function for singing a song: shi shang zhi you mama hao.
//Here using timer1 to calculate delay.
//-------------------------------------------------------------------------------------
void song_delay(unsigned char t) {
	unsigned char i;
	unsigned long j;
	if ((mute_flag == 0)){
		for(i=0;i<t;i++)
			for(j=0;j<8000;j++);
		TR1 = 0;
	} else {
		TR1 = 0;
		return;
	}
	
}

void song_begin(void){
	  unsigned char i,m = 0;
	  
		//TMOD = 0x11;
		//ET1 = 1;
	  if((mute_flag == 0)) {											//If mute flag is 1, break the loop.
    	EA = 1;
			i = 0;
			m = 0;
			//TMOD = 0x11;
			song_time = 1;
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
		} else {
				return;
		}
}
//-------------------------------------------------------------------------------------


//Some interrupt functions.
//-------------------------------------------------------------------------------------
void Timer0Interrupt(void) interrupt 1		//Timer0's interrupt.
{

}

void Timer1Interrupt(void) interrupt 3 		//Timer1's interrupt.
{	
	if(uart_flag == 0) {
		TR1 = 0;
    	BEEM = !BEEM;
		TH1 = song_timer1_high;
    	TL1 = song_timer1_low;
		TR1 = 1;
	}
}

void interruptUART() interrupt 4{
	unsigned char a;
	if (RI) {
		RI = 0;		//Clear RI manually.
		a = SBUF;
		SBUF = a;
	} 
	if (TI) {
		TI = 0; //Clear TI manually
		if (real_time == 1) {
			SBUF = CurTime.TimeString[uart_i];
		} else {
			SBUF = SetTime.TimeString[uart_i];
		}
		uart_i++;
		if (uart_i >9)
			//SBUF = ' ';
			uart_i = 0;
	}
}

void Timer2_Server(void) interrupt 5 		//Timer2's interrupt.
{ 
	static unsigned char Timer2_Server_Count;
	RCAP2H= 0x3C; 
	RCAP2L= 0x0B0;
	
	Timer2_Server_Count++; 
	if(Timer2_Server_Count==10)
	{ 
		Timer2_Server_Count=0; 
		TF2=0;
		ir_key_serve();
		if (present_state == DEFAULT){
			lcd_close_cursor();
			ds1302_get_time(&CurTime);
	 		ds1302_date_to_str (&CurTime);
	 		ds1302_time_to_str(&CurTime);
	 		lcd_show_string(0, 1, CurTime.DateString);
	 		lcd_show_string(8, 1, CurTime.TimeString);
	 		lcd_set_cursor(11, 0);
			lcd_write_data((CurTime.week & 0x0F) + '0');
	 		if(rd_flag == 0 && uart_flag == 1){
	 			eeprom_write_multi(CurTime.TimeString, 0x00, 8);
	 		} else if (uart_flag == 1 && rd_flag == 1){
	 			eeprom_read_multi(SetTime.TimeString, 0x00, 8);
	 		}
		} 
	}
	temp_counter++;
	if (temp_counter == 20) {
		if (present_state == DEFAULT){
			show_temp();
		}
		temp_counter = 0;
	}
}
//-------------------------------------------------------------------------------------

//Display function. & Store value function.
//-------------------------------------------------------------------------------------
void show_temp() {
	int temper = 0;
	unsigned char str[5];
	unsigned char a,b;
	a = ds18b20_get_temp(&temper);
	b = ds18b20_start_transition();
	
	temper >>= 4;
	present_temper = temper;
	str[0] = (temper/10) + '0';
	str[1] = (temper%10) + '0';
	str[2] = '\'';
	str[3] = 'C';
	str[4] = '\0';
	lcd_show_string(0,0,"T:");
	lcd_show_string(2,0,str);
}

//Show some static string. So we don not need to refresh it.
void show_init() {
	lcd_clear_screen();
	lcd_show_string(0, 0, "T:");
	lcd_show_string(4, 0, "'C");
	lcd_show_string(7, 0, "week");
	lcd_close_cursor();	
	if(mute_flag == 1){
		lcd_show_string(13,0,"Mut");
	} else {
		lcd_show_string(13,0,"Dis");
	}
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
//-------------------------------------------------------------------------------------

//Main function some initialize.
//-------------------------------------------------------------------------------------
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
	//delayms(30);
	RCAP2H= 0x3C; 
	RCAP2L= 0x0B0;

	ET2=1;
	EA=1;
	TR2=1;
	ir_init();
	show_init();
	while(1){

		if (((CurTime.minute >> 4) == 0) && ((CurTime.minute & 0x0F) == 0) && ((CurTime.second >> 4) == 0) && ((CurTime.second & 0x0F) <= 1)) {
				if ((mute_flag == 0)) {
	 				song_begin();
	 			}
	 	} else if((present_temper/10>=2) && (present_temper%10>=8)){
	 		if((mute_flag == 0))
				song_begin();
	 	}
	}
}
//-------------------------------------------------------------------------------------

//Key action and state transition function.
//-------------------------------------------------------------------------------------

//Acting when dectect key was pressed.
void key_act(unsigned char key_op) {
	if((key_op >= '0') && (key_op <= '9')) {
		number_setting(key_op);
	} else if (key_op == 'r') {									//Set cursor right, when right key was pressed.
		cursor_shift_right();
	} else if(key_op == 'l') {									//Set cursor left, when left key was pressed
		cursor_shift_left();
	} else if(key_op == 'o') {									//Change the state of the system. when mode key was pressed.
		state_transition();
	} else if(key_op == 'm') {									//Change the state to mute state, if mute key was pressed.
		if (present_state == DEFAULT) {							//If the current state is default state just mute the song.
			mute_flag = ~mute_flag;
			if(mute_flag == 1){
				lcd_show_string(13,0,"Mut");
			} else {
				lcd_show_string(13,0,"Dis");
			}
		} else {
			cancel_set();										//else cancel the current setting.
		}
	} else if(key_op == 'y') {
		uart_flag = ~uart_flag;
		//mute_flag = 1;
		if(uart_flag == 1) {
			lcd_show_string(13,0,"UAR");
		} else if (mute_flag == 1){
			lcd_show_string(13,0,"Mut");
		} else if(mute_flag == 0) {
			lcd_show_string(13,0,"Dis");
		}
		timer_config();
	} else if (key_op == 'x') {
		if (rd_flag == 0 && uart_flag == 1) {
			lcd_show_string(13,0,"EWR");
		} else if (rd_flag == 1 && uart_flag == 1) {
			lcd_show_string(13,0,"ERD");
		}
		rd_flag = ~rd_flag;
	} else if (key_op == 'e') {
		real_time = ~real_time;
	}
}

void state_transition() {
	if (present_state == DEFAULT) {
		present_state = SETTING; 								//Change the current state to setting time state.
		SetTime.hour = CurTime.hour;							//Store the current time to buffer.
		SetTime.minute = CurTime.minute;
		SetTime.second = CurTime.second;
		SetTime.week   = CurTime.week;
		SetTime.year   = CurTime.year;
		SetTime.month  = CurTime.month;
		SetTime.day    = CurTime.day;
		lcd_show_string(13,0,"SET");
		cursor_index = 20;										//Set the cursor when user press the key it will set at most left location
		cursor_shift_right();
		lcd_open_cursor();										//Open the cursor.

	} else if(present_state == SETTING) {
		present_state = DEFAULT;								//Set the present state to the default state.
		ds1302_set_time(&SetTime);								//Give the setting time to ds1302. Updata the time.
		lcd_close_cursor();
		if(mute_flag == 1){
			lcd_show_string(13,0,"Mut");
		} else {
			lcd_show_string(13,0,"Dis");
		}
	}
}

//Cancel the current setting,if user press the mute key at setting state.
void cancel_set() {
	present_state = DEFAULT;
	lcd_close_cursor();
	lcd_clear_screen();
	lcd_show_string(7, 0, "week");
	ds1302_date_to_str (&CurTime);							//Write the setting time to lcd.
	ds1302_time_to_str(&CurTime);
	lcd_show_string(0, 1, CurTime.DateString);
	lcd_show_string(8, 1, CurTime.TimeString);
	lcd_set_cursor(11, 0);
	lcd_write_data((CurTime.week & 0x0F) + '0');
	if(mute_flag == 1){
			lcd_show_string(13,0,"Mut");
	} else {
			lcd_show_string(13,0,"Dis");
	}
	show_temp();
}

void timer_config(void) {
	if(uart_flag) {
		TMOD = 0x21; 										//Timer1 8 auto, Timer0,16 bit timer.
		SCON = 0x50;
		PCON = 0x00;
    	TH1 = 0xE6;
    	TL1 = TH1;
   		EA = 1;
   		ET1 = 0;
    	ES = 1;
    	TR1 = 1;
	} else {
		TMOD = 0X11;
		TH1 = 0;
		TL1 = 0;
		EA = 1;
		ET1 = 1;
		ES = 0;
		TR1 = 0;
	}
}
//-------------------------------------------------------------------------------------


//Set operation function, set number, shift left, shift right.
//-------------------------------------------------------------------------------------
void cursor_shift_right() {
	if(present_state == SETTING) {
		//lcd_open_cursor();
		switch (cursor_index) {
			case 0: cursor_index = 1; lcd_set_cursor(0,1); break;	//Year second position
			case 1: cursor_index = 2; lcd_set_cursor(1,1); break;	//Month first position
			case 2: cursor_index = 3; lcd_set_cursor(3,1); break; 	//Month first position
			case 3: cursor_index = 4; lcd_set_cursor(4,1); break; 	//Day first position
			case 4: cursor_index = 5; lcd_set_cursor(6,1); break; 	//Day second position.
			case 5: cursor_index = 6; lcd_set_cursor(7,1); break; 	//Hour first position.
			case 6: cursor_index = 7; lcd_set_cursor(8,1); break; 	//Hour second position.
			case 7: cursor_index = 8; lcd_set_cursor(9,1); break; 	//Minute first position.
			case 8: cursor_index = 9; lcd_set_cursor(11,1); break; 	//Minute second position.
			case 9: cursor_index = 10; lcd_set_cursor(12,1); break; //Second first position.
			case 10: cursor_index = 11; lcd_set_cursor(14,1); break;//Second second position.
			case 11: cursor_index = 12; lcd_set_cursor(15,1); break;//Week position.
			default: cursor_index = 0; lcd_set_cursor(11,0); break;  //Back to begin.
		}
	}
}

void cursor_shift_left() {
	//lcd_open_cursor();
	if(present_state == SETTING) {
		switch (cursor_index) {
			case 0: cursor_index = 12; lcd_set_cursor(15,1); break;
			case 1: cursor_index = 0; lcd_set_cursor(11,0); break;
			case 2: cursor_index = 1; lcd_set_cursor(0,1); break;
			case 3: cursor_index = 2; lcd_set_cursor(1,1); break;
			case 4: cursor_index = 3; lcd_set_cursor(3,1); break;
			case 5: cursor_index = 4; lcd_set_cursor(4,1); break;
			case 6: cursor_index = 5; lcd_set_cursor(6,1); break;
			case 7: cursor_index = 6; lcd_set_cursor(7,1); break;
			case 8: cursor_index = 7; lcd_set_cursor(8,1); break;
			case 9: cursor_index = 8; lcd_set_cursor(9,1); break;
			case 10: cursor_index = 9; lcd_set_cursor(11,1); break;
			case 11: cursor_index = 10; lcd_set_cursor(12,1); break;
			default: cursor_index = 11; lcd_set_cursor(14,1); break;
		}
	}
}
				

void number_setting(unsigned char opcode) {
	unsigned char numb = 0;
	numb = opcode - '0';
	//lcd_open_cursor();
	if(numb <= 9) {
		if (present_state == SETTING) {								//Only change when we are at setting mode.
			switch (cursor_index) {
				case 0:  {
					if (numb>7)
						numb = 7;
					SetTime.week = (SetTime.week&0xF0)|(numb);
					lcd_show_char(11,0,opcode);
					break;
				};
				case 1: SetTime.year = (SetTime.year&0xFF0F)|(numb << 4); lcd_show_char(0,1,opcode); break;
				case 2:	SetTime.year = (SetTime.year&0xFFF0)|(numb); lcd_show_char(1,1,opcode); break;
				case 3: SetTime.month = (SetTime.month&0x0F)|(numb<<4); lcd_show_char(3,1,opcode); break;
				case 4: SetTime.month = (SetTime.month&0xF0)|(numb); lcd_show_char(4,1,opcode); break;
				case 5: SetTime.day = (SetTime.day&0x0F)|(numb<<4); lcd_show_char(6,1,opcode); break;
				case 6: SetTime.day = (SetTime.day&0xF0)|(numb); lcd_show_char(7,1,opcode); break;
				case 7:	SetTime.hour = (SetTime.hour&0x0F)|(numb<<4); lcd_show_char(8,1,opcode); break;
				case 8: SetTime.hour = (SetTime.hour&0xF0)|(numb); lcd_show_char(9,1,opcode); break;
				case 9: SetTime.minute = (SetTime.minute&0x0F)|(numb<<4); lcd_show_char(11,1,opcode); break;
				case 10: SetTime.minute = (SetTime.minute&0xF0)|(numb); lcd_show_char(12,1,opcode); break;
				case 11: SetTime.second = (SetTime.second&0x0F)|(numb<<4); lcd_show_char(14,1,opcode); break;
				default: SetTime.second = (SetTime.second&0xF0)|(numb); lcd_show_char(15,1,opcode); break;
			}
		}
		cursor_shift_right();										//Shift to right automatically.
	}
}
//-------------------------------------------------------------------------------------


