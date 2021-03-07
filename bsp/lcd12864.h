#ifndef _MLCD_H_
#define _MLCD_H_
void MLCD_GPIO_Init(void);
void MLCD_Init(void);
void MLCD_Fill(unsigned char data);
void LCD_P6x8Str(unsigned char x,unsigned char y,unsigned char ch[]);
void LCD_P8x16Str(unsigned char x,unsigned char y,unsigned char ch[]);
void LCD_NP8x16Str(unsigned char x,unsigned char y,unsigned char ch[]);
void Lcd12864_Print(unsigned char row,unsigned char column,unsigned char width,unsigned char height,char reverse_flag,const unsigned char* add);
#endif
