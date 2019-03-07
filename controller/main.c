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
#include <stdint.h>
#include <stdbool.h>
#include "cc3100_usage.h"
#include "simplelink.h"
#include "sl_common.h"
#include "_data_pack_.h"
#include "ClockSys.h"
#include "uart_driver.h"

// Global variables
_controller_t ctrl_info;
_device_t meter_info;
uint32_t HUB_IP;
uint32_t METER_IP = 0xC0A80104;
uint32_t FIRE_IP;
//Receive UART Variables
#define NUM_RX_CHARS 64
char rxMsgData[NUM_RX_CHARS] = "";
int numMsgsRx = 0;
int tempIndex = 5;
int numChars = 0;

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
    //FPU_enableModule();
    //FPU_enableLazyStacking();

    /* Store the addresses of the wifi packet structures for reuse */
    uint8_t *ctrl_ptr = &ctrl_info;
    uint8_t *meter_ptr = &meter_info;

    /* Initialize SPI to be used with WiFi */
    spi_Open(0,0);
    snprintf(test.txString, 70,
             "Initializing Wifi\n");
    sendText();
    /* Initialize CC3100 device for WiFi capability */
    initCC3100(Client);
    HUB_IP = getLocalIP();
    snprintf(test.txString, 70,
             "WiFi initialized!\n");
    sendText();

    meter_info.voltage = 0;
    meter_info.fire_requesting = false;
    meter_info.meter_requesting = false;
    meter_info.curr1 = 0;
    meter_info.curr2 = 0;
    meter_info.fire_reading = 0;
    ctrl_info.ctrl_acknowledged_fire = false;
    ctrl_info.ctrl_acknowledged_meter = false;

    snprintf(test.txString, 70,
             "Waiting for meter connection.\n");
    sendText();
    uint32_t count = 0;
    while(1) {
        ReceiveData(meter_ptr, sizeof(meter_info));
        ++count;
        if (meter_info.meter_requesting) {
            //ctrl_info.ctrl_acknowledged_meter = true;
            //SendData(ctrl_ptr, METER_IP, sizeof(ctrl_info));
            snprintf(test.txString, 70, "transmission# = %d voltage = %d\n", count, meter_info.voltage);
            sendText();
            count = 0;
            meter_info.meter_requesting = false;
            //ctrl_info.ctrl_acknowledged_meter = false;
        }
    }

    while(!meter_info.meter_requesting) {
        ReceiveData(meter_ptr, sizeof(meter_info));
    }
    snprintf(test.txString, 70,
             "Meter requested, hub listened. Sending acknowledgment.\n");
    sendText();

    ctrl_info.ctrl_acknowledged_meter = true;
    SendData(ctrl_ptr, METER_IP, sizeof(ctrl_info));

    snprintf(test.txString, 70,
             "Waiting for meter to send me readings.\n");
    sendText();
    while (ReceiveData(ctrl_ptr, sizeof(ctrl_info)) < 0);
        if (meter_info.voltage == 37) {
            snprintf(test.txString, 70,
                     "We have established good comms!\n");
            sendText();
    }

    while(1) {
        //PCM_gotoLPM0();
    }
}

