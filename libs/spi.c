/***************************************************************************//**
 * @file    spi.c
 * @author  Christopher Haas
 * @date    21.06.23
 *
 * @brief   Implementation for the SPI functions.
 *          For more comments check the functions itself.
 *
 ******************************************************************************/

#include "./spi.h"


/******************************************************************************
 * VARIABLES
 *****************************************************************************/

unsigned char spi_tx_counter;           // counter for write data
unsigned char spi_rx_counter;           // counter for read data

unsigned char *spi_tx_data;             // pointer to write data
unsigned char *spi_rx_data;             // pointer to read data

unsigned char spi_transferFinished;         // blocking variable used in both write and read, 0 unfinished, 1 finished

/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/



/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/



/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

void spi_init(void){
    // software reset
    UCB0CTL1 |= UCSWRST;

    // configure UCB0 register
    UCB0CTL0 = UCMST                        // set as single master
               + UCSYNC                     // synchronous mode
               + UCCKPH                     // data captured on the first UCLK edge and changed on the following edge
               + UCMSB                      // MSB sent first
               + UCMODE_0;                  // 3-pin spi mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;          // use SMCLK (set to 16 MHZ in templateEMP.c)
    UCB0BR0 = 160;                          // divider to achieve 100 kbit/s speed
    UCB0BR1 = 0;                            // just set to 0, not really needed here

    // Port setup, P1.5 is CC_CLK, P1.6 is CC_SO (XSCL on board), P1.7 is CC_SI (XSDA on board)
    P1SEL |= BIT5 + BIT6 + BIT7;
    P1SEL2 |= BIT5 + BIT6 + BIT7;

    // P1.3 is connected to I2C_/SPI which should be low to use SPI mode (otherwise I2C)
    P1DIR |= BIT3;
    P1OUT &= ~BIT3;

    // Setup the clock select line which is idle high
    P3DIR |= BIT4;
    P3OUT |= BIT4;

    // software reset finished
    UCB0CTL1 &= ~UCSWRST;

    // set callbacks
    tx_callback(spi_tx_isr);
    rx_callback(spi_rx_isr);
}

void spi_read(unsigned char length, unsigned char * rxData){
    spi_transferFinished = 0;
    spi_rx_counter = length;
    spi_rx_data = rxData;

    IE2 |= UCB0RXIE;            // enable receive interrupt

    while(spi_busy());
    UCB0TXBUF = 0x00;           // write first time here to ensure that the timing is correct
    while(!spi_transferFinished);   // wait for all data to be read
}

void spi_write(unsigned char length, unsigned char * txData){
    spi_transferFinished = 0;
    spi_tx_counter = length;
    spi_tx_data = txData;

    IE2 |= UCB0TXIE;            // enable transmit interrupt

    while(spi_busy());
    while(!spi_transferFinished);   // wait for all data to be written
}

unsigned char spi_busy(void){
    if(UCB0STAT & UCBUSY){
        return 1;
    }
    else{
        return 0;
    }
}

void spi_tx_isr(void) {
    // transfer about to be finished, set blocking variable to exit loop
    if(spi_tx_counter == 0){
        spi_transferFinished = 1;
        IE2 &= ~UCB0TXIE;
    }
    // write to buffer and increment counter & pointer
    else{
        UCB0TXBUF = *spi_tx_data;
        spi_tx_counter--;
        spi_tx_data++;
    }
}

void spi_rx_isr(void) {
    // transfer about to be finished, read one last time then set transferFinished to exit loop
    if(spi_rx_counter == 0){
        UCB0TXBUF = 0x00;
        *spi_rx_data = UCB0RXBUF;
        spi_transferFinished = 1;
        IE2 &= ~UCB0RXIE;
    }
    // write dummy to buffer then read from buffer and increment pointer & counter
    else{
        UCB0TXBUF = 0x00;
        *spi_rx_data = UCB0RXBUF;
        spi_rx_data++;
        spi_rx_counter--;
    }
}
