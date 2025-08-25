/***************************************************************************//**
 * @file    i2c.h
 * @author  Christopher Haas
 * @date    01.06.23
 *
 * @brief   Header file for I2C Functions
 *
 ******************************************************************************/

#ifndef EXERCISE_LIBS_I2C_H_
#define EXERCISE_LIBS_I2C_H_

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

unsigned char txCounter;    // variable to keep track of the remaining bytes in the transmission data stack
                            // set in i2c_write function, decremented in transmission ISR

unsigned char * ptxData;    // pointer to the transmission data stack
                            // set in i2c_write function, incremented in transmission ISR

unsigned char rxCounter;    // variable to keep track of the remaining bytes in the receiver data stack
                            // set in i2c_read function, decremented in transmission ISR

unsigned char * prxData;    // pointer to the receiver data stack
                            // set in i2c_read function, incremented in transmission ISR

/******************************************************************************
 * FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * Init Function to set a slave address and do configurations.
 * The transmission speed is 100kbit/s, assuming a 16MHZ SMCLK.
 */
void i2c_init (unsigned char addr);

/**
 * Function to write I2C signal from *txData, can be of various length.
 * Only sends stop if requested (value any other than 0).
 * Returns 0 if transfer successful, 1 else.
 */
unsigned char i2c_write(unsigned char length, unsigned char * txData, unsigned char stop);

/**
 * Function to read from the I2C bus and save it in rxData.
 * Returns 0 if transfer uccessful, else 1.
 */
void i2c_read(unsigned char length, unsigned char * rxData);

/**
 * Implementation of the TX ISR in I2C
 */
void i2c_tx_isr(void);

/**
 * Implementation of the RX ISR in I2C
 */
void i2c_rx_isr(void);

#endif /* EXERCISE_LIBS_I2C_H_ */
