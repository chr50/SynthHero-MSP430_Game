/***************************************************************************//**
 * @file    common_isr.c
 * @author  Christopher Haas
 * @date    21.06.23
 *
 * @brief   To implement the callbacks
 *
 ******************************************************************************/

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "common_isr.h"

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/



/******************************************************************************
 * VARIABLES
 *****************************************************************************/

ISR_callback tx_isr = 0;
ISR_callback rx_isr = 0;

/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/

void tx_callback(ISR_callback callback) {
    tx_isr = callback;
}

void rx_callback(ISR_callback callback) {
    rx_isr = callback;
}

/**
 * Common ISR for USCIAB0TX
 * Vector number 6 can be found in msp430g2553.h
 */
#pragma vector = 6
__interrupt void USCIAB0TX_ISR(void)
{
    tx_isr();
}

/**
 * Common ISR for USCIAB0RX
 * Vector number 7 can be found in msp430g2553.h
 */
#pragma vector = 7
__interrupt void USCIAB0RX_ISR(void)
{
    rx_isr();
}
