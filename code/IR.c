#define _IR_C
#include "pins.h"
#include "IR.h"

struct time_and_date SetTime;
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


//Init the infrared equipment.
void ir_init(void) {
	IR_DATA = 1;						//To prepare read the value.
	TMOD = 0x01;
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
	while(!IR_DATA) {				//Get the low time
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
	while(IR_DATA){
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
	if((tmp < 7800) || (tmp > 8700)) {						//Get the first 9ms
		IE0 = 0;
		return;
	}
	
	tmp = ir_get_high_time();											//Get the low 4.5ms
	if((tmp < 3600) || (tmp > 4600)) {
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
			} else if ((tmp > 1340)&&(tmp < 1750)) {
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

