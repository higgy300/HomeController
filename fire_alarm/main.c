/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432 Empty Project
 *
 * Description: An empty project that uses DriverLib. In this project, DriverLib
 * is built from source instead of the usual library.
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 * Author: 
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "firealarm.h"
#include "cc3100_usage.h"
//#include "uart_driver.h"
#include "simplelink.h"
#include "sl_common.h"
#include "_data_pack_.h"
#include "ClockSys.h"

//#define HUB_IP 0xC0A80102
#define RESOLUTION 16384
#define Vref 3.3
#define SAMPLE_LENGTH       128
#define OFFSET 0.14


#define D4 PORT10_PIN0
#define D5 PORT10_PIN1
#define D6 PORT10_PIN2
#define D7 PORT10_PIN3
#define RS PORT10_PIN4
#define EN PORT10_PIN5

// Global variables
static uint16_t ADC_result_buffer;
static uint32_t voltage_buffer;
bool transmit;
static int i;
float norm_voltage;
_controller_t ctrl_info;
_device_t alarm_info;
uint32_t FIREALARM_IP;
uint32_t HUB_IP;
_controller_t dummy_struc;

void floatToArray(float n, char *str, int decimal_places);
void reverse(char *str, int len);
int integerToString(int num, char str[], int decimal_places);

/** MAIN **/
void main(void) {

    /* Stop Watchdog  */
    WDT_A_clearTimer();
    WDT_A_holdTimer();

    ClockSys_SetMaxFreq();
    //Interrupt_enableSleepOnIsrExit();

    /* Enabling the FPU for floating point operation */
    FPU_enableModule();
    FPU_enableLazyStacking();

    /* Store the addresses of the wifi packet structures for reuse */
    //uint8_t *dummy_ptr = &dummy_struc;
    uint8_t *ctrl_ptr = &ctrl_info;
    uint8_t *alarm_ptr = &alarm_info;

    /* Initialize ADC buffers to 0 */
    ADC_result_buffer = 0;
    voltage_buffer = 0;
    i = 0;
    transmit = false;
    norm_voltage = 0.0;
    alarm_info.voltage = 0;
    alarm_info.fire_requesting = false;
    alarm_info.meter_requesting = false;
    alarm_info.curr1 = 0;
    alarm_info.curr2 = 0;
    alarm_info.fire_reading = 0;
    ctrl_info.fire_ack = false;
    ctrl_info.hub_req = false;
    ctrl_info.meter_ack = false;

    /* Initialize SPI to be used with WiFi */
    spi_Open(0,0);
    /* Initialize CC3100 device for WiFi capability */
    initCC3100(Client);
    FIREALARM_IP = getLocalIP();

    /*uint16_t x = 0;
    bool wifiConnected = false;
    while (!wifiConnected) {
        ReceiveData(ctrl_ptr, sizeof(ctrl_info));
        if (ctrl_info.hub_req) {
            wifiConnected = true;
            HUB_IP = ctrl_info.IPaddr;
            ctrl_info.fire_ack = true;
            ctrl_info.IPaddr = FIREALARM_IP;
        }
    }
    for (x = 0; x < 10; x++) {
        SendData(ctrl_ptr, HUB_IP, sizeof(ctrl_info));
        _delay(30);
        ReceiveData(ctrl_ptr, sizeof(ctrl_info));
        if (ctrl_info.hub_req) {
            break;
        }
    }*/
    HUB_IP = 0xC0A80102;

    /* Initialize ADC to measure Voltage */
    init_fireAlarm_ADC();

    init_LCD();

    //Initialize uart FOR DEBUGGING PURPOSES
    //uartInit();
    while(1) {
        LCD_setCursor(1,0);
        LCD_string("firealarm: ");


        char *voltage_str = malloc(10);
        norm_voltage = (float)(alarm_info.fire_reading * Vref) / (float)RESOLUTION;
        norm_voltage += OFFSET;
        floatToArray(norm_voltage, voltage_str, 3);
        LCD_setCursor(2,0);
        LCD_string(voltage_str);
        LCD_string(" V ");


        if (transmit) {
            SendData(alarm_ptr, HUB_IP, sizeof(alarm_info));
            alarm_info.fire_requesting = false;
            /*snprintf(test.txString, 70, "V = %d curr1 = %d curr2 = %d\r\n",
                     alarm_info.voltage, alarm_info.curr1, alarm_info.curr2);
            sendText();*/
            transmit = false;
            ADC14_enableConversion();
            Interrupt_enableInterrupt(INT_ADC14);
        }
        free(voltage_str);
    }
}

/* This interrupt is fired whenever a conversion is completed and placed in
 * ADC_MEM2. This signals the end of conversion and the results array is
 * grabbed and placed in resultsBuffer. */
void ADC14_IRQHandler(void) {
    uint64_t status;
    /* Returns the status of a the ADC interrupt register masked with the
     * enabled interrupts. When the ADC_MEM2 location finishes a conversion
     * cycle, the ADC_INT0 interrupt will be set. */
    status = ADC14_getEnabledInterruptStatus();

    /* When the ADC_MEM2 location finishes a conversion cycle, the ADC_INT2
     * interrupt will be set. */
    ADC14_clearInterruptFlag(status);

    // Compare the status and interrupt flag we are looking for
    if(status & ADC_INT0) {
        /* Returns the conversion results of Vin, I1, I2 and stores it in the
         * buffer. */
        ADC_result_buffer = ADC14_getResult(ADC_MEM0);

        if (i == SAMPLE_LENGTH) {
            Interrupt_disableInterrupt(INT_ADC14);
            ADC14_disableConversion();
            i = 0;
            voltage_buffer /= SAMPLE_LENGTH;
            transmit = true;
            alarm_info.fire_reading = voltage_buffer;
            alarm_info.fire_requesting = true;
            voltage_buffer = 0;
        }
        ++i;
        voltage_buffer += ADC_result_buffer;
    }
}

void floatToArray(float n, char *str, int decimal_places) {
    // Extract integer part
    int integer_part = (int)n;

    // Extract floating part
    float floating_part = n - (float)integer_part;

    // convert integer part to string
    int i = integerToString(integer_part, str, 0);

    // check for display option after point
    if (decimal_places != 0) {
        str[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 3.14159
        floating_part = floating_part * pow(10, decimal_places);

        integerToString((int)floating_part, str + i + 1, decimal_places);
    }
}

void reverse(char *str, int len) {
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

int integerToString(int num, char str[], int decimal_places) {
    int i = 0;

    while (num) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < decimal_places)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}
