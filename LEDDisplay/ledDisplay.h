/**
 * The LED Display module contains a thread which handles configuring the seg
 * display and displaying the menu system
 */
#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H

// starts the display thread
void LedDisplay_start(void);

// stops the display thread and does cleanup
void LedDisplay_stop(void);

// sets the display to display 'number'
void LedDisplay_setDisplayNumber(int number);

#endif
