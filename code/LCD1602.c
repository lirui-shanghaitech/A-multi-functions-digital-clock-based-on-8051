#define _LCD1602_C
#include "pins.h"
#include "LCD1602.h"

void Delay(unsigned char x)
{
	unsigned int i,j;
	for(i=x;i>0;i--)
	{
		for(j=120;j>0;j--)
		{}
	}
}

//Check if LCD is ready, assistant function.
void lcd_ready(void){
	unsigned char a;
	P0 = 0xff;    //To read the busy pin, set 1 first
	LCD_RS = 0;  
	LCD_RW = 1;
	do {
		LCD_E = 1;  //Need a low to high signal.
		a = LCD_DATA;   //Read the condition bit.
		//Delay(5);
		LCD_E = 0;
	} while (a&0x80);//Stuck at here untill LCD ready.
}

//Write a commond to LCD. Exposed to outside.
void lcd_write_cmd(unsigned char command){
	lcd_ready();
	LCD_RS = 0;
	LCD_RW = 0;
	LCD_E = 0;
	LCD_DATA = command;
	LCD_E = 1;
	_nop_();
	_nop_();
	LCD_E = 0;
}

//Write data to LCD. Exposed to outside.
void lcd_write_data(unsigned char dat) {
	lcd_ready();
	LCD_RS = 1;
	LCD_RW = 0;
	LCD_E = 0;
	LCD_DATA = dat;
	LCD_E = 1;
	_nop_();
	_nop_();
	LCD_E = 0;
}

//Set the cursor of LCD.Set the cursor at (x,y)
void lcd_set_cursor(unsigned char x, unsigned char y){
	unsigned char ad = 0;
	if(y == 0){
		ad = 0x00 + x;
	} else {
		ad = 0x40 + x;
	}
	lcd_write_cmd(ad | 0x80);
}

//Show string at the LCD where start at (x,y)
void lcd_show_string(unsigned char x, unsigned char y, unsigned char *strs){
	lcd_set_cursor(x, y);
	while (*strs != '\0') {     //If do not reach the end of string, write data. 
		lcd_write_data(*strs++);
		//strs++;
	}
}

//Clear the whole LCD.
void lcd_clear_screen(void) {
	lcd_write_cmd(0x01);
}

//Close the cursor of LCD.
void lcd_close_cursor(void) {
	lcd_write_cmd(0x0C);
}

//Open the cursor of LCD.
void lcd_open_cursor(void) {
	lcd_write_cmd(0x0F);
}

//Init the LCD
void lcd_init(void){
	lcd_write_cmd(0x38); //16*2 dispaly, 5*7 dot matrix, 8 parralel data path.
	lcd_write_cmd(0x0C); //Open the LCD, close the cursor.
	lcd_write_cmd(0x06); //Char stay, address increment automatically.
	lcd_write_cmd(0x01); //Clear the LCD.
}

//Show one char to lcd
void lcd_show_char(unsigned char x, unsigned char y, unsigned char tmp1) {
	lcd_set_cursor(x, y);
	lcd_write_data(tmp1);
}