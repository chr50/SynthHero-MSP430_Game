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

/**
 * Function to init SPI communication
 * Settings are: 3-pin mode, KPH = 1, KPL = 0 (corresponds to CPOL = CPHA = 0)
 * Transmission speed 100 kbit/s
 */
void spi_init(void);

/**
 * Function to read out data from the MISO line
 * Store <length> characters in rxData
 */
void spi_read(unsigned char length, unsigned char * rxData);

/**
 * Function to write data to the MOSI line
 * Write <length> characters from txData
 */
void spi_write(unsigned char length, unsigned char * txData);

/**
 * Return 1 if SPI busy, else 0
 */
unsigned char spi_busy(void);

/**
 * Implementation of the TX ISR in SPI
 */
void spi_tx_isr(void);

/**
 * Implementation of the RX ISR in SPI
 */
void spi_rx_isr(void);

#endif /* LIBS_SPI_H_ */
