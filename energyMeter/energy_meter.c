/*
 * energy_meter.c
 *
 *  Created on: Feb 23, 2019
 *      Author: juanh
 */
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "energy_meter.h"

#define SMCLK_FREQUENCY     12000000
#define SAMPLE_FREQUENCY    12000
#define SAMPLE_LENGTH       128

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

void init_EnergyMeter() {

    ADC14_setResolution(ADC_14BIT);

    /* Turn on ADC module */
    ADC14_enableModule();
    /* Configure ADC to use master clock, no prescalers, and no routing */
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    /* Configuring GPIOs for Analog In */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                                   GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN7,
                                                   GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory [0..2] with repeat mode and internal 3.3V
     * reference */
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM2, true);
    ADC14_configureConversionMemory(ADC_MEM0, // A6 = P4.7
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A6, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM1, // A8 = P4.5
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A8, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM2, // A9 = P4.4
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

    /* Configuring Timer_A in continuous mode and sourced from ACLK */
    Timer_A_configureUpMode(TIMER_A0_BASE, &upModeConfig);

    /* Configuring Timer_A0 in CCR1 to trigger at 16000 (0.5s) */
    Timer_A_initCompare(TIMER_A0_BASE, &compareConfig);

    /* Configuring the sample trigger to be sourced from Timer_A0  and setting it
     * to automatic iteration after it is triggered*/
    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);
    //ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_192, ADC_PULSE_WIDTH_192);

    /* Enabling the interrupt when a conversion on channel 2 (end of sequence)
     * is complete and enabling conversions */
    ADC14_enableInterrupt(ADC_INT2);

    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    ADC14_enableConversion();

    /* Enable ADC Interrupt */
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    /* Starting the Timer */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    /* Triggering the start of the sample */
    //ADC14_enableConversion();
    //ADC14_toggleConversionTrigger();
}

