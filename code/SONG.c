#define _SONG_C
#include "pins.h"
#include "SONG.h"

//Using timer1 to play song.

void song_delay(unsigned char t) {
	unsigned char i,j;
	for(i=0;i<t;i++)
		for(j=0;j<8200;j++);
	TR1 = 1;
}

void song_begin(void){
		unsigned char song_time = 1;
	  unsigned char i,k = 0;
		TMOD = 0x10;
    TH1 = 0x00;
    TL1 = 0x00;
    EA = 1;
    ET1 = 1;
		while(song_time) {
			k = song_code[i] + 7*song_code[i+1] - 1;
			song_timer1_high = FH[k];
			song_timer1_low  = FL[k];
			song_time = song_code[i+2];
			i+=3;
			TH1 = song_timer1_high;
			TL1 = song_timer1_LOW;
			TR1 = 1;
			song_delay(song_time);
		}
}