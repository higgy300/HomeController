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
Point userInput;
#define FIRE_THRESHOLD 1.0
float currentFireReading = 0.0;
bool fireDetected;




// ********** WiFi global variables ***************************
_controller_t ctrl_info;
_device_t meter_info;
_device_t fire_info;
_device_t buffer_info;
uint32_t HUB_IP;
uint32_t METER_IP = 0xC0A80103;
uint32_t FIRE_IP;

typedef struct Wifi_t {
    uint32_t HUB_IP;
    uint32_t METER_IP;
    uint32_t FIRE_IP;
    uint32_t currIP;
    bool meterConnected;
    bool fireConnected;
}Wifi_t;
Wifi_t Wifi_p;
// ************************************************************





// ********* Energy Meter globals *****************************
#define RESOLUTION 16384
#define Vref 3.3
#define OFFSET1 0.07
#define OFFSET2 0.18
#define FIRE_OFFSET 0.14
#define DEBOUNCE_PERIOD 8436
#define UPDATE_ENERGY_CONSUMPTION_TIME 47000000
#define AC_VOLTAGE 120.0

typedef struct Meter_t {
    float voltage_value;
    float current_value;
    float theft_value;
    float curr_wattMeasurement;
    float sec_watt;
    float min_watt;
    float hour_watt;
    float dailyUse;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint32_t meter_collection_timer;
}Meter_t;
Meter_t meter_t;
// ************************************************************





// ******** BME280 Globals ************************************
typedef struct Sensor_t {
    s32 returnRslt;
    s32 ActualTemp;
    u32 ActualPress;
    u32 ActualHumity;
    float norm_press;
    float norm_temp;
    float norm_hum;
}Sensor_t;
Sensor_t sensor_t;

//Receive UART Variables
#define NUM_RX_CHARS 64
char rxMsgData[NUM_RX_CHARS] = "";
int numMsgsRx = 0;
int tempIndex = 5;
int numChars = 0;
// ************************************************************





// *********** LCD state menu trackers ************************
bool cleared;
bool celsius_mode;
#define main_s 0
#define meter_s 1
#define air_s 2
#define fire_s 3
#define therm_s 4
typedef struct MenuState {
    uint8_t state;
    bool mainPrinted;
    bool meterPrinted;
    bool airPrinted;
    bool firePrinted;
    bool thermPrinted;
}MenuState;
MenuState menu_t;
// ************************************************************


// Function Prototypes
void IP_stringConvert(size_t);
void convertMeterReadings(char*, float);
void convertEnvironmentReadings();
void reverse(char*, int);
int integerToString(int, char[], int);
void floatToArray(float, char*, int);
void init_WiFi();
void launchBackup();
void config_WiFi_strucs();
void init_sensors();
void init_systemConfig();
void init_LCD();
void updateMenuState(size_t);
void printMainMenu();
void printEnergyMenu();
void printAirMenu();
void printFireMenu();
void printThermMenu();
void processWiFiTransmission(uint8_t*);
void processEnvironmentalSensorData();
void printEnergyData();
void printTemperature();
void printAirQualityData();
void printFireData();
void syncDevices(uint8_t*);


// MAIN
void main(void) {
    // Stop watchdog timer, change clock, initialize FPU
    init_systemConfig();

    // Initialize the LCD driver
    init_LCD();

    // Store the addresses of the wifi packet structures for reuse
    uint8_t* ctrl_ptr = &ctrl_info;
    uint8_t* meter_ptr = &meter_info;
    uint8_t* fire_ptr = &fire_info;
    uint8_t* buffer_ptr = &buffer_info;

    // Initialize SPI to be used with WiFi
    config_WiFi_strucs();
    init_WiFi();

    // Initialize environmental sensors
    init_sensors();
    _delay(200);

    // Sync communication with energy meter and fire alarm devices
    /*syncDevices(ctrl_ptr);
    while(1);*/

    launchBackup();

    while(1) {
        processWiFiTransmission(buffer_ptr);
        processEnvironmentalSensorData();

        switch(menu_t.state) {
            case main_s:
                if (!menu_t.mainPrinted) {
                    printMainMenu();
                    updateMenuState(main_s);
                }
                break;
            case meter_s:
                if (!menu_t.meterPrinted) {
                    printEnergyMenu();
                    updateMenuState(meter_s);
                    printEnergyData();
                } else {
                    printEnergyData();
                }
                break;
            case air_s:
                if (!menu_t.airPrinted) {
                    printAirMenu();
                    updateMenuState(air_s);
                    printAirQualityData();
                } else {
                    printAirQualityData();
                }
                break;
            case fire_s:
                if (!menu_t.firePrinted) {
                    printFireMenu();
                    updateMenuState(fire_s);
                    printFireData();
                } else {
                    printFireData();
                }
                break;
            case therm_s:
                if (!menu_t.thermPrinted) {
                    printThermMenu();
                    updateMenuState(therm_s);
                    printTemperature();
                } else {
                    printTemperature();
                }
                break;
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
    sensor_t.norm_press = (float)press / 1000.0;
    sensor_t.norm_temp = (float)temp / 100.0;
    sensor_t.norm_hum = (float)hum / 1000.0;
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

void IP_stringConvert(size_t ip_param) {
    uint16_t shftAmnt = 24;
    uint32_t arr[4];
    uint32_t _ip_addr_ = 0;
    char outputString[17];
    int i = 0, index = 0, j = 0;

    if (ip_param == main_s) {
        LCD_Text(25, 105, "IP: ", LCD_WHITE);
        _ip_addr_ = Wifi_p.HUB_IP;
    } else if (ip_param == meter_s) {
        _ip_addr_ = Wifi_p.METER_IP;
    } else if (ip_param == fire_s) {
        _ip_addr_ = Wifi_p.FIRE_IP;
    } else if (ip_param == therm_s) {
        _ip_addr_ = Wifi_p.currIP;
    }

    // Split the 4 byte number into a single byte array
    for (i = 0; i < 4; i++) {
        arr[i] = (_ip_addr_ >> shftAmnt) & 0xFF;
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
    if (ip_param == main_s)
        LCD_Text(55, 105, outputString, LCD_WHITE);
    else if (ip_param == meter_s)
        LCD_Text(155, 85, outputString, LCD_WHITE);
    else if (ip_param == fire_s)
        LCD_Text(130, 105, outputString, LCD_WHITE);
    else if (ip_param == therm_s)
        LCD_Text(130, 65, outputString, LCD_WHITE);
}

void init_WiFi() {
    LCD_Text(5, 45, "Initializing Wifi...", LCD_WHITE);
    spi_Open(0,0);
    LCD_Text(25, 65, "-SPI channel opened", LCD_WHITE);
    initCC3100(Client);
    LCD_Text(25, 85, "-WiFi driver configured", LCD_WHITE);
    Wifi_p.HUB_IP = getLocalIP();
    IP_stringConvert(main_s);
    LCD_Text(25, 125, "-Wifi connected successfully!", LCD_WHITE);
}

void config_WiFi_strucs() {
    LCD_Text(5, 5, "initializing WiFi params...", LCD_WHITE);
    meter_info.voltage = 0;
    meter_info.fire_requesting = false;
    meter_info.meter_requesting = false;
    meter_info.curr1 = 0;
    meter_info.curr2 = 0;
    meter_info.fire_reading = 0;
    Wifi_p.FIRE_IP = 0;
    Wifi_p.METER_IP = 0;
    Wifi_p.currIP = 0;
    Wifi_p.fireConnected = false;
    Wifi_p.meterConnected = false;
    ctrl_info.hub_req = true;
    ctrl_info.fire_ack = false;
    ctrl_info.meter_ack = false;
    cleared = false;
    celsius_mode = true;
    LCD_Text(25, 25, "-WiFi params initialized!", LCD_WHITE);
}

void printEnergyMenu() {
    LCD_DrawRectangle(0, 320, 0, 17, LCD_WHITE);
    LCD_Text(2, 1, "Energy Meter Section", LCD_BLACK);
    LCD_DrawRectangle(0, 320, 16, 17, LCD_BLACK);
    LCD_DrawRectangle(0, 320, 17, 240, LCD_YELLOW);
    LCD_Text(15, 25, "Raw Energy Data:", LCD_BLACK);
    LCD_Text(30, 45, "V  Ch:", LCD_BLACK);
    LCD_Text(30, 65, "C1 Ch:", LCD_BLACK);
    LCD_Text(30, 85, "C2 Ch:", LCD_BLACK);
    LCD_Text(15, 105, "Power Measured:         Watts", LCD_BLACK);
    LCD_Text(15, 125, "Daily Usage:            kWh", LCD_BLACK);
    LCD_Text(15, 145, "Monthly Usage:          kWh", LCD_BLACK);
    LCD_DrawRectangle(75, 215, 175, 220, LCD_BLACK);
    LCD_Text(120, 190, "Return", LCD_WHITE);
}

void printAirMenu() {
    LCD_DrawRectangle(0, 320, 0, 17, LCD_WHITE);
    LCD_Text(2, 1, "Air Quality Section", LCD_BLACK);
    LCD_DrawRectangle(0, 320, 16, 17, LCD_BLACK);
    LCD_DrawRectangle(0, 320, 17, 240, LCD_BLUE);
    LCD_Text(15, 25, "Environmental Measurements:", LCD_WHITE);
    LCD_Text(30, 45, "Humidity:           %rH", LCD_WHITE);
    LCD_Text(30, 65, "Pressure:           kPa", LCD_WHITE);
    LCD_Text(30, 85, "CO2:                ppm", LCD_WHITE);
    LCD_DrawRectangle(75, 215, 175, 220, LCD_BLACK);
    LCD_Text(120, 190, "Return", LCD_WHITE);
}

void printThermMenu() {
    LCD_DrawRectangle(0, 320, 0, 17, LCD_WHITE);
    LCD_Text(2, 1, "Thermostat Section", LCD_BLACK);
    LCD_DrawRectangle(0, 320, 16, 17, LCD_BLACK);
    LCD_DrawRectangle(0, 320, 17, 240, LCD_CYAN);
    if (celsius_mode)
        LCD_Text(15, 35, "Temperature:        C", LCD_BLACK);
    else
        LCD_Text(15, 35, "Temperature:        F", LCD_BLACK);
    LCD_DrawRectangle(75, 235, 105, 145, LCD_BLACK);
    LCD_Text(90, 115, "Toggle Units C/F", LCD_WHITE);
    LCD_DrawRectangle(75, 215, 175, 220, LCD_BLACK);
    LCD_Text(120, 190, "Return", LCD_WHITE);
}

void printFireMenu() {
    LCD_DrawRectangle(0, 320, 0, 17, LCD_WHITE);
    LCD_Text(2, 1, "Fire Alarm Section", LCD_BLACK);
    LCD_DrawRectangle(0, 320, 16, 17, LCD_BLACK);
    LCD_DrawRectangle(0, 320, 17, 240, LCD_RED);
    LCD_Text(10, 25, "Device#       Status      Reading", LCD_WHITE);
    LCD_Text(25, 45, "#1", LCD_WHITE);
    LCD_Text(5, 110, "NOTE: If reading below 1.01, There is", LCD_WHITE);
    LCD_Text(20, 130, "a fire present in the fire alarm", LCD_WHITE);
    LCD_Text(20, 150, "highlighted", LCD_WHITE);
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
    LCD_Text(190, 172, "Fire Alarm", LCD_WHITE);
}

void init_sensors() {
    LCD_Text(5, 145, "Initializing Sensors...", LCD_WHITE);

    // Initialize variables
    sensor_t.ActualHumity = 0;
    sensor_t.ActualPress = 0;
    sensor_t.ActualTemp = 0;
    sensor_t.norm_hum = 0.0;
    sensor_t.norm_press = 0.0;
    sensor_t.norm_temp = 0.0;
    fireDetected = false;

    // Initialize I2C with interrupts
    initI2C();
    LCD_Text(25, 165, "I2C channel opened", LCD_WHITE);

    // Initialize bme280 sensor
    bme280_data_readout_template();
    sensor_t.returnRslt = bme280_set_power_mode(BME280_NORMAL_MODE);
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
    menu_t.state = 0;
    menu_t.mainPrinted = false;
    menu_t.meterPrinted = false;
    menu_t.airPrinted = false;
    menu_t.firePrinted = false;
    menu_t.thermPrinted = false;
    userInput.x = 0;
    userInput.y = 0;
    _delay(500);
    LCD_Clear(LCD_BLACK);
}

void updateMenuState(size_t state) {
    if (state == main_s) {
        menu_t.state = main_s;
        menu_t.mainPrinted = true;
    } else
        menu_t.mainPrinted = false;
    if (state == meter_s) {
        menu_t.state = meter_s;
        menu_t.meterPrinted = true;
    } else
        menu_t.meterPrinted = false;
    if (state == fire_s) {
        menu_t.state = fire_s;
        menu_t.firePrinted = true;
    } else
        menu_t.firePrinted = false;
    if (state == air_s) {
        menu_t.state = air_s;
        menu_t.airPrinted = true;
    } else
        menu_t.airPrinted = false;
    if (state == therm_s) {
        menu_t.state = therm_s;
        menu_t.thermPrinted = true;
    } else
        menu_t.thermPrinted = false;
}

void syncDevices(uint8_t* _sync_t) {
    LCD_Clear(LCD_BLACK);
    LCD_Text(5, 5, "Syncing devices:", LCD_WHITE);
    LCD_Text(15, 25, "-Broadcasting controller IP", LCD_WHITE);
    LCD_Text(15, 45, "-Waiting for device response...", LCD_WHITE);
    uint32_t currentIP = Wifi_p.HUB_IP + (uint32_t)1;
    uint32_t targetIP = 0xC0A801FE;
    uint16_t i = 0, j = 0;
    ctrl_info.IPaddr = Wifi_p.HUB_IP;
    bool meter_pr = false, first_meter_contact = true, fire_pr = false, first_fire_contact = true;

    for (currentIP = Wifi_p.HUB_IP + (uint32_t)1; currentIP < targetIP; currentIP++) {
        Wifi_p.currIP = currentIP;
        ctrl_info.IPaddr = Wifi_p.HUB_IP;
        ctrl_info.toAddr = currentIP;
        LCD_Text(5, 65, "Scanning IP:", LCD_WHITE);
        LCD_DrawRectangle(125, 245, 62, 82, LCD_BLACK);
        IP_stringConvert(therm_s);

        for (i = 0; i < 5; i++) {
            SendData(_sync_t, currentIP, sizeof(ctrl_info));

            // New stuff
            _delay(50);
            ReceiveData(_sync_t, sizeof(ctrl_info));
            if (ctrl_info.meter_ack) {
                if (first_meter_contact) {
                    SendData(_sync_t, currentIP, sizeof(ctrl_info));
                    Wifi_p.METER_IP = currentIP;
                    first_meter_contact = false;
                    Wifi_p.meterConnected = true;
                    ctrl_info.meter_ack = false;
                    break;
                }
                break;
            } else if (ctrl_info.fire_ack) {
                if (first_fire_contact) {
                    SendData(_sync_t, currentIP, sizeof(ctrl_info));
                    Wifi_p.FIRE_IP = currentIP;
                    Wifi_p.fireConnected = true;
                    ctrl_info.fire_ack = false;
                    break;
                }
                break;
            }
        }
        i = 0;

        if (Wifi_p.meterConnected && !meter_pr) {
            meter_pr = true;
            LCD_Text(15, 85, "-Energy Meter IP:", LCD_WHITE);
            IP_stringConvert(meter_s);
        }
        if (Wifi_p.fireConnected && !fire_pr) {
            fire_pr = true;
            LCD_Text(15, 105, "-Firealarm IP:", LCD_WHITE);
            IP_stringConvert(fire_s);
        }
        if (meter_pr && fire_pr) {
            LCD_Text(10, 210, "found all available devices", LCD_WHITE);
            return;
        }
    }

    if (!Wifi_p.meterConnected)
        LCD_Text(15, 85, "-Energy Meter NOT FOUND", LCD_WHITE);
    if (!Wifi_p.fireConnected)
        LCD_Text(15, 105, "-Firealarm NOT FOUND", LCD_WHITE);
}

void processWiFiTransmission(uint8_t* buffer_ptr) {
    meter_t.meter_collection_timer++;

    // start processing wifi transmission
    ReceiveData(buffer_ptr, sizeof(buffer_info));

    // Check if transmission came from the energy meter or fire alarm
    if (buffer_info.meter_requesting) {
        // Convert the energy meter readings to actual readings
        meter_t.voltage_value = (float)(buffer_info.curr2 * Vref) / (float)RESOLUTION;
        meter_t.current_value = (float)(buffer_info.curr1 * Vref) / (float)RESOLUTION;
        meter_t.theft_value = (float)(buffer_info.voltage * Vref) / (float)RESOLUTION;

        //meter_t.voltage_value -= OFFSET2;
        //meter_t.current_value -= OFFSET2;
        meter_t.theft_value -= OFFSET2;
        if (meter_t.voltage_value < 0)
            meter_t.voltage_value = 0.0;
        if (meter_t.current_value < 0)
            meter_t.current_value = 0.0;
        if (meter_t.theft_value < 0)
            meter_t.theft_value = 0.0;
        // Convert reading to watts
        float AC_CURRENT = (1.0408 * meter_t.current_value) - 0.0535;
        meter_t.curr_wattMeasurement = AC_VOLTAGE * AC_CURRENT;

        /*if (theft detected) {

        } */

        // Change this boolean to prevent unwanted repeated data
        buffer_info.meter_requesting = false;
        buffer_info.fire_requesting = false;
    } else if (buffer_info.fire_requesting) {
        // Read FIRE ALARM data
        currentFireReading = (float)(buffer_info.fire_reading * Vref) / (float)RESOLUTION;
        currentFireReading += FIRE_OFFSET;
        if (currentFireReading <= FIRE_THRESHOLD) {
            fireDetected = true;
        } else {
            fireDetected = false;
        }

        buffer_info.fire_requesting = false;
        buffer_info.meter_requesting = false;
    }

    // Check if we are updating energy usage records
    if (meter_t.meter_collection_timer == UPDATE_ENERGY_CONSUMPTION_TIME) {
        meter_t.seconds++;
        meter_t.meter_collection_timer = 0;
        meter_t.sec_watt = (meter_t.sec_watt + meter_t.curr_wattMeasurement) / (float) meter_t.seconds;

        if (meter_t.seconds == 60) {
            meter_t.seconds = 0;
            meter_t.minutes++;
            meter_t.min_watt = (meter_t.min_watt + meter_t.sec_watt) / (float) meter_t.minutes;
            meter_t.sec_watt = 0.0;

            if (meter_t.minutes == 60) {
                meter_t.hours++;
                meter_t.hour_watt = (meter_t.hour_watt + meter_t.min_watt) / (float) meter_t.hours;
                meter_t.minutes = 0;
                meter_t.min_watt = 0.0;
            }
        }
    }
}

void processEnvironmentalSensorData() {
    // Read the BME280 data
    sensor_t.returnRslt = bme280_read_pressure_temperature_humidity(&sensor_t.ActualPress,
                                                           &sensor_t.ActualTemp,
                                                           &sensor_t.ActualHumity);
    // Convert the sensor data to float values
    convertEnvironmentReadings(sensor_t.ActualPress, sensor_t.ActualTemp, sensor_t.ActualHumity);
}

void printEnergyData() {
    // Paint over area that will be overwritten
    LCD_DrawRectangle(80, 140, 42, 105, LCD_YELLOW);
    LCD_DrawRectangle(137, 200, 100, 170, LCD_YELLOW);

    // Allocate temporary memory to handle strings of unknown size
    char* v_str = malloc(10);
    char* c1_str = malloc(10);
    char* c2_str = malloc(10);
    char* pow_str = malloc(10);
    char* daily_str = malloc(10);
    char* monthly_str = malloc(10);

    // Convert the float values to strings of unknown size
    floatToArray(meter_t.voltage_value, v_str, 3);
    floatToArray(meter_t.current_value, c1_str, 3);
    floatToArray(meter_t.theft_value, c2_str, 3);
    floatToArray(meter_t.curr_wattMeasurement, pow_str, 2);
    floatToArray(meter_t.dailyUse, daily_str, 2);
    floatToArray(meter_t.hour_watt, monthly_str, 2);

    // Print data to screen
    LCD_Text(90, 45, v_str, LCD_BLACK);
    LCD_Text(90, 65, c1_str, LCD_BLACK);
    LCD_Text(90, 85, c2_str, LCD_BLACK);
    LCD_Text(145, 105, pow_str, LCD_BLACK);
    LCD_Text(145, 125, daily_str, LCD_BLACK);
    LCD_Text(145, 145, monthly_str, LCD_BLACK);

    // Deallocate memory used for strings of unknown size
    free(v_str);
    free(c1_str);
    free(c2_str);
    free(pow_str);
    free(daily_str);
    free(monthly_str);
}

void printTemperature() {
    char *final_temp = malloc(10);
    LCD_DrawRectangle(112, 183, 30, 60, LCD_CYAN);
    if (celsius_mode) {
        floatToArray(sensor_t.norm_temp, final_temp, 2);
        LCD_Text(120, 35, final_temp, LCD_BLACK);
    } else {
        float temp;
        temp = sensor_t.norm_temp * (9.0 / 5.0) + 32.0;
        floatToArray(temp, final_temp, 2);
        LCD_Text(120, 35, final_temp, LCD_BLACK);
    }
    free(final_temp);
}

void printAirQualityData() {
    char* final_press = malloc(10);
    char* final_hum = malloc(10);
    LCD_DrawRectangle(105, 185, 40, 105, LCD_BLUE);

    // Convert float values to string
    floatToArray(sensor_t.norm_press, final_press, 2);
    floatToArray(sensor_t.norm_hum, final_hum, 2);
    LCD_Text(120, 45, final_hum, LCD_WHITE);
    LCD_Text(115, 65, final_press, LCD_WHITE);

    // Deallocate temporary memory used for strings of unknown size
    free(final_press);
    free(final_hum);
}

void printFireData() {
    char* faia = malloc(10);
    LCD_DrawRectangle(40, 300, 42, 67, LCD_RED);
    floatToArray(currentFireReading, faia, 2);
    if (fireDetected) {
        LCD_DrawRectangle(15, 300, 42, 65, LCD_BLACK);
        LCD_Text(105, 45, "FIRE DETECTED", LCD_WHITE);
    } else
        LCD_Text(135, 45, "GOOD", LCD_WHITE);

    LCD_Text(230, 45, faia, LCD_WHITE);
    free(faia);
}

void launchBackup() {
    Wifi_p.METER_IP = 0xC0A80103;
    Wifi_p.FIRE_IP = 0xC0A80104;
}



/*************************************************************************
 *  INTERRUPT HANDLING
 ************************************************************************/
void PORT4_IRQHandler(void) {
    // Reset point coordinates for new data
    userInput.x = 0;
    userInput.y = 0;

    // Initialize timer32 to debounce the touch screen input
    // with an interrupt
    Timer32_enableInterrupt(INT_T32_INT1);
    Timer32_setCount(TIMER32_0_BASE, DEBOUNCE_PERIOD);
    Timer32_startTimer(TIMER32_0_BASE, true);
    GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN0);
}

void T32_INT1_IRQHandler(void) {
    Timer32_clearInterruptFlag(TIMER32_0_BASE);

    if (!(P4->IN & GPIO_PIN0)) {
        userInput = TP_ReadXY();

        // Check current menu state and then check on expected coordinate input
        if (menu_t.mainPrinted) {
            // Check if input is on right side of screen
            if (userInput.x < 157) {
                // It is on right side of screen. Now checking if input is
                // top or bottom
                if (userInput.y > 122) {
                    menu_t.state = therm_s;
                } else
                    menu_t.state = meter_s;
            } else { // touch input is on left side of screen
                // Now checking if input is in top or bottom
                if (userInput.y > 122) {
                    menu_t.state = fire_s;
                } else
                    menu_t.state = air_s;
            }
        } else if (menu_t.meterPrinted) {
            if (userInput.x >= 75 && userInput.x <= 215 && userInput.y >= 175 && userInput.y <= 220) {
                menu_t.state = main_s;
            }
        } else if (menu_t.airPrinted) {
            if (userInput.x >= 75 && userInput.x <= 215 && userInput.y >= 175 && userInput.y <= 220) {
                menu_t.state = main_s;
            }
        } else if (menu_t.firePrinted) {
            if (userInput.x >= 75 && userInput.x <= 215 && userInput.y >= 175 && userInput.y <= 220) {
                menu_t.state = main_s;
            }
        } else if (menu_t.thermPrinted) {
            if (userInput.x >= 75 && userInput.x <= 215 && userInput.y >= 175 && userInput.y <= 220) {
                menu_t.state = main_s;
            }
            if (userInput.x >= 75 && userInput.x <= 235 && userInput.y >= 105 && userInput.y <= 145) {
                LCD_DrawRectangle(192, 200, 30, 55, LCD_CYAN);
                if (celsius_mode) {
                    celsius_mode = false;
                    LCD_Text(192,35, "F", LCD_BLACK);
                } else {
                    celsius_mode = true;
                    LCD_Text(192, 35, "C", LCD_BLACK);
                }
            }
        }
    }
}


