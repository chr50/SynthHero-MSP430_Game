/***************************************************************************//**
 * @file    shift.c
 * @author  Christopher Haas
 * @date    05.07.23
 *
 * @brief Implementation of shift register functions
 *
 ******************************************************************************/

#include "./shift.h"

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/


/******************************************************************************
 * VARIABLES
 *****************************************************************************/


/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/

// function to reset output of the shift register
void clear(){
    P2OUT &= ~BIT5;
    P2OUT |= BIT5;
}

// function to access clockline fast
// 0 set clock to low, 1 set to high
void clock(unsigned char i){
    if(i == 0){
        P2OUT &= ~BIT4;
    }
    else{
        P2OUT |= BIT4;
    }
}

// function to send bit to shift register
void sendBit(unsigned char i){
    if(i == 0){
        P2OUT &= ~BIT6;
    }
    else{
        P2OUT |= BIT6;
    }
}

/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

void shift_init(void){
    // Set pins P2 (for LEDs)
    P2DIR |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
    P2SEL &= ~(BIT6|BIT7);
    P2SEL2 &= ~(BIT6|BIT7);
}

// function to control output of LED
// i = 1, ..., 4 determines which LED will be on, 0 if no LED is on
void stateLED(unsigned char i){

    switch(i){
        case 0:
            clear();
            return;
        case 1:
            clear();
            clock(0);
            sendBit(1);
            clock(1);
            clock(0);
            return;
        case 2:
            clear();
            clock(0);
            sendBit(1);
            clock(1);
            clock(0);
            sendBit(0);
            clock(1);
            clock(0);
            return;
        case 3:
            clear();
            clock(0);
            sendBit(1);
            clock(1);
            clock(0);
            sendBit(0);
            clock(1);
            clock(0);
            sendBit(0);
            clock(1);
            clock(0);
            return;
        case 4:
            clear();
            clock(0);
            sendBit(1);
            clock(1);
            clock(0);
            sendBit(0);
            clock(1);
            clock(0);
            sendBit(0);
            clock(1);
            clock(0);
            sendBit(0);
            clock(1);
            clock(0);
            return;
    }
}

// returns index of pressed button,
// 0 if nothing is pressed, 1 to 4 if corresponding
// buttons are pressed
unsigned char stateButton(void){

    unsigned char pb1;
    unsigned char pb2;
    unsigned char pb3;
    unsigned char pb4;

    clear();
    clock(1);
    sendBit(0);

    // turn off register 2
    P2OUT &= ~(BIT0|BIT1);

    // set register 1 to parallel mode
    P2OUT |= (BIT2|BIT3);

    //load button states into register 1
    clock(0);
    clock(1);

    //set register 1 to right shift mode
    //P2OUT |= BIT2;
    P2OUT &= ~BIT3;

    // shift trough register and save button states
    pb4 = P2IN;
    pb4 &= BIT7;

    clock(0);
    clock(1);

    pb3 = P2IN;
    pb3 &= BIT7;

    clock(0);
    clock(1);

    pb2 = P2IN;
    pb2 &= BIT7;

    clock(0);
    clock(1);

    pb1 = P2IN;
    pb1 &= BIT7;

    // turn off register 1
    P2OUT &= ~BIT2;

    // check if button 4 is pressed or not
    if(pb4){
        return 4;
    }

    // check if button 3 is pressed or not
    if(pb3){
        return 3;
    }

    // check if button 2 is pressed or not
    if(pb2){
        return 2;
    }

    // check if button 3 is pressed or not
    if(pb1){
        return 1;
    }

    // no button is pressed
    return 0;
}


