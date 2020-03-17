#ifndef _LCD1602_H
#define _LCD1602_H
#ifndef _LCD1602_C //External varaible defined at here.
#endif

//External function defined at here.
void lcd_write_cmd(unsigned char command);
void lcd_write_data(unsigned char dat);
void lcd_set_cursor(unsigned char x, unsigned char y);
void lcd_show_string(unsigned char x, unsigned char y, unsigned char *strs);
void lcd_clear_screen(void);
void lcd_close_cursor(void);
void lcd_open_cursor(void);
void lcd_ready(void);
void lcd_init(void);
void lcd_show_char(unsigned char x, unsigned char y, unsigned char tmp1);
#endif