#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <stdio.h> //printf()
#include <stdbool.h>
#include <string.h> //strlen()
#include <stdlib.h> //rand()
#include "lcd.h"
#include "uart.h"
#include "customer.h"
#include "type.h"


void sweepAnimation(HD44780 &lcd) {

    const char sweepChar = '#';
    const char blankChar = ' ';

    //sweep forward
    for(int i = 0; i < 16; i++) {
        lcd.GoTo(i, 0);
        lcd.WriteData(sweepChar);

        lcd.GoTo(i, 1);
        lcd.WriteData(sweepChar);

        _delay_ms(20);
    }
    //sweep backward (clear)
    for(int i = 16; i >= 0; i--) {
        lcd.GoTo(i, 0);
        lcd.WriteData(blankChar);

        lcd.GoTo(i, 1);
        lcd.WriteData(blankChar);

        _delay_ms(20);
    }
} 