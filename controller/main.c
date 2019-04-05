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

#include "LCDLib.h"

// Global variables
_controller_t ctrl_info;
_device_t meter_info;
uint32_t HUB_IP;
uint32_t METER_IP = 0xC0A80103;
uint32_t FIRE_IP;
#define RESOLUTION 16384
#define Vref 3.3
#define OFFSET1 0.07
#define OFFSET2 0.18

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
float norm_voltage = 0.0;
float norm_curr1 = 0.0;
float norm_curr2 = 0.0;
bool cleared;

/* Function Prototypes */
void IP_stringConvert();
void convertMeterReadings(char*, float);
void convertEnvironmentReadings();
void reverse(char*, int);
int integerToString(int, char[], int);
void floatToArray(float, char*, int);
void init_WiFi();
void config_WiFi_strucs();
void init_sensors();
void init_systemConfig();
void init_LCD();
void printMainMenu();
void printEnergyMenu();

/** MAIN **/
void main(void) {
    // Stop watchdog timer, change clock, initialize FPU
    init_systemConfig();

    // Initialize the LCD driver
    init_LCD();

    // Store the addresses of the wifi packet structures for reuse
    uint8_t *ctrl_ptr = &ctrl_info;
    uint8_t *meter_ptr = &meter_info;

    // Initialize SPI to be used with WiFi
    config_WiFi_strucs();
    init_WiFi();

    // Initialize environmental sensors
    init_sensors();
    _delay(300);

    // Print the main menu
    //printMainMenu();
    printEnergyMenu();

    while(1) {
        ReceiveData(meter_ptr, sizeof(meter_info));
        if (meter_info.meter_requesting && !cleared) {
            cleared = true;
            LCD_Clear(LCD_WHITE);
            LCD_Text(40, 5, "TEAM SPARK by Juan H. & Mateo P.", LCD_BLUE);
            LCD_Text(5, 22, "Environmental Measurements:", LCD_BLACK);
            LCD_Text(20, 40, "Humidity:          %rH", LCD_BLACK);
            LCD_Text(20, 60, "Pressure:          kPa", LCD_BLACK);
            LCD_Text(20, 80, "Temperature:       C", LCD_BLACK);
            LCD_Text(5, 120, "Energy Measurements:", LCD_BLACK);
            LCD_Text(20, 140, "Voltage:              V", LCD_BLACK);
            LCD_Text(20, 160, "Current              mA", LCD_BLACK);
            LCD_Text(20, 180, "Theft Current:       mA", LCD_BLACK);
        } else if (meter_info.meter_requesting && cleared) {
            // Create temporary dynamic arrays for the float to string conversion values
            char *final_press = malloc(10);
            char *final_temp = malloc(10);
            char *final_hum = malloc(10);
            char *norm_voltage_string = malloc(10);
            char *norm_curr1_string = malloc(10);
            char *norm_curr2_string = malloc(10);

            // Convert the energy meter readings to actual readings
            norm_voltage = (float)(meter_info.curr2 * Vref) / (float)RESOLUTION;
            norm_curr1 = (float)(meter_info.curr1 * Vref) / (float)RESOLUTION;
            norm_curr2 = (float)(meter_info.voltage * Vref) / (float)RESOLUTION;

            // Apply ADC offset
            norm_voltage -= OFFSET2;
            if (norm_voltage < 0)
                norm_voltage = 0.0;
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
            LCD_DrawRectangle(111, 165, 40, 75, LCD_WHITE);
            LCD_DrawRectangle(116, 165, 80, 95, LCD_WHITE);
            LCD_DrawRectangle(130, 180, 140, 175, LCD_WHITE);
            LCD_DrawRectangle(135, 180, 178, 200, LCD_WHITE);
            LCD_Text(121, 40, final_hum, LCD_BLACK);
            LCD_Text(113, 60, final_press, LCD_BLACK);
            LCD_Text(121, 80, final_temp, LCD_BLACK);
            LCD_Text(141, 140, norm_voltage_string, LCD_BLACK);
            LCD_Text(141, 160, norm_curr1_string, LCD_BLACK);
            LCD_Text(141, 180, norm_curr2_string, LCD_BLACK);

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

void IP_stringConvert() {
    uint16_t shftAmnt = 24;
    uint32_t arr[4];
    char outputString[17];
    int i = 0, x = 50, y = 58, index = 0, j = 0;

    LCD_Text(25, 105, "IP: ", LCD_WHITE);

    // Split the 4 byte number into a single byte array
    for (i = 0; i < 4; i++) {
        arr[i] = (HUB_IP >> shftAmnt) & 0xFF;
        shftAmnt -= 8;
    }

    i = 0;
    // For each single byte, convert the byte to a string. Once converted,
    // Add each character to the output string
    for (i = 0; i < 4; i++) {
        // Make a temporary string
        char* temp = malloc(10);
        // Convert the integer to string and retrieve number of digits in string
        int numOfDigits = integerToString((int)arr[i], temp, 0);

        // Copy the contents of the integer to string function to the output string
        for (j = 0; j < numOfDigits; j++)
            outputString[index++] = temp[j];
        outputString[index++] = '.';
        // Deallocate temporary string memory
        free(temp);
    }
    // Attach null-terminating character to string and print
    outputString[index - 1] = '\0';
    LCD_Text(55, 105, outputString, LCD_WHITE);
}

void init_WiFi() {
    LCD_Text(5, 45, "Initializing Wifi...", LCD_WHITE);
    spi_Open(0,0);
    LCD_Text(25, 65, "SPI channel opened", LCD_WHITE);
    initCC3100(Client);
    LCD_Text(25, 85, "WiFi driver configured", LCD_WHITE);
    HUB_IP = getLocalIP();
    IP_stringConvert();
    LCD_Text(25, 125, "Wifi connected successfully!", LCD_WHITE);
}

void config_WiFi_strucs() {
    LCD_Text(5, 5, "initializing WiFi params...", LCD_WHITE);
    meter_info.voltage = 0;
    meter_info.fire_requesting = false;
    meter_info.meter_requesting = false;
    meter_info.curr1 = 0;
    meter_info.curr2 = 0;
    meter_info.fire_reading = 0;
    ctrl_info.ctrl_acknowledged_fire = false;
    ctrl_info.ctrl_acknowledged_meter = false;
    cleared = false;
    norm_voltage = 0.0;
    norm_curr1 = 0.0;
    norm_curr2 = 0.0;
    LCD_Text(25, 25, "WiFi params initialized!", LCD_WHITE);
}

void printEnergyMenu() {
    LCD_DrawRectangle(0, 320, 0, 17, LCD_WHITE);
    LCD_Text(2, 1, "Energy Meter Section", LCD_BLACK);
    LCD_DrawRectangle(0, 320, 16, 17, LCD_BLACK);
    LCD_DrawRectangle(0, 320, 17, 240, LCD_YELLOW);
    LCD_Text(15, 25, "Energy Measurements:", LCD_BLACK);
    LCD_Text(30, 45, "Voltage:", LCD_BLACK);
    LCD_Text(30, 65, "Current:", LCD_BLACK);
    LCD_Text(30, 85, "Theft Detection:", LCD_BLACK);
    LCD_DrawRectangle(75, 215, 175, 220, LCD_BLACK);
    LCD_Text(120, 190, "Return", LCD_WHITE);
}

void printMainMenu() {
    LCD_Clear(LCD_WHITE);
    LCD_Text(2, 1, "Home Controller", LCD_BLACK);
    LCD_DrawRectangle(0, 320, 16, 17, LCD_BLACK);
    LCD_DrawRectangle(157, 161, 17, 240, LCD_BLACK);
    LCD_DrawRectangle(0, 320, 122, 126, LCD_BLACK);
    LCD_DrawRectangle(0, 157, 17, 122, LCD_YELLOW);
    LCD_DrawRectangle(161, 320, 17, 122, LCD_BLUE);
    LCD_DrawRectangle(0, 157, 126, 240, LCD_CYAN);
    LCD_DrawRectangle(161, 320, 126, 240, LCD_RED);
    LCD_Text(18, 55, "Energy Meter", LCD_BLACK);
    LCD_Text(190, 55, "Air Quality", LCD_WHITE);
    LCD_Text(20, 172, "Thermostat", LCD_BLACK);
    LCD_Text(190, 172, "Fire Alarm", LCD_BLACK);
}

void init_sensors() {
    LCD_Text(5, 145, "Initializing Sensors...", LCD_WHITE);
    // Initialize I2C
    initI2C();
    LCD_Text(25, 165, "I2C channel opened", LCD_WHITE);

    // Initialize bme280 sensor
    bme280_data_readout_template();
    returnRslt = bme280_set_power_mode(BME280_NORMAL_MODE);
    LCD_Text(25, 185, "Sensors initialized...", LCD_WHITE);
}

void init_systemConfig() {
    // Stop Watchdog
    WDT_A_clearTimer();
    WDT_A_holdTimer();

    // Change CPU clock to 48 MHz
    ClockSys_SetMaxFreq();

    // Enabling the FPU for floating point operation
    FPU_enableModule();
    FPU_enableLazyStacking();
}

void init_LCD() {
    LCD_Init(true);
    LCD_Clear(LCD_BLUE);
    LCD_Text(65, 45, "Home Controller System", LCD_ORANGE);
    LCD_Text(130, 70, "by", LCD_ORANGE);
    LCD_Text(100, 100, "Juan Higuera", LCD_ORANGE);
    LCD_Text(110, 120, "Mateo Pena", LCD_ORANGE);
    _delay(1000);
    LCD_Clear(LCD_BLACK);
}

void PORT4_IRQHandler(void) {
    uint32_t status;

    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);
    GPIO_clearInterruptFlag(GPIO_PORT_P4, status);

    // start timer here?

}
