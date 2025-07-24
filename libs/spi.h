/***************************************************************************//**
 * @file    spi.h
 * @author  Christopher Haas
 * @date    21.06.23
 *
 * @brief   .h file for the SPI functions.
 *
 ******************************************************************************/

#ifndef LIBS_SPI_H_
#define LIBS_SPI_H_

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <msp430g2553.h>
#include "./common_isr.h"

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/



/******************************************************************************
 * VARIABLES
 *****************************************************************************/



/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/

// Set the USCI-machine to SPI and switch the 74HCT4066 (1 pt.)
void spi_init(void);

// Read <length> bytes into <rxData> (1 pt.)
void spi_read(unsigned char length, unsigned char * rxData);

// Write <length> bytes from <txData> (1 pt.)
void spi_write(unsigned char length, unsigned char * txData);

// Interrupt service routines in your spi.c (1 pt.)

// Returns 1 if the SPI is still busy or 0 if not.
// Note: this is optional. You will most likely need this, but you don't have
// to implement or use this.
unsigned char spi_busy(void);

void spi_tx_isr(void);
void spi_rx_isr(void);

#endif /* LIBS_SPI_H_ */
