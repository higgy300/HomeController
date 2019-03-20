/*
 * firealarm.c
 *
 *  Created on: Mar 18, 2019
 *      Author: juanh
 */
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "firealarm.h"

#define SMCLK_FREQUENCY     12000000
#define SAMPLE_FREQUENCY    12000
#define SAMPLE_LENGTH       128

// Using PORT10[0..5] for LCD
#define PORT10_PIN0 0
#define PORT10_PIN1 1
#define PORT10_PIN2 2
#define PORT10_PIN3 3
#define PORT10_PIN4 4
#define PORT10_PIN5 5
#define PORT10_PIN6 6
#define PORT10_PIN7 7

#ifndef D0
#define D4 PORT10_PIN0
#define D5 PORT10_PIN1
#define D6 PORT10_PIN2
#define D7 PORT10_PIN3
#define RS PORT10_PIN4
#define EN PORT10_PIN5
#endif

#define LOW 0
#define HIGH 1

/* Timer_A Continuous Mode Configuration Parameter */
const Timer_A_UpModeConfig upModeConfig = {
TIMER_A_CLOCKSOURCE_SMCLK,            // SMCLK Clock Source
TIMER_A_CLOCKSOURCE_DIVIDER_1,       // SMCLK/1 = 12 MHz
12000,                  //  12 KHz sampling rate (1ms period)
TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer ISR
TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE, // Disable CCR0
TIMER_A_DO_CLEAR                     // Clear Counter
};

// Timer_A Compare Configuration Parameter
const Timer_A_CompareModeConfig compareConfig = {
TIMER_A_CAPTURECOMPARE_REGISTER_1,          // Use CCR1
TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
TIMER_A_OUTPUTMODE_SET_RESET,               // Toggle output but
12000          // 1ms Period
};

void init_fireAlarm_ADC() {
    ADC14_setResolution(ADC_14BIT);

    /* Turn on ADC module */
    ADC14_enableModule();
    /* Configure ADC to use master clock, no prescalers, and no routing */
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    /* Configuring GPIOs for Analog In P4.7 */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN7, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory 0 with repeat mode and internal 3.3V
     * reference */
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A6, ADC_NONDIFFERENTIAL_INPUTS);

    /* Configuring Timer_A in continuous mode and sourced from ACLK */
    Timer_A_configureUpMode(TIMER_A0_BASE, &upModeConfig);

    /* Configuring Timer_A0 in CCR1 to trigger at 16000 (0.5s) */
    Timer_A_initCompare(TIMER_A0_BASE, &compareConfig);

    /* Configuring the sample trigger to be sourced from Timer_A0  and setting it
     * to automatic iteration after it is triggered*/
    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    /* Enabling the interrupt when a conversion on channel 2 (end of sequence)
     * is complete and enabling conversions */
    ADC14_enableInterrupt(ADC_INT0);

    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    ADC14_enableConversion();

    /* Enable ADC Interrupt */
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    /* Starting the Timer */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}

void init_fireAlarm_LCD() {
    GPIO_setAsOutputPin(GPIO_PORT_P10, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 |
                        GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5);

    LCD_transmit(0x00);
    _delay_ms(20);
    LCD_command(0x03);
    _delay_ms(5);
    LCD_command(0x03);
    _delay_ms(5);
    LCD_command(0x03);
    _delay_ms(5);
    LCD_command(0x02);
    _delay_ms(1);

    // Function set:
    LCD_command(0x02);
    // 2-line mode
    LCD_command(0x08);
    // display on, blinker off, cursor off
    LCD_command(0x00);
    LCD_command(0x0C);
    // Return cursor home
    LCD_command(0x00);
    // Increment shifter to the right when writing
    LCD_command(0x06);
    LCD_clearScreen();
}

// Clears entire LCD screen
void LCD_clearScreen() {
    LCD_command(0x01);
}

void LCD_setCursor(const char row, const char col) {
    // Range for LCD cursor matrix is R: 1 -> 2  and C: 0 -> 15
    char temp,z,y;
    if(row == 1) {
        // MSB is high to indicate write to LCD with the DB[7..4] bit signals ready
        temp = 0x80 + col;
        // shift right 4 to adjust to 4-bit transmission mode (high byte)
        z = temp>>4;
        // Same as temp but now  adjusting to send other 4-bit transmission (low byte)
        y = (0x80+col) & 0x0F;
        // Send both z and y to LCD to set cursor at desired location
        LCD_command(z);
        LCD_command(y);
    } else if(row == 2) {
        temp = 0xC0 + col;
        z = temp>>4;
        y = (0xC0+col) & 0x0F;
        LCD_command(z);
        LCD_command(y);
    }
}

void LCD_command(char cmd) {
    // Drive RS low to tell LCD transmission is a LCD command
    setPin(RS,LOW);
    // set DB[7..4] as cmd binary signal
    LCD_transmit(cmd);
    // Drive ENABLE pin high to start writing to LCD
    setPin(EN,HIGH);
    // wait for LCD
    _delay_ms(1);
    // Drive ENABLE low to finish transmission
    setPin(EN,LOW);
    // Wait for LCD
    _delay_ms(1);
}

void LCD_transmit(char pinToUse) {
    if (pinToUse & GPIO_PIN0)
        setPin(D4,HIGH);
    else
        setPin(D4,LOW);

    if (pinToUse & GPIO_PIN1)
        setPin(D5,HIGH);
    else
        setPin(D5,LOW);

    if (pinToUse & GPIO_PIN2)
        setPin(D6,HIGH);
    else
        setPin(D6,LOW);

    if (pinToUse & GPIO_PIN3)
        setPin(D7,HIGH);
    else
        setPin(D7,LOW);
}

// To update LCD write functions depending on port pin passed in parameter
// y variable = 0 means to invert pin. y = 1 means set pin at position.
void setPin(const int portPin, const int pinSignal) {
    if (pinSignal == LOW) {
        // Look for the correct pin to invert
        if (portPin == PORT10_PIN0)
            P10->OUT &= ~(1<<PORT10_PIN0);
        else if (portPin == PORT10_PIN1)
            P10->OUT &= ~(1<<PORT10_PIN1);
        else if (portPin == PORT10_PIN2)
            P10->OUT &= ~(1<<PORT10_PIN2);
        else if (portPin == PORT10_PIN3)
            P10->OUT &= ~(1<<PORT10_PIN3);
        else if (portPin == PORT10_PIN4)
            P10->OUT &= ~(1<<PORT10_PIN4);
        else if (portPin == PORT10_PIN5)
            P10->OUT &= ~(1<<PORT10_PIN5);
    } else {
        // Look for the correct pin to set as high
        if (portPin == PORT10_PIN0)
            P10->OUT |= (1<<PORT10_PIN0);
        else if (portPin == PORT10_PIN1)
            P10->OUT |= (1<<PORT10_PIN1);
        else if (portPin == PORT10_PIN2)
            P10->OUT |= (1<<PORT10_PIN2);
        else if (portPin == PORT10_PIN3)
            P10->OUT |= (1<<PORT10_PIN3);
        else if (portPin == PORT10_PIN4)
            P10->OUT |= (1<<PORT10_PIN4);
        else if (portPin == PORT10_PIN5)
            P10->OUT |= (1<<PORT10_PIN5);
    }
}

void LCD_string(char *arr) {
    int i;
    for(i=0; arr[i] != '\0'; i++)
        LCD_char(arr[i]);
}

void LCD_char(char ch) {
    // Adjust to 4-bit data transmission
    char lowerNibble,upperNibble;
    lowerNibble = ch & 0x0F;
    upperNibble = (ch & 0xF0)>>4;

    // Drive RS high to send command
    setPin(RS,HIGH);
    // Send high byte first
    LCD_transmit(upperNibble);
    // Drive ENABLE high to start write to LCD
    setPin(EN,HIGH);
    _delay_ms(1);
    // Drive EN low to finish writing to LCD
    setPin(EN,LOW);
    _delay_ms(1);
    // Send low byte signals
    LCD_transmit(lowerNibble);
    // Drive ENABLE high to start writing to LCD
    setPin(EN,HIGH);
    _delay_ms(1);
    // Drive ENABLE low to finish transmission
    setPin(EN,LOW);
    _delay_ms(1);
}

void _delay_ms(int ms) {
    int i = 0, j = 0;

    for (j = 0; j < ms; j++) {
        for (i = 45000; i > 0; i--);
    }
}
