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
 * MSP432 Energy Meter Measurement Module
 *
 * Description: Measures single-phase AC power
 *
 *                             MSP432P401
 *                       _  ------------------
 *                      /|\|                  |
 *                       | |                  |
 *                       --|RST               |
 *                         |                  |
 *   (Analog Input) Vin -->|A6                |
 *   (Analog Input)  I1 -->|A8                |
 *   (Analog Input)  I2 -->|A9                |
 *                         |                  |
 *                          ------------------
 * Authors: Juan Higuera and Mateo Pena
*******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "energy_meter.h"
#include "cc3100_usage.h"
#include "simplelink.h"
#include "sl_common.h"
#include "_data_pack_.h"

#define HANDHELD_DEVICE_IP_ADDR HOST_IP_ADDR
#define ADC_14BIT_SAMPLERESOLUTION 16384
#define Vref 3.3
#define SAMPLE_LENGTH       128

// Global variables
//static uint8_t ADC_channel_selector 0;
static uint16_t ADC_result_buffer[3];
static uint16_t voltage_buffer[3];
static uint16_t curr1_buffer[3];
static uint16_t curr2_buffer[3];
float ADC_normalized_buffer[3];
static int i;
//_controller_t ctrl_info;
//_eMeter_t meter_info;
//bool doneADC = false;

/** MAIN **/
void main(void) {

    /* Stop Watchdog  */
    WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();


    /* Enabling the FPU for floating point operation */
    //FPU_enableModule();
    //FPU_enableLazyStacking();

    /* Store the addresses of the wifi packet structures for reuse */
    //uint8_t *ctrl_ptr = &ctrl_info;
    //uint8_t *meter_ptr = &meter_info;

    /* Initialize ADC buffers to 0 */
    memset(ADC_result_buffer, 0x00, 3 * sizeof(uint16_t));
    memset(voltage_buffer, 0x00, 3 * sizeof(uint16_t));
    memset(curr1_buffer, 0x00, 3 * sizeof(uint16_t));
    memset(curr2_buffer, 0x00, 3 * sizeof(uint16_t));
    ADC_normalized_buffer[0] = 0.0;
    ADC_normalized_buffer[1] = 0.0;
    ADC_normalized_buffer[2] = 0.0;
    i = 0;

    /* Initialize SPI to be used with WiFi */
    //spi_Open(0,0);

    /* Initialize CC3100 device for WiFi capability */
    //initCC3100(Client);

    //meter_info.curr1 = 0;
    //meter_info.curr2 = 0;
    //meter_info.voltage = 0;
    //meter_info.ctrl_acknowledged = false;
    // How do I get energy meter's IP?

    /* Establish communication between energy meter and controller */
    /*ctrl_info.meter_requesting = true;
    SendData(ctrl_ptr, ctrl_info.ip_addr, sizeof(meter_info));
    while(!meter_info.ctrl_acknowledged) {
        ReceiveData(meter_ptr, sizeof(meter_info));
    }*/


    /* Initialize ADC to measure Voltage and currents */
    init_EnergyMeter();

    while(1) {
        PCM_gotoLPM0();
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
    if(status & ADC_INT2) {
        /* Returns the conversion results of Vin, I1, I2 and stores it in the
         * buffer. */
        ADC14_getMultiSequenceResult(ADC_result_buffer);

        voltage_buffer[0] = ADC_result_buffer[0];
        curr1_buffer[0] = ADC_result_buffer[1];
        curr2_buffer[0] = ADC_result_buffer[2];
        ADC_normalized_buffer[0] = (ADC_result_buffer[0] * Vref) / ADC_14BIT_SAMPLERESOLUTION;
        ADC_normalized_buffer[1] = (ADC_result_buffer[1] * Vref) / ADC_14BIT_SAMPLERESOLUTION;
        ADC_normalized_buffer[2] = (ADC_result_buffer[2] * Vref) / ADC_14BIT_SAMPLERESOLUTION;
    }
}



