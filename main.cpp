#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h> // Behövs för uint8_t tror jag?
#include "lcd.h"
#include "uart.h"

// https://wokwi.com/projects/416241646559459329 // C (analog input pins) // PORTB B (digital pin 8 to 13) // PORTD D (digital pins 0 to 7)
#define LED_PIN 2
#define BUTTON_PIN 1   

#define BIT_SET(a, b) ((a) |= (1ULL << (b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))


#define BUTTON_IS_CLICKED(PINB,BUTTON_PIN) !BIT_CHECK(PINB,BUTTON_PIN) 

//--------------------------------FUNCTIONS--------------------------------------
void FixSpecChar(char *InStr) // Döpte om från CGRAM till FixSpecChar
{
    int j = 0;
    char OutStr[strlen(InStr)];
    memset(OutStr,0,sizeof(OutStr));
    for (int i = 0; i < strlen(InStr); i++)
    {   
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
    memset(InStr,0,sizeof(InStr));
    strcpy(InStr,OutStr);
}

//--------------------------------MAIN--------------------------------------
int main(void)
{
    init_serial();
    HD44780 lcd;
    lcd.Initialize(); // Initialize the LCD
    lcd.Clear();      // Clear the LCD

    // Detta block ritar upp själva bitmapen för varje enskilt specialtecken.
    uint8_t AW[8] = { 0b00100,0b00000,0b01110,0b10001,0b11111,0b10001,0b00000,0b10001 }; // Å
    uint8_t AE[8] = { 0b01010,0b00000,0b01110,0b10001,0b11111,0b10001,0b00000,0b10001 }; // Ä
    uint8_t OO[8] = { 0b01010,0b00000,0b01110,0b10001,0b10001,0b10001,0b01110,0b00000 }; // Ö
    uint8_t aw[8] = { 0b00100,0b01010,0b00100,0b01110,0b00001,0b01111,0b10001,0b01111 }; // å
    uint8_t ae[8] = { 0b01010,0b00000,0b01110,0b00001,0b01111,0b10001,0b01111,0b00000 }; // ä
    uint8_t oo[8] = { 0b01010,0b00000,0b01110,0b10001,0b10001,0b10001,0b01110,0b00000 }; // ö
    
    /* Här sparas varje tecken till CGRAM(Character Generator RAM) som är HD44780:s dedikerade minnesutrymme(8 slots maxcap)
    för custom chars, dem tilldelas plats 1,2,3,4,5,6*/
    lcd.CreateChar(1, AW); lcd.CreateChar(2, AE); lcd.CreateChar(3, OO); 
    lcd.CreateChar(4, aw); lcd.CreateChar(5, ae); lcd.CreateChar(6, oo);



        
    return 0;
}
 