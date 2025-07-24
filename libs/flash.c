/***************************************************************************//**
 * @file    flash.c
 * @author  Christopher Haas
 * @date    21.06.23
 *
 * @brief   Implementation for the flash memory chip functions
 *
 ******************************************************************************/

#include "./flash.h"


/******************************************************************************
 * VARIABLES
 *****************************************************************************/

unsigned char rdid[4] = {};     // store rdid vales
unsigned char status[2];        // status register bits

/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/



/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/



/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

/**
 * Inits the flash module
 * HOLD and protect Write currently are not really used and just held constant high
 */
void flash_init(void){
    spi_init();

    // Init ports for HOLD and protect Write
    P3DIR |= BIT3 + BIT5;
    P3OUT |= BIT3 + BIT5;      // Both are idle high
}

/**
 * Program for testing, this sends the Read Identification instruction
 * and returns 3 bytes (should be 0x20, 0x20, 0x15).
 */
void flash_rdid(void){

    unsigned char RDID = 0x9F;

    P3OUT &= ~BIT4;
    spi_write(1, &RDID);
    spi_read(3, rdid);
    P3OUT |= BIT4;
}

/**
 * Read <length> bytes from <address> and store them in rxData
 * IMPORTANT!: current implementation always reads out 0 at first
 * data transmission and then the data stored beginning at address.
 * This means, that if we want to read for example with length 4
 * bytes, we need to provide rxData of size 5.
 */
void flash_read(long int address, unsigned char length, unsigned char * rxData){

    unsigned char read = 0x3;
    unsigned char split_address[3] = {0x0, 0x0, 0x0};

    // splits the 24 bit address into 3 bytes
    split_address[0] = (address >> 16) & 0xFF;
    split_address[1] = (address >> 8) & 0xFF;
    split_address[2] = address & 0xFF;

    P3OUT &= ~BIT4;
    spi_write(1, &read);
    spi_write(3, split_address);
    spi_read(length, rxData);
    P3OUT |= BIT4;
}

/**
 * Write <length> bits from txData and store them in address
 */
void flash_write(long int address, unsigned char length, unsigned char * txData){

    unsigned char WREN = 0x6;
    unsigned char PP = 0x2;
    unsigned char SE = 0xD8;

    unsigned char split_address[3] = {0x0, 0x0, 0x0};

    // splits the 24 bit address into 3 bytes
    split_address[0] = (address >> 16) & 0xFF;
    split_address[1] = (address >> 8) & 0xFF;
    split_address[2] = address & 0xFF;

    // write enable
    P3OUT &= ~BIT4;
    spi_write(1, &WREN);
    P3OUT |= BIT4;

    // sector erase for clearing this sector first
    P3OUT &= ~BIT4;
    spi_write(1, &SE);
    spi_write(3, split_address);
    P3OUT |= BIT4;

    __delay_cycles(12000000);           // wait 0.75s

    // write enable again, since completing sector erase resets write enable
    P3OUT &= ~BIT4;
    spi_write(1, &WREN);
    P3OUT |= BIT4;

    // page program
    P3OUT &= ~BIT4;
    spi_write(1, &PP);
    spi_write(3, split_address);
    spi_write(length, txData);
    P3OUT |= BIT4;

}

/**
 * Check if flash is busy, e.g. WIP bit Status register is set or not
 * Return 1 if busy, 0 else
 * Function currently not used
 */
unsigned char flash_busy(void){

    unsigned char RDSR = 0x5;

    // read status register & save it
    P3OUT &= ~BIT4;
    spi_write(1, &RDSR);
    spi_read(1, status);
    P3OUT |= BIT4;

    return status[1] & 0x1;
}
