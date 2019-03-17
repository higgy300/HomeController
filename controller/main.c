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
 * MSP432 Shwifty Project
 *
 * Description: Please let me graduate huehuehuehue
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

/* WiFi libraries */
#include "cc3100_usage.h"
#include "simplelink.h"
#include "sl_common.h"
#include "_data_pack_.h"
#include "ClockSys.h"

/* Sensor libraries */
#include "bme280_support.h"
#include "bme280.h"
#include "i2c_driver.h"
#include "uart_driver.h"
#include "typedef.h"

// Global variables
_controller_t ctrl_info;
_device_t meter_info;
uint32_t HUB_IP;
uint32_t METER_IP = 0xC0A80103;
//uint32_t FIRE_IP;
#define RESOLUTION 16384
#define Vref 3.3
// BME280
s32 returnRslt;
s32 g_s32ActualTemp   = 0;
u32 g_u32ActualPress  = 0;
u32 g_u32ActualHumity = 0;
//Receive UART Variables
#define NUM_RX_CHARS 64
char rxMsgData[NUM_RX_CHARS] = "";
int numMsgsRx = 0;
int tempIndex = 5;
int numChars = 0;

float norm_press;
float norm_temp;
float norm_hum;

/* Function Prototypes */
void convertMeterReadings(char*, float);
void convertEnvironmentReadings();
void reverse(char*, int);
int integerToString(int, char[], int);
void floatToArray(float, char*, int);

/** MAIN **/
void main(void) {
    /* Stop Watchdog  */
    WDT_A_clearTimer();
    WDT_A_holdTimer();

    ClockSys_SetMaxFreq();
    //Interrupt_enableSleepOnIsrExit();

    //Initialize uart
    uartInit();

    /* Enabling the FPU for floating point operation */
    FPU_enableModule();
    FPU_enableLazyStacking();

    /* Store the addresses of the wifi packet structures for reuse */
    uint8_t *ctrl_ptr = &ctrl_info;
    uint8_t *meter_ptr = &meter_info;

    /* Initialize SPI to be used with WiFi */
    //spi_Open(0,0);
    snprintf(test.txString, 70,
             "Initializing Wifi\r\n");
    sendText();
    /* Initialize CC3100 device for WiFi capability */
    initCC3100(Client);
    HUB_IP = getLocalIP();
    snprintf(test.txString, 70,
             "WiFi initialized!\r\n");
    sendText();

    meter_info.voltage = 0;
    meter_info.fire_requesting = false;
    meter_info.meter_requesting = false;
    meter_info.curr1 = 0;
    meter_info.curr2 = 0;
    meter_info.fire_reading = 0;
    ctrl_info.ctrl_acknowledged_fire = false;
    ctrl_info.ctrl_acknowledged_meter = false;
    float norm_voltage = 0.0;
    float norm_curr1 = 0.0;
    float norm_curr2 = 0.0;

    snprintf(test.txString, 70,
             "Waiting for meter connection.\r\n");
    sendText();
    uint32_t count = 0;

    // Initialize I2C
    initI2C();

    // Initialize bme280 sensor
    bme280_data_readout_template();
    returnRslt = bme280_set_power_mode(BME280_NORMAL_MODE);

    while(1) {
        ReceiveData(meter_ptr, sizeof(meter_info));
        if (meter_info.meter_requesting) {
            ++count;

            // Create temporary dynamic arrays for the float to string conversion values
            char *final_press = malloc(10);
            char *final_temp = malloc(10);
            char *final_hum = malloc(10);
            char *norm_voltage_string = malloc(10);
            char *norm_curr1_string = malloc(10);
            char *norm_curr2_string = malloc(10);

            // Convert the energy meter readings to actual readings
            norm_voltage = (float)(meter_info.voltage * Vref) / (float)RESOLUTION;
            norm_curr1 = (float)(meter_info.curr1 * Vref) / (float)RESOLUTION;
            norm_curr2 = (float)(meter_info.curr2 * Vref) / (float)RESOLUTION;
            // Convert the float values to string
            floatToArray(norm_voltage, norm_voltage_string, 2);
            floatToArray(norm_curr1, norm_curr1_string, 2);
            floatToArray(norm_curr2, norm_curr2_string, 2);

            // Read the BME280 data
            returnRslt = bme280_read_pressure_temperature_humidity(&g_u32ActualPress,
                                                                   &g_s32ActualTemp,
                                                                   &g_u32ActualHumity);
            // Convert the sensor data to float values
            convertEnvironmentReadings(g_u32ActualPress, g_s32ActualTemp, g_u32ActualHumity);
            // Convert float values to string
            floatToArray(norm_press, final_press, 2);
            floatToArray(norm_temp, final_temp, 2);
            floatToArray(norm_hum, final_hum, 2);

            // Send results to putty
            snprintf(test.txString, 70, "#%d voltage:  %s curr1:  %s curr2:  %s\r\n",
                     count, norm_voltage_string, norm_curr1_string, norm_curr2_string);
            sendText();
            snprintf(test.txString, 70, "humidity: %s %%rH pressure: %s kPa temperature: %s C\r\n",
                     final_hum, final_press, final_temp);
            sendText();

            // Change this boolean to prevent unwanted repeated data
            meter_info.meter_requesting = false;

            // Deallocate temporary dynamic arrays
            free(final_press);
            free(final_temp);
            free(final_hum);
            free(norm_voltage_string);
            free(norm_curr1_string);
            free(norm_curr2_string);
        }
    }
}

void convertMeterReadings(char* str, float ADC) {
    int num0 = (int)ADC;
    float temp = 10 * (ADC - num0);
    int num1 = (int)temp;
    float temp1 = 10 * (temp - num1);
    int num2 = (int)temp1;
    str[0] = num0 + 48;
    str[1] = '.';
    str[2] = num1 + 48;
    str[3] = num2 + 48;
    str[4] = '\0';
}

void convertEnvironmentReadings(s32 press, u32 temp, u32 hum) {
    norm_press = (float)press / 1000.0;
    norm_temp = (float)temp / 100.0;
    norm_hum = (float)hum / 1000.0;
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
