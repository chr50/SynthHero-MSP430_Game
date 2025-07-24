/***************************************************************************//**
 * @file    pwm.c
 * @author  Christopher Haas
 * @date    05.07.23
 *
 * @brief Implementation of Buzzer control
 *
 ******************************************************************************/

#include "./pwm.h"

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/


/******************************************************************************
 * VARIABLES
 *****************************************************************************/


/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/


/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

/**
 * Init for needed Ports for sending PWM signals to buzzer
 * This function is used in initPorts
 */
void pwm_init(void){
    // Init for sending PWM to buzzer
    P3DIR |= BIT6;
    P3SEL |= BIT6;
    TA0CCTL2 = OUTMOD_3;
    TA0CTL = TASSEL_2 + MC_1;
}

/**
 * Play a note using PWM
 * Parameter freq is frequency for a note, always use 50% duty cycle
 */
void playNotes(int freq){
    int duty = 8*freq;
    TA0CCR0 = freq*16;
    TA0CCR2 = duty;
}



