/*
 * firealarm.h
 *
 *  Created on: Mar 18, 2019
 *      Author: juanh
 */

#ifndef FIREALARM_H_
#define FIREALARM_H_

/* Initialize the ADC in PORT 4 PIN 7 to convert thermistor voltage to digital value */
void init_fireAlarm_ADC();

/* Initialize LCD in function set mode, 2-line mode, display on, cursor blink off, cursor off */
void init_LCD();

/* SPI write sends one byte at a time*/
void send_char_FA_SPI(char* data);

/* Changes the signal at the pin passed in */
void setPin(const int portPin, const int pinSignal);

/* Depending on the parameter passed in to the function, this will set the signals high/low to the pins
 * we need to pass the data to the LCD screen. For example:
 * MSP432 transmits a 0x05 to LCD screen it will prepare DB[7..4] as:
 * DB7 = 0
 * DB6 = 1
 * DB5 = 0
 * DB4 = 1   since we are using 4-bit mode (transmission is only 4-bits wide)*/
void LCD_transmit(char pinToUse);

/* Erase everything displayed in the screen */
void LCD_clearScreen();

/* LCD is a 2x16 matrix so cursor = index of where we want to write stuff. This function
 * moves the cursor to a specific position denoted by row and column in function parameters */
void LCD_setCursor(const char row, const char col);

/* Sends ONE character to LCD */
void LCD_char(char ch);

/* This function tells LCD we are going to write to screen following datasheet protocol
 * to start a transmission:
 *
 * 1) Drive RS pin low
 * 2) Transmit command using LCD_transmit()
 * 3) Drive EN high to start writing
 * 4) Wait 1ms for LCD to catch up
 * 5) Drive EN low to finish transmission
 * 6) Wait 1ms for LCD to catch up */
void LCD_command(char cmd);

/* Sends a string of characters to the LCD */
void LCD_string(char *arr);

/* Software delay for when needed */
void _delay_ms(int ms);

#endif /* FIREALARM_H_ */
