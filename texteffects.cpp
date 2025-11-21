#include <stdio.h> //sprintf()
#include <string.h> //strlen()
#include <util/delay.h> //_delay_ms()
#include <time.h>
#include <stdlib.h> //rand()
#include "lcd.h"
#include "customer.h"
#include "texteffects.h"

#define DISPLEN 16

void scrollText(HD44780 *lcd, char *txt){

    // SCROLL FUNCTION + delay
    int cnt = 0;

    // two 'complete scrolls'
    while (cnt < 2){
        cnt++;

        char txt_part[45];
        
        // scroll to txt ends (roll out of screen)
        int lengthOfText = strlen(txt); //skapade en variabel, så att for-loopen inte behöver räkna längden på texten varje gång den körs
        for (int i = 0; i < (lengthOfText + DISPLEN + 1); i++){
            if (i <= DISPLEN){
                snprintf(txt_part, i + 1, txt);
                lcd->GoTo(DISPLEN - i, 0);
                lcd->WriteText((char *)txt_part);    
            } 
            else {
                snprintf(txt_part, DISPLEN + 1, txt + i - DISPLEN);
                lcd->Clear();
                lcd->WriteText((char *) txt_part);
            }
            _delay_ms(130);
        }
    }
}

void createSpecChar(HD44780 *lcd){
    // Detta block ritar upp själva bitmapen för varje enskilt specialtecken.
    uint8_t AW[8] = { 0b00100,0b00000,0b01110,0b10001,0b11111,0b10001,0b00000,0b10001 }; // Å
    uint8_t AE[8] = { 0b01010,0b00000,0b01110,0b10001,0b11111,0b10001,0b00000,0b10001 }; // Ä
    uint8_t OO[8] = { 0b01010,0b00000,0b01110,0b10001,0b10001,0b10001,0b01110,0b00000 }; // Ö
    uint8_t aw[8] = { 0b00100,0b01010,0b00100,0b01110,0b00001,0b01111,0b10001,0b01111 }; // å
    uint8_t ae[8] = { 0b01010,0b00000,0b01110,0b00001,0b01111,0b10001,0b01111,0b00000 }; // ä
    uint8_t oo[8] = { 0b01010,0b00000,0b01110,0b10001,0b10001,0b10001,0b01110,0b00000 }; // ö
    
    /* Här sparas varje tecken till CGRAM(Character Generator RAM) som är HD44780:s dedikerade minnesutrymme(8 slots maxcap)
    för custom chars, dem tilldelas plats 1,2,3,4,5,6*/
    lcd->CreateChar(1, AW); lcd->CreateChar(2, AE); lcd->CreateChar(3, OO); 
    lcd->CreateChar(4, aw); lcd->CreateChar(5, ae); lcd->CreateChar(6, oo);
}

void FixSpecChar(char *InStr){ // Döpte om från CGRAM till FixSpecChar
    
    int j = 0;
    char OutStr[strlen(InStr) + 1];
    memset(OutStr, 0, sizeof(OutStr));
    
    int lengthOfText = strlen(InStr); //samma gäller som tidigare
    for (int i = 0; i < lengthOfText; i++){   
        if      ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0x85) {OutStr[j] = 1;j++; i++;} // Å
        else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0x84) {OutStr[j] = 2;j++; i++;} // Ä
        else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0x96) {OutStr[j] = 3;j++; i++;} // Ö
        else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0xA5) {OutStr[j] = 4;j++; i++;} // å
        else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0xA4) {OutStr[j] = 5;j++; i++;} // ä
        else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0xB6) {OutStr[j] = 6;j++; i++;} // ö
        else
        {OutStr[j] = InStr[i];j++;} 
    }
    // Loop itererar över angiven sträng och jämför hex-tal sekvenser för att ersätta element med dem custom chars vi skapat.
    // Man kan alltså kalla funktionen som FixSpecChar(Min-Sträng-Var)
    memset(InStr, 0, lengthOfText + 1);
    strcpy(InStr, OutStr);
}


//slumpa delay till typeAnimation
void randomDelay() {
    int r = rand() % 3; //Pick random delay to simulate real typing
    switch(r) {
        case 0:
            return _delay_ms(70);
        case 1:
            return _delay_ms(100);
        case 2:
            return _delay_ms(130);
        default:
            return _delay_ms(100);
    }
}
//Animation to "transition" into new text (work in progress)
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

//text-animation to simulate typing
void typeAnimation(HD44780 &lcd, char* txt) {

    lcd.GoTo(0, 0);
    
    int charCount = 0;
    while (*txt) {
        lcd.WriteData(*txt++);
        charCount++;
        randomDelay();

        if (charCount >= 16) {
            lcd.GoTo(0, 1); // Move to second line after 16 chars
            charCount = 0; // Reset count for second line
            lcd.WriteData(*txt++);
            charCount++;
            randomDelay();
        }

        if (txt[-1] == ',' || txt[-1] == '.' || txt[-1] == '!' || txt[-1] == '?') {
            _delay_ms(500); // Longer delay at punctuation
        }
    }
}
