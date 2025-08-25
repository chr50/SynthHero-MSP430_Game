/***************************************************************************//**
 * @file    main.c
 * @author  Christopher Haas
 * @date    04.07.23
 *
 * @brief   Exercise 5 - Project
 *
 *
 * Pin connections (all are the same as from previous exercises, only Buzzer and Shift reg. new):
 *
 *          LCD related:
 *                          RS <-> P3.0, R/W <-> P3.1, E <-> P3.2
 *                          D4 <-> P2.0, D5 <-> P2.1, D6 <-> P2.2, D7 <-> P2.3
 *                          Also set JP1, (JP2,) JP3 to left side (arrow).
 *
 *          I2C + ADAC related:
 *                          P1.6 <-> XSCL, P1.7 <-> XSDA, P1.3 <-> I2C_/SPI
 *
 *          SPI + Flash related:
 *                          P1.5 <-> CC_CLK, CC_SO <-> F_SO, CC_SI <-> F_SI, CC_CLK <-> F_CLK
 *                          P3.4 <-> F_/CS, P3.3 <-> F_/HOLD, P3.5 <-> F_/WP
 *
 *          Buzzer related:
 *                          P3.6 <-> BUZZER, JP5 to VFO
 *
 *          Shift register related:
 *                          Set jumpers on Port 2 for P2.7 to P2.2
 *
 *
 * For detailed description of the project refer to the video.
 *
 * @note    The project was exported using CCS 8.0.0.
 *
 ******************************************************************************/

#include "libs/templateEMP.h"   // UART disabled, see @note!
#include "libs/lcd.h"
#include "libs/adac.h"
#include "libs/flash.h"
#include "libs/shift.h"
#include "libs/pwm.h"
#include <stddef.h>


/******************************************************************************
 * CONSTANTS or GAMEPARAMETERS
 *****************************************************************************/

#define delay_gameover         48000000     // how long game over screen is held, 3 real-time seconds
#define delay_menu              3200000     // how fast menu gets updated, 0.2 real-time seconds
#define delay_tone               800000     // how long tone is played when button pressed in game, 50ms real time
#define delay_song1             2000000     // how fast notes move in game for song 1
#define delay_song2             2400000     // how fast notes move in game for song 2
#define delay_song3             3600000     // how fast notes move in game for song 3

#define song1_note1                 262     // frequency of tone 1 of song 1, C4
#define song1_note2                 349     // frequency of tone 2 of song 1, F4
#define song1_note3                 392     // frequency of tone 3 of song 1, G4
#define song1_note4                 523     // frequency of tone 4 of song 1, C5

#define song2_note1                 262     // frequency of tone 1 of song 2, C4
#define song2_note2                 294     // frequency of tone 2 of song 2, D4
#define song2_note3                 330     // frequency of tone 3 of song 2, E4
#define song2_note4                 349     // frequency of tone 4 of song 2, F4

#define song3_note1                 523     // frequency of tone 1 of song 3, C5
#define song3_note2                 587     // frequency of tone 2 of song 3, D5
#define song3_note3                 659     // frequency of tone 3 of song 3, E5
#define song3_note4                 698     // frequency of tone 4 of song 3, F5

/**
 * Arrays for storing which chars will be displayed.
 * 1 - 4 in this will correspond to the button that shall
 * be pressed. There are three songs in total.
 */ 
const unsigned char notes1[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                '1', ' ', ' ', ' ', '4', ' ', ' ', ' ',
                                '2', ' ', ' ', ' ', '3', ' ', ' ', ' ',
                                '3', ' ', '3', ' ', ' ', ' ', '1', ' ',
                                ' ', ' ', '4', ' ', ' ', ' ', '2', ' ',
                                ' ', ' ', '3', ' ', ' ', '3', ' ', '1',
                                ' ', ' ', ' ', '1', ' ', ' ', '4', ' ',
                                ' ', ' ', '2', ' ', ' ', ' ', '3', ' ',
                                ' ', ' ', '3', ' ', '3', ' ', ' ', ' ',
                                '1', ' ', ' ', ' ', '2', ' ', ' ', ' ',
                                '3', ' ', ' ', ' ', '4', ' ', '4', ' ',
                                ' ', ' ', '1', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',};

const unsigned char notes2[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                '1', ' ', ' ', ' ', '4', ' ', ' ', ' ',
                                '1', ' ', ' ', ' ', '4', ' ', ' ', ' ',
                                '1', ' ', ' ', ' ', '4', ' ', ' ', ' ',
                                '1', ' ', ' ', ' ', '3', ' ', ' ', ' ',
                                '1', ' ', ' ', '2', ' ', ' ', '3', ' ',
                                ' ', '4', ' ', ' ', ' ', ' ', '2', ' ',
                                ' ', '3', ' ', ' ', '2', ' ', ' ', '3',
                                ' ', ' ', '2', ' ', ' ', '3', ' ', ' ',
                                '2', ' ', ' ', '3', ' ', ' ', ' ', ' ',
                                '1', ' ', ' ', '2', ' ', ' ', '3', ' ',
                                ' ', '4', ' ', ' ', ' ', ' ', '4', ' ',
                                ' ', '3', ' ', ' ', '2', ' ', ' ', '1',
                                ' ', ' ', ' ', ' ', '3', ' ', ' ', '1',
                                ' ', ' ', '2', ' ', ' ', '1', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',};

const unsigned char notes3[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                '1', ' ', '4', ' ', '2', ' ', '3', ' ',
                                '1', ' ', '4', ' ', '3', ' ', '2', ' ',
                                '1', ' ', '2', ' ', '3', ' ', '4', ' ',
                                '4', ' ', '2', ' ', '3', ' ', '1', ' ',
                                '1', ' ', '4', ' ', '2', ' ', '3', ' ',
                                '1', ' ', '3', ' ', '2', ' ', '4', ' ',
                                '1', ' ', '2', ' ', '3', ' ', '4', ' ',
                                '4', ' ', '1', ' ', '4', ' ', '1', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',};


/******************************************************************************
 * VARIABLES
 *****************************************************************************/

enum GameState{
    menus,
    ingame,
    gameover,
};

enum MenuPoint{
    chooseSong,
    playSong1,
    playSong2,
    playSong3,
    chooseDifficulty,
    setDifficultyNormal,
    setDifficultyHard,
    resetScore,
    chooseName,
    setName,
    chooseScore,
    score1,
    score2,
    score3,
};

typedef struct{
    const char* text;                         // name to be displayed
    unsigned char up, down, left, right, bt;  // flags for symbols shown
    unsigned char scores;                     // 255: no score, else index of it
    unsigned char customName;                 // 1 to show the custom name, 0 else
}MenuEntry;

enum Difficulty{
    normal,
    hard,
};

enum SongChoice{
    song1,
    song2,
    song3,
};

enum GameState game_state = menus;
enum MenuPoint menu_point = chooseSong;
enum Difficulty difficulty = normal;
enum SongChoice song_choice = song1;

// Init the Lookup table for menupoints elements, used in drawMenu function
const MenuEntry menuEntries[] = {
    [chooseSong]         = {"Play a Song",    0,1,0,1,0,  255, 0},
    [playSong1]          = {"Song 1",         0,1,1,0,1,  255, 0},
    [playSong2]          = {"Song 2",         1,1,0,0,1,  255, 0},
    [playSong3]          = {"Song 3",         1,0,0,0,1,  255, 0},
    [chooseDifficulty]   = {"Difficulty",     1,1,0,1,0,  255, 0},
    [setDifficultyNormal]= {"Normal",         0,1,1,0,1,  255, 0},
    [setDifficultyHard]  = {"Expert",         1,0,0,0,1,  255, 0},
    [resetScore]         = {"Reset?",         1,0,0,0,1,  255, 0},
    [chooseName]         = {"Player Name",    1,1,0,1,0,  255, 0},
    [setName]            = {NULL,             0,1,1,0,0,  255, 1},
    [chooseScore]        = {"Highscore",      1,0,0,1,0,  255, 0},
    [score1]             = {"Score Song1:",   0,1,1,0,0,  0,   0},
    [score2]             = {"Score Song2:",   0,1,1,0,0,  1,   0},
    [score3]             = {"Score Song3:",   0,1,1,0,0,  2,   0},
};

unsigned char score;                            // variable to store the current score
unsigned char bestScores[3];                    // stored values of best scores of all time for three songs

unsigned char note_count = 0;                   // used as control variable to play the songs (and display them)
                                                // can be interpreted as follows:
                                                // note_count is @0, then notesX[0], ..., notesX[15] will be displayed
                                                // note_count is @1, then notesX[1], ..., notesX[16] will be displayed etc.

unsigned char cursor_position = 0;              // used to keep track where we are when in naming menu
unsigned char joystick[2];                      // ADAC value from joystick is stored
                                                // first entry is horizontal position, second is vertical

unsigned char name[4] = {'a', 'a', 'a', 'a'};   // Array to store the chosen name, limited to 4 chars, could be more if wanted
unsigned char scoresW[3] = {0, 0, 0};           // Only needed if you want to reset the highscore
unsigned char scoresR[4] = {};                  // Array to read out the highscore values from the flash on init


/******************************************************************************
 * GAME FUNCTIONS
 *****************************************************************************/

/**
 * Init all necessary functions.
 * Also access the stored highscores on the flash.
 */
void init_all(void){
    initMSP();                                            
    flash_init(); flash_read(0, 4, scoresR);              // read the stored scores values on the flash
    bestScores[0] = scoresR[1];                           // best scores for first song
    bestScores[1] = scoresR[2];                           // best scores for second song
    bestScores[2] = scoresR[3];                           // best scores for third song
    shift_init(); lcd_init(); adac_init(); pwm_init();    // init used modules, see lib files
    create_custom_char_one(); create_custom_char_two(); 
    create_custom_char_three();                           // for the custom chars displayed
}


/**
 * Used to change the name in the respective submenu (menu_point == setName).
 * Function uses the read-out joystick values and then chooses the action.
 */
void modifyName(void){
    if(joystick[1] == 0) name[cursor_position]++;                             // joystick up, increment char on display
    else if(joystick[1] == 255) name[cursor_position]--;                      // joystick down, decrement char on display
    if((joystick[0] == 0) && (cursor_position < 3)) cursor_position++;        // joystick to right, increment cursor
    else if((joystick[0] == 255) && (cursor_position > 0)) cursor_position--; // joystick to left, decrement cursor
}


/**
 * Function to draw the current menu view.
 * First line is always the fixed gametitle,
 * second line depends on which menu_point we
 * are currently in.
 */
void drawMenu(){
    lcd_clear();
    const MenuEntry *entry = &menuEntries[menu_point];

    // First line drawn, always the same
    lcd_putChar(0x00); lcd_putChar(0x00);
    lcd_putChar(' '); lcd_putText("Synth Hero"); lcd_putChar(' ');
    lcd_putChar(0x00); lcd_putChar(0x00);
    
    // Second line drawn, depends on menu_point
    lcd_cursorSet(0, 1);
    
    // First check in which menu we are and draw the strings
    if(entry->customName){
        for(unsigned char i = 0; i < 4; i++){
            lcd_putChar(name[i]);
        }
    }
    else if(entry->scores != 255){
        lcd_putText(entry->text);
        lcd_putNumber(bestScores[entry->scores]);
    }
    else{
        lcd_putText(entry->text);
    }
    
    // Now draw arrow and button elements to indicate where we can navigate
    if(entry->up) {
        lcd_cursorSet(15, 1);
        lcd_putChar(0x04);
    }
    if(entry->down) {
        lcd_cursorSet(14, 1);
        lcd_putChar(0x02);
    }
    if(entry->left) {
        lcd_cursorSet(12, 1);
        lcd_putChar(0x7F);
    }
    if(entry->right) {
        lcd_cursorSet(13, 1);
        lcd_putChar(0x7E);
    }
    if(entry->bt) {
        lcd_cursorSet(11, 1);
        lcd_putChar(0x6F);
    }
    
    // This is needed to set the cursor to the right position
    // in naming menu, where cursor show is on to allow the user
    // to enter a name and then also turn it on
    if(menu_point == setName){
        lcd_cursorSet(cursor_position, 1);
        lcd_cursorShow(1);
    }
}


/**
 * Function to navigate up, down, left, right in the menu.
 * Depending on joystick value which are global,
 * global variable menu_point gets modified.
 * Function returns 1 if something changed, 0 else (so that we
 * only draw when a change is registered).
 */
unsigned char navigateMenu(void){
    if(joystick[1] == 255){             // joystick down
        switch(menu_point){
            case chooseSong:
                menu_point = chooseDifficulty;
                return 1;
            case chooseDifficulty:
                menu_point = chooseName;
                return 1;
            case chooseName:
                menu_point = chooseScore;
                return 1;
            case playSong1:
                menu_point = playSong2;
                return 1;
            case playSong2:
                menu_point = playSong3;
                return 1;
            case setDifficultyNormal:
                menu_point = setDifficultyHard;
                return 1;
            case score1:
                menu_point = score2;
                return 1;
            case score2:
                menu_point = score3;
                return 1;
            case score3:
                menu_point = resetScore;
                return 1;
            case setName:
                modifyName();
                return 1;
        }
    }
    else if(joystick[1] == 0){          // joystick up
        switch(menu_point){
            case chooseDifficulty:
                menu_point = chooseSong;
                return 1;
            case chooseName:
                menu_point = chooseDifficulty;
                return 1;
            case chooseScore:
                menu_point = chooseName;
                return 1;
            case playSong2:
                menu_point = playSong1;
                return 1;
            case playSong3:
                menu_point = playSong2;
                return 1;
            case setDifficultyHard:
                menu_point = setDifficultyNormal;
                return 1;
            case score2:
                menu_point = score1;
                return 1;
            case score3:
                menu_point = score2;
                return 1;
            case resetScore:
                menu_point = score3;
                return 1;
            case setName:
                modifyName();
                return 1;
        }
    }
    // horizontal joystick position
    if(joystick[0] == 0){               // joystick right
        switch(menu_point){
            case chooseSong:
                menu_point = playSong1;
                return 1;
            case chooseDifficulty:
                menu_point = setDifficultyNormal;
                return 1;
            case chooseName:
                menu_point = setName;
                return 1;
            case chooseScore:
                menu_point = score1;
                return 1;
            case setName:
                modifyName();
                return 1;
        }
    }
    else if(joystick[0] == 255){        // joystick left
        switch(menu_point){
            case playSong1:
                menu_point = chooseSong;
                return 1;
            case setDifficultyNormal:
                menu_point = chooseDifficulty;
                return 1;
            case setName:
                if(cursor_position == 0){
                    menu_point = chooseName;
                    lcd_cursorShow(0);
                }
                modifyName();
                return 1;
            case score1:
                menu_point = chooseScore;
                return 1;
        }
    }
    return 0;
}


/**
 * Function to process pressed button in menu_points which allow to.
 * Can be used while in menu_point playSongX and setDifficultyX.
 * This function switches the game_state from menu_point to ingame.
 */
void processPressMenu(void){
    switch(menu_point){
        case playSong1:
            song_choice = song1;
            game_state = ingame;
            break;
        case playSong2:
            song_choice = song2;
            game_state = ingame;
            break;
        case playSong3:
            song_choice = song3;
            game_state = ingame;
            break;
        case setDifficultyNormal:
            difficulty = normal;
            menu_point = chooseSong;
            drawMenu();
            break;
        case setDifficultyHard:
            difficulty = hard;
            menu_point = chooseSong;
            drawMenu();
            break;
        case resetScore:
            bestScores[0] = 0;
            bestScores[1] = 0;
            bestScores[2] = 0;
            flash_init();
            flash_write(0, 3, bestScores);
            adac_init(); // need this, because flash and adac are not compatible and only one can be active
            menu_point = chooseScore;
            drawMenu();
    }
}


/**
 * Function to update the score and play a short tone with given frequency.
 * The score gets updated depending on which difficulty we play in.
 * This function gets called in processPressGame.
 */
void processNote(unsigned char correct, unsigned int frequency){
    lcd_cursorSet(0, 0);  
    if(correct){
        switch(difficulty){
            case normal:
                lcd_putText("+2");
                score += 2;
                break;
            case hard:
                lcd_putText("+5");
                score += 5;
                break;
        }
    }
    else{
        lcd_putText("-1");
        if(score > 0){
            score --;
        }
    }
    playNotes(frequency);
    __delay_cycles(delay_tone);
    playNotes(0);
}


/**
 * Function to register and process all button presses during a song ingame.
 * Depending on the pressed button a tone the helper function processNote() gets
 * called which updates the score and plays tone with certain frequency.
 */
void processPressGame(unsigned char press){
    const unsigned char *notes = NULL;
    unsigned int frequency = 0;
    
    // determine correct notes and frequencies dependend on song_choice and press
    switch(song_choice){
        case song1: 
            notes = notes1;
            frequency = (press == 1) ? song1_note1 :
                        (press == 2) ? song1_note2 :
                        (press == 3) ? song1_note3 : 
                                       song1_note4;
            break;
        case song2:
            notes = notes2;
            frequency = (press == 1) ? song2_note1 :
                        (press == 2) ? song2_note2 :
                        (press == 3) ? song2_note3 : 
                                       song2_note4;
            break;
        case song3:
            notes = notes3;
            frequency = (press == 1) ? song3_note1 :
                        (press == 2) ? song3_note2 :
                        (press == 3) ? song3_note3 : 
                                       song3_note4;
            break;
    }
    
    // define variable to check if correct button is pressed for the note
    unsigned char expectedNote = '1' + (press - 1);
    
    // check if note matches current position +- 1
    if((notes[note_count] == expectedNote) ||           // this would be the precise case when the note is exactly at the left side of screen and should be pressed
       (notes[note_count + 1] == expectedNote) ||       // also include buffer that it counts for +1
       (notes[note_count - 1] == expectedNote)) {       // and -1 such that the gameplay feels a bit smoother
        processNote(1, frequency);
    }
    else {
        processNote(0, frequency);
    }
}


/**
 * Function to play the current song_choice.
 * This prints out all the values from notesX on the screen,
 * depending on the current position note_count.
 * Also this changes the program to gameover state if all
 * content from notesX are played.
 */
void playSong(){
    const unsigned char *notes = NULL;
    size_t notesSize = 0;
    
    // get needed notes and determine size of notebook
    switch(song_choice){
        case song1: notes = notes1; notesSize = sizeof(notes1); break;
        case song2: notes = notes2; notesSize = sizeof(notes2); break;
        case song3: notes = notes3; notesSize = sizeof(notes3); break;
    }
    
    // always draw 16 elements of the song and go through it sequentally
    lcd_cursorSet(0, 1);
    for(unsigned char i = note_count; i < note_count + 16; i++){
        lcd_putChar(notes[i]);
    }
    
    // gameover condition when all elements of the respective notes are drawn once
    if(note_count >= notesSize - 16){
        game_state = gameover;
    }
}


/**
 * This function sets the speed with which the song is played.
 * Define the delay depending on song_choice and difficulty.
 * Difficulty hard is always twice as fast as normal.
 */
void delay(void){
    switch(song_choice){
        case song1:
            if(difficulty == normal){
                __delay_cycles(delay_song1);
            }
            __delay_cycles(delay_song1);
            break;
        case song2:
            if(difficulty == normal){
                __delay_cycles(delay_song2);
            }
            __delay_cycles(delay_song2);
            break;
        case song3:
            if(difficulty == normal){
                __delay_cycles(delay_song3);
            }
            __delay_cycles(delay_song3);
            break;
    }
}


/**
 * Draw the gameover menu view.
 * First line is the score, second line a message depending on the score.
 */
void drawGameOver(void){
    // first line
    lcd_clear();
    lcd_putText("Score: ");
    lcd_putNumber(score);
    lcd_cursorSet(0, 1);
    // second line
    unsigned char perfectScore = 0;
    unsigned char goodScore = 0;
    // determine tresholds for displayed message depending on difficulty
    switch(song_choice){
        case song1: 
            perfectScore = (difficulty == normal) ? 48 : 120;
            goodScore = (difficulty == normal) ? 40 : 99;
            break;
        case song2:
        case song3:
            perfectScore = (difficulty == normal) ? 64 : 160;
            goodScore = (difficulty == normal) ? 50 : 134;
            break;
    }
    if(score == perfectScore){
        lcd_putText("Perfect!");
    }
    else if(score > goodScore){
        lcd_putText("Very Good!");
    }
    else{
        lcd_putText("Practice more!");
    }
}


/**
 * Function to reset important game variables.
 * Also saves new best score on the flash if necessary.
 */
void resetGame(void){
    game_state = menus;
    menu_point = chooseSong;

    // if necessary update highscore
    if(score > bestScores[song_choice]){
        bestScores[song_choice] = score;
        flash_init();
        flash_write(0, 3, bestScores);
        adac_init();
    }
    score = 0;
    note_count = 0;
}


/******************************************************************************
 * GAME LOOP
 *****************************************************************************/


int main(void) {
    init_all();

    unsigned char change = 0;
    unsigned char press;

    while(1){
        switch(game_state){
            case menus:
                drawMenu();
                while(game_state == menus){
                    adac_read(joystick);        // read out joystick
                    change = navigateMenu();    // check if some input was registered
                    if(change){
                        drawMenu();             // only draw if input was registered
                    }
                    press = stateButton();      // check if button was pressed
                    if(press){
                        processPressMenu();     // execute possible button press actions
                    }
                    __delay_cycles(delay_menu);
                }
                break;
            case ingame:
                while(game_state == ingame){
                    lcd_clear();
                    lcd_cursorShow(0);                      // turn off before drawing game related stuff
                    playSong();                             // draw the notes of the current note_count position
                    press = stateButton();                  // check if (and if yes which) button (1-4) was pressed
                    processPressGame(press);                // process pressed button either increase score or decrease
                    note_count++;                           // increment to iterate through notesX (1 or 2 or 3)
                    lcd_cursorSet(0, 1);
                    lcd_cursorShow(1);                      // turn on cursor for little help when to press
                    delay();                                // defines speed of song based on difficulty
                }
                lcd_cursorShow(0);
                break;
            case gameover:
                drawGameOver();
                resetGame();
                __delay_cycles(delay_gameover);
                break;
        }
    }
}


/******************************************************************************
 * TIMER
 *****************************************************************************/

/**
 * Timer only used for playing PWM signals with Buzzer
 */
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)
{
    TACTL &= ~TAIFG;
}







