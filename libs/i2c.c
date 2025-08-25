/***************************************************************************//**
 * @file    i2c.c
 * @author  Christopher Haas
 * @date    01.06.23
 *
 * @brief   Function implementation of the I2C Functions
 *
 * Implements a init, write and read function. For more details see the
 * individual function descriptions.
 ******************************************************************************/

#include "./i2c.h"

//#define LPMO

/******************************************************************************
 * VARIABLES
 *****************************************************************************/

// A variable to be set by your interrupt service routine:
// 1 if all bytes have been sent, 0 if transmission is still ongoing.
unsigned char transferFinished = 0;

// variable set in ISR, 0 if NACK was received upon write request, 1 else
unsigned char success = 0;

/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/



/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/



/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

void i2c_init (unsigned char addr) {
    // software reset
    UCB0CTL1 |= UCSWRST;

    // configure UCB0 register
    UCB0CTL0 = UCMST + UCSYNC + UCMODE_3;   // set as (single) master, synchronous mode, i2c mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;          // use SMCLK (set to 16 MHZ in templateEMP.c)
    UCB0BR0 = 160;                          // divider to achieve 100 kbit/s speed
    UCB0BR1 = 0;                            // just set to 0, not really needed here
    UCB0I2CSA = addr;                       // set slave address

    // Port setup, P1.6 is XSCL, P1.7 is XSDA
    P1SEL |= BIT6 + BIT7;
    P1SEL2 |= BIT6 + BIT7;

    // P1.3 is connected to I2C_/SPI which should be high to use I2C mode (otherwise SPI)
    P1DIR |= BIT3;
    P1OUT |= BIT3;

    // software reset finished
    UCB0CTL1 &= ~UCSWRST;

    // set callbacks
    tx_callback(i2c_tx_isr);
    rx_callback(i2c_rx_isr);
}

unsigned char i2c_write(unsigned char length, unsigned char * txData, unsigned char stop) {
    while(UCB0CTL1 & UCTXSTP);  // check if the last STOP-condition has already been sent

    transferFinished = 0;       // reset transfer variable
    success = 0;                // reset success variable

    IE2 |= UCB0TXIE;            // enable transmitter interrupt
    IE2 &= ~UCB0RXIE;           // disable receiver interrupt
    UCB0I2CIE |= UCNACKIE;      // enable NACK interrupt

    ptxData = txData;           // set global variable
    txCounter = length;         // set global variable

    UCB0CTL1 |= UCTR + UCTXSTT; // start the transmission
    while(UCB0CTL1 & UCTXSTT);  // wait until start bit is transmitted

    #ifdef LPMO
    __bis_SR_register(CPUOFF + GIE);    // shutdown CPU, use low power mode until all data read
    __no_operation();
    #endif

    #ifndef LPMO
    while(!transferFinished);   // wait until transferFinished flag is set in ISR
    #endif

    // generate stop condition if it was requested
    if(stop != 0){
        UCB0CTL1 |= UCTXSTP;            // stop transmission
        while(UCB0CTL1 & UCTXSTP);      // wait until it is transmitted
    }

    // check if successful transfer or NACK was received
    if(success){
        return 0;           // successful transmission
    }
    else{
        return 1;           // unsuccessful (NACK)
    }
}

void i2c_read(unsigned char length, unsigned char * rxData) {
//  while(UCB0CTL1 & UCTXSTP);  // check if the last STOP-condition has already been sent

    transferFinished = 0;       // reset transfer variable

    IE2 &= ~UCB0TXIE;           // disable transmitter interrupt
    IE2 |= UCB0RXIE;            // enable receiver interrupt
    UCB0I2CIE |= UCNACKIE;      // enable NACK interrupt

    prxData = rxData;           // set global variable
    rxCounter = length;         // set global variable

    // when only reading one bit, need to immediately send stop after start
    if(length == 1) {
        UCB0CTL1 &= ~UCTR;              // set to receiver
        UCB0CTL1 |= UCTXSTT;            // start the transmission
        while(UCB0CTL1 & UCTXSTT);      // wait until start bit is transmitted
        UCB0CTL1 |= UCTXSTP;            // immediately stop transmission
        while(UCB0CTL1 & UCTXSTP);      // wait until it is transmitted
    }
    else{
        UCB0CTL1 &= ~UCTR;              // set to receiver
        UCB0CTL1 |= UCTXSTT;            // start the transmission
        while(UCB0CTL1 & UCTXSTT);      // wait until start bit is transmitted
        #ifdef LPMO
        __bis_SR_register(CPUOFF + GIE);    // shutdown CPU, use low power mode until all data read
        __no_operation();                   // set breakpoint here
        #endif

        #ifndef LPMO
        while(!transferFinished);       // wait until transfer is done
        //UCB0CTL1 |= UCTXSTP;            // stop transmission
//        while(UCB0CTL1 & UCTXSTP);      // wait until it is transmitted
        #endif
    }
}

void i2c_tx_isr(void){
    // write mode
    if(UCB0TXIFG & IFG2){
        // there is no more data to be transmitted
        if(txCounter == 0){
            transferFinished = 1;       // set to 1 to be able to exit i2c_write

            success = 1;                // set to 1 to indicate successful transaction
            IFG2 &= ~UCB0TXIFG;         // clear interrupt flag
        }
        // there is still data to be transmitted
        else
        {
            UCB0TXBUF = *ptxData;   // transmission buffer data, use current pointer position
            ptxData++;              // increment pointer position to process next byte
            txCounter--;            // decrement counter respectively
        }
    }
    // read mode
    else if(UCB0RXIFG & IFG2){
        rxCounter--;
        // last data read, after this exit LPMO
        if(rxCounter == 0){
            *prxData = UCB0RXBUF;
            transferFinished = 1;
        }
        // read from buffer
        else{
            *prxData++ = UCB0RXBUF;
            // second to last position, need to send stop already
            if(rxCounter == 1){
                UCB0CTL1 |= UCTXSTP;
            }
        }
    }
}

void i2c_rx_isr(void){

    // triggered because a NACK was received
    if(UCB0STAT & UCNACKIFG){
        transferFinished = 1;   // set to 1 to be able to exit i2c_write
        UCB0STAT &= ~UCNACKIFG; // clear interrupt flag
    }
}
