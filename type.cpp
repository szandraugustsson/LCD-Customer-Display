#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <stdio.h> //printf()
#include <stdbool.h>
#include <string.h> //strlen()
#include <stdlib.h> //rand()
#include <time.h>

#include "lcd.h"
#include "uart.h"
#include "customer.h"
#include "type.h"
#include "sweep.h"



void randomDelay() {
    int r = rand() % 3; //Pick random delay to simulate real typing
    switch(r) {
        case 0:
            return _delay_ms(50);
        case 1:
            return _delay_ms(150);
        case 2:
            return _delay_ms(300);
        default:
            return _delay_ms(150);
    }
}

void typeAnimation(HD44780 &lcd, const char* msg) {

    lcd.GoTo(0, 0);
    
    int charCount = 0;
    while (*msg) {
        lcd.WriteData(*msg++);
        charCount++;
        randomDelay();

        if (charCount >= 16) {
            lcd.GoTo(0, 1); // Move to second line after 16 chars
            charCount = 0; // Reset count for second line
            lcd.WriteData(*msg++);
            charCount++;
            randomDelay();
        }

        if (msg[-1] == ',' || msg[-1] == '.' || msg[-1] == '!' || msg[-1] == '?') {
            _delay_ms(500); // Longer delay at punctuation
        }
    }

    _delay_ms(3000); // Wait before clearing
    sweepAnimation(lcd);

}