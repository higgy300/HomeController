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
#include <stdint.h>
#include <stdbool.h>

/* Timer_A Continuous Mode Configuration Parameter */
const Timer_A_UpModeConfig upModeConfig =
{
        TIMER_A_CLOCKSOURCE_ACLK,            // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,       // ACLK/1 = 32Khz
        16384,
        TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer ISR
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE, // Disable CCR0
        TIMER_A_DO_CLEAR                     // Clear Counter
};

/* Timer_A Compare Configuration Parameter */
const Timer_A_CompareModeConfig compareConfig =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_1,          // Use CCR1
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_SET_RESET,               // Toggle output but
        16384                                       // 10ms Period
};

// Global variables
static uint16_t ADC_result_buffer[3];

int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();
    Interrupt_enableSleepOnIsrExit();

    /* Initialize buffer to 0 */
    memset(ADC_result_buffer, 0x00, 3 * sizeof(uint16_t));

    /* Change ACLK () to REFO (32 KHz) to use Timer_A */
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    ADC14_setResolution(ADC_14BIT);

    /* Turn on ADC module */
    ADC14_enableModule();
    /* Configure ADC to use master clock, no prescalers, and no routing */
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    /* Configuring GPIOs for Analog In */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
            GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN7,
            GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory [0..2] with repeat mode and internal 3.3V
     * reference */
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM2, true);
    ADC14_configureConversionMemory(ADC_MEM0, // A6 = P4.7
            ADC_VREFPOS_EXTPOS_VREFNEG_EXTNEG,
            ADC_INPUT_A6, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM1, // A8 = P4.5
            ADC_VREFPOS_EXTPOS_VREFNEG_EXTNEG,
            ADC_INPUT_A8, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM2, // A9 = P4.4
            ADC_VREFPOS_EXTPOS_VREFNEG_EXTNEG,
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

    /* Setting up the sample timer to automatically step through the sequence
     * convert. */
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);


    ADC14_enableConversion();

    /* Enable ADC Interrupt */
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    /* Starting the Timer */
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    /* Triggering the start of the sample */
    //ADC14_enableConversion();
    //ADC14_toggleConversionTrigger();

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
    if(status & ADC_INT2) 
        /* Returns the conversion results of Vin, I1, I2 and stores it in the
         * buffer. */
        ADC14_getMultiSequenceResult(ADC_result_buffer);
    }

