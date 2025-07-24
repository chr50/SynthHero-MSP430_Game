/***************************************************************************//**
 * @file    adac.c
 * @author  Christopher Haas
 * @date    01.06.23
 *
 * @brief   Function implementation of the ADAC functions.
 *
 ******************************************************************************/

#include "./adac.h"

/******************************************************************************
 * VARIABLES
 *****************************************************************************/

#define address 0x48        // i2c adress of the device, found on the master schematic
#define ctrlOutput 0x40     // control byte of the ADAC for analog output

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
 * Inits the ADAC unit as slave.
 */
unsigned char adac_init(void){
    i2c_init(address);
    return 0;
}


/**
 * Read from the ADAC unit.
 */
unsigned char adac_read(unsigned char * values){

    unsigned char wdata = 0x44; // control byte for AD conversion

    unsigned char status;
    status = i2c_write(1, &wdata, 0);   // first send control byte
    i2c_read(1, &wdata);                // read old pending data to dummy
    i2c_read(2, values);                // then read out data
    return status;
}


/**
 * Write a value to the ADAC unit. Also already send the control byte,
 * s.t. the digital value gets converted and we receive output at AOUT.
 */
unsigned char adac_write(unsigned char value){

    unsigned char data[] = {ctrlOutput, value};     // create data stack
    unsigned char status;                           // variable to return 0 (successful) or 1 (unsuccessful, NACK)

    status = i2c_write(2, data, 1);                 // send data including control byte
    return status;
}
