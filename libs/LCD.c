/***************************************************************************//**
 * @file    LCD.c
 * @exnum   1
 * @author  Christopher Haas
 * @date    11.05.23
 * @mtrcn   3308365
 * @mail    tronikhaas@web.de
 *
 * @brief   Implementation of functions defined in LCD.h.
 *          More details in specific function descriptions.
 *
 ******************************************************************************/

#include "./LCD.h"
#include <stdio.h>

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/

// Those delays assume clock speed of 16MHz.
// They are also set relatively generously and could possibly be optimized a bit.

#define delay_enable 200*16
#define delay_send_data 200*16
#define delay_clear 1000*16
#define delay_cursorSet 200*16

// Shorthands for the pin connects.
#define EN BIT2
#define RW BIT1
#define RS BIT0
#define D4 BIT0
#define D5 BIT1
#define D6 BIT2
#define D7 BIT3

// Custom char to be used.
// A musical note.
unsigned char custom_one[8][5] = {
                                  {0, 0, 1, 0, 0},
                                  {0, 0, 1, 1, 0},
                                  {0, 0, 1, 1, 1},
                                  {0, 0, 1, 0, 1},
                                  {0, 0, 1, 0, 0},
                                  {0, 1, 1, 0, 0},
                                  {1, 1, 1, 0, 0},
                                  {0, 1, 1, 0, 0}
};

// Custom char to be used.
// Arrow down
unsigned char custom_two[8][5] = {
                                  {0, 0, 0, 0, 0},
                                  {0, 0, 0, 0, 0},
                                  {0, 0, 1, 0, 0},
                                  {0, 0, 1, 0, 0},
                                  {1, 0, 1, 0, 1},
                                  {0, 1, 1, 1, 0},
                                  {0, 0, 1, 0, 0},
                                  {0, 0, 0, 0, 0}
};

// Custom char to be used.
//Arrow up
unsigned char custom_three[8][5] = {
                                    {0, 0, 0, 0, 0},
                                    {0, 0, 1, 0, 0},
                                    {0, 1, 1, 1, 0},
                                    {1, 0, 1, 0, 1},
                                    {0, 0, 1, 0, 0},
                                    {0, 0, 1, 0, 0},
                                    {0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0}
};

// Custom char to be used.
// Arrow right
unsigned char custom_four[8][5] = {
                                    {0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0},
                                    {0, 0, 1, 0, 0},
                                    {1, 1, 1, 1, 0},
                                    {1, 1, 1, 1, 1},
                                    {1, 1, 1, 1, 0},
                                    {0, 0, 1, 0, 0},
                                    {0, 0, 0, 0, 0}
};

// Custom char to be used.
// Arrow left
unsigned char custom_five[8][5] = {
                                    {0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0},
                                    {0, 0, 1, 0, 0},
                                    {0, 1, 1, 1, 1},
                                    {1, 1, 1, 1, 1},
                                    {0, 1, 1, 1, 1},
                                    {0, 0, 1, 0, 0},
                                    {0, 0, 0, 0, 0}
};

/******************************************************************************
 * VARIABLES
 *****************************************************************************/

// Those variables are first set for init process and can be changed later with corr. functions.

unsigned char display = 1;              // 0 display off, 1 display on
unsigned char cursor = 0;               // 0 cursor off, 1 cursor on
unsigned char blink = 0;                // 0 blinking off, 1 blinking on

// Those variables are first set for init process and could be changed later,
// but currently no such functions are implemented.

const unsigned char shift = 0;          // 0 no shift, 1 shift display
const unsigned char ID = 1;             // 0 decrement, 1 increment cursor or display

// Those variables are once set for init process and can not be changed later.

const unsigned char mode = 0;           // 0 is 4 bit mode, 1 is 8 bit mode
const unsigned char line = 1;           // 0 is 1 line, 1 are two lines
const unsigned char font = 0;           // 0 is 5x8 font, 1 is 5x10 font

// Variables for current x and y position of cursor.
// Only x-pos currently used in lcd_putText() function to calculate how much space is left in line,
// maybe y_pos will be useful in future.

int x_pos = 0;                          // value between 0 and 39 (max line length)
int y_pos = 0;                          // value between 0 and 1 (two line mode)

/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/


/******************************************************************************
 * LOCAL FUNCTION IMPLEMENTATION
 *****************************************************************************/

/**
 * Function to send an enable pulse: h = high = 1, l = low = 0.
 */
void enable(unsigned char e){
    if(e == 0x00){
        P3OUT &= ~EN;
        __delay_cycles(delay_enable);
    } else if(e == 0x01){
        P3OUT |= EN;
        __delay_cycles(delay_enable);
    }
}

/**
 * Function to send packet of 4 bits of data.
 * Write either 0 or 1 for corresponding bit.
 */
void send_data(unsigned char d7, unsigned char d6, unsigned char d5, unsigned char d4){
    if(d7 == 0x01){
        P2OUT |= D7;
    } else if(d7 == 0x00){
        P2OUT &= ~D7;
    }
    if(d6 == 0x01){
        P2OUT |= D6;
    } else if(d6 == 0x00){
        P2OUT &= ~D6;
    }
    if(d5 == 0x01){
        P2OUT |= D5;
    } else if(d5 == 0x00){
        P2OUT &= ~D5;
    }
    if(d4 == 0x01){
        P2OUT |= D4;
    } else if(d4 == 0x00){
        P2OUT &= ~D4;
    }
    __delay_cycles(delay_send_data);
}

/******************************************************************************
 * FUNCTION IMPLEMENTATION
 *****************************************************************************/

/** Initialization */

/**
 * Function for manual init process, as described in datasheet p.46.
 * Settings for init are: 4 bit mode, 2 line, 5x8 Font, display on, cursor off, blinking off.
 */
void lcd_init(void){
    // Set all pins as outputs
    P3DIR |= (RS|EN|RW);
    P2DIR |= (D4|D5|D6|D7);

    // Init all pins with low
    P3OUT &= ~(RS|EN|RW);
    P2OUT &= ~(D4|D5|D6|D7);

    // Init process as described in Figure 24 of HD44780 datasheet, p.46

    __delay_cycles(50000*16);                          // wait for 50 ms to be sure

    // set data to 0011, repeat 3 times with different delays in between

    enable(1);                                      // enable high
    send_data(0, 0, 1, 1);                          // write instruction 0011
    enable(0);                                      // enable low

    __delay_cycles(80000);                           // wait for 5 ms to be sure

    enable(1);
    send_data(0, 0, 1, 1);
    enable(0);

    __delay_cycles(3200);                            // wait for 200 us to be sure

    enable(1);
    send_data(0, 0, 1, 1);
    enable(0);

    // Intermission

    enable(1);
    send_data(0, 0, 1, 0);
    enable(0);

    // Function set: 4 Bit mode, 2 lines, 5x8 Font

    enable(1);
    send_data(0, 0, 1, mode);                   // 0010 sets 4 bit mode, 0011 would be 8 bit
    enable(0);
    enable(1);
    send_data(line, font, 0, 0);                // NF**; N=1 2 lines (N=0 1 line); F=0 5x8 Font (F=1 5x10)
    enable(0);

    // Set display, cursor, blinking on/off

    enable(1);
    send_data(0, 0, 0, 0);                      // default
    enable(0);
    enable(1);
    send_data(1, display, cursor, blink);       // 1DCB; here display on, cursor off, blinking off
    enable(0);

    // clear display

    enable(1);
    send_data(0, 0, 0, 0);                      // default
    enable(0);
    enable(1);
    send_data(0, 0, 0, 1);                      // default
    enable(0);

    __delay_cycles(1000*16);

    // Entry mode set

    enable(1);
    send_data(0, 0, 0, 0);                      // default
    enable(0);
    enable(1);
    send_data(0, 1, ID, shift);                 // 01 I/D S; I/D increment by 1; no shift
    enable(0);
}

/** Control functions */

/**
 * Function to enable (1) or disable (0) the LCD.
 * Function modifies variable display and then sends correct instruction depending on
 * variable values of display, cursor, blink.
 */
void lcd_enable (unsigned char on){
    // modify display variable
    if(on == 0x01){
        display = 1;
    } else if(on == 0x00){
        display = 0;
    }

    // send instruction to show display
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);
    enable(1);
    send_data(1, display, cursor, blink);
    enable(0);
}

/**
 * Function to set cursor to a certain x/y-position.
 * Range of positions: x = 0, ..., 39 (only 16 bits can be displayed at once, but you could shift display)
 *                     y = 0, 1 (2 line mode set)
 */
void lcd_cursorSet (unsigned char x, unsigned char y){
    char bits[7];
    int i;

    // assign internal variables to the new positions
    x_pos = x;
    y_pos = y;

    // determine x position, 39 (decimal) values possible
    // access the bits with right shift and extract with and 1,
    // store their bit values in array
    for(i = 5; i >= 0; i--){
        bits[i] = (x >> i) & 1;
    }

    // determine y position, only two values possible
    if(y == 0x00){
        bits[6] = 0;
    } else if(y == 0x01){
        bits[6] = 1;
    }

    // send instruction to set DDRAM address to change cursor pos
    // D7 of first data packet has to be 1 by default
    // D6 of first data packet determines y-pos
    // D5 - D0 of first and second data packet determine x-pos
    P3OUT &= ~RS;
    enable(1);
    send_data(1, bits[6], bits[5], bits[4]);
    enable(0);
    enable(1);
    send_data(bits[3], bits[2], bits[1], bits[0]);
    enable(0);

    __delay_cycles(delay_cursorSet);
}

/**
 * Function to show (1) or hide (0) cursor.
 * Function modifies variable cursor and then sends correct instruction depending on
 * variable values of display, cursor, blink.
 */
void lcd_cursorShow (unsigned char on){
    // modify cursor variable
    if(on == 0x01){
        cursor = 1;
    } else if(on == 0x00){
        cursor = 0;
    }

    // send instruction show cursor
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);
    enable(1);
    send_data(1, display, cursor, blink);
    enable(0);
}

/**
 * Function to turn blink on (1) or off (0).
 * Function modifies variable blink and then sends correct instruction depending on
 * variable values of display, cursor, blink.
 */
void lcd_cursorBlink (unsigned char on){
    // modify blink variable
    if(on == 0x01){
        blink = 1;
    } else if(on == 0x00){
        blink = 0;
    }

    // send instruction blink cursor
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);
    enable(1);
    send_data(1, display, cursor, blink);
    enable(0);
}

/** Data manipulation */

/**
 * Function to delete everything on the LCD.
 */
void lcd_clear (void){
    // send instruction to clear LCD
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);
    enable(1);
    send_data(0, 0, 0, 1);
    enable(0);

    // also reset x_pos and y_pos
    x_pos = 0;
    y_pos = 0;

    __delay_cycles(delay_clear);
}

/**
 * Function to put a single character on the display at cursor's current position.
 */
void lcd_putChar (char character){
    char bits[8];
    int i;

    // access the bits with right shift and extract with and 1,
    // store their bit values in the array
    for(i = 7; i >= 0; i--){
        bits[i] = (character >> i) & 1;
    }

    // send data to write char on LCD
    P3OUT |= RS;
    enable(1);
    send_data(bits[7], bits[6], bits[5], bits[4]);
    enable(0);
    enable(1);
    send_data(bits[3], bits[2], bits[1], bits[0]);
    enable(0);

    // also modify x_pos and y_pos variable of cursor
    if(x_pos == 39){
        if(y_pos == 0){
            x_pos = 0;
            y_pos = 1;
        } else if(y_pos == 1){
            x_pos = 0;
            y_pos = 0;
        }
    } else {
        x_pos++;
    }
}

/**
 * Function to put a string on current cursor position.
 * If string is too long (max length per line 40), don't show rest.
 */
void lcd_putText (char * text){
    // calculate how much space is left by using current x_pos
    // and subtracting it from max space per line (0 - 39, so 40 max)
    int space_left = 40 - x_pos;

    // if there is still space left,
    // iterate over all chars in string and use before implemented lcd_putChar function
    while (*text && space_left > 0) {
        lcd_putChar(*text);
        text++;
        space_left--;
    }
}

/**
 * Function to show number at cursors position.
 */
void lcd_putNumber (int number){
    // convert number to string, then use lcd_putText
    char arr[32];
    sprintf(arr, "%d", number);     // this standard function takes number of type int and stores it into arr
    lcd_putText(arr);
}

/**
 * Function to create custom character and save it in the CGRAM.
 * This function only creates the defined custom character custom_one in the very specific free space of CGRAM.
 * Function could be modified a bit to be more general, but since only one custom character is needed for the task
 * leave it as it is for now.
 */
void create_custom_char_one(){

    // Set CGRAM address to 0x00
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 1, 0, 0);
    enable(0);
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);

    // store custom_char 1
    P3OUT |= RS;
    unsigned char i;
    for(i = 0; i < 8; i++){
        enable(1);
        send_data(0, 0, 0, custom_one[i][0]);
        enable(0);
        enable(1);
        send_data(custom_one[i][1], custom_one[i][2], custom_one[i][3], custom_one[i][4]);
        enable(0);
    }

    // return home
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);
    enable(1);
    send_data(0, 0, 1, 0);
    enable(0);

    __delay_cycles(16000);
}

void create_custom_char_two(){

    // Set CGRAM address to 0x1
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 1, 0, 1);
    enable(0);
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);

    // store custom_char 2
    P3OUT |= RS;
    unsigned char i;
    for(i = 0; i < 8; i++){
        enable(1);
        send_data(0, 0, 0, custom_two[i][0]);
        enable(0);
        enable(1);
        send_data(custom_two[i][1], custom_two[i][2], custom_two[i][3], custom_two[i][4]);
        enable(0);
    }

    // return home
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);
    enable(1);
    send_data(0, 0, 1, 0);
    enable(0);

    __delay_cycles(16000);
}

void create_custom_char_three(){

    // Set CGRAM address to 0x1
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 1, 1, 0);
    enable(0);
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);

    // store custom_char 3
    P3OUT |= RS;
    unsigned char i;
    for(i = 0; i < 8; i++){
        enable(1);
        send_data(0, 0, 0, custom_three[i][0]);
        enable(0);
        enable(1);
        send_data(custom_three[i][1], custom_three[i][2], custom_three[i][3], custom_three[i][4]);
        enable(0);
    }

    // return home
    P3OUT &= ~RS;
    enable(1);
    send_data(0, 0, 0, 0);
    enable(0);
    enable(1);
    send_data(0, 0, 1, 0);
    enable(0);

    __delay_cycles(16000);
}


