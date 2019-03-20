/*
 * firealarm.h
 *
 *  Created on: Mar 18, 2019
 *      Author: juanh
 */

#ifndef FIREALARM_H_
#define FIREALARM_H_

void init_fireAlarm_ADC();
void init_fireAlarm_LCD();
void send_char_FA_SPI(char* data);
void setPin(const int portPin, const int pinSignal);
void LCD_transmit(char pinToUse);
void LCD_clearScreen();
void LCD_setCursor(const char row, const char col);
void LCD_char(char ch);
void LCD_command(char cmd);
void LCD_string(char *arr);
void _delay_ms(int ms);

#endif /* FIREALARM_H_ */
