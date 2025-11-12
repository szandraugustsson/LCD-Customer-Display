#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "lcd.h"
#include "uart.h"

// https://wokwi.com/projects/416241646559459329 // C (analog input pins) // PORTB B (digital pin 8 to 13) // PORTD D (digital pins 0 to 7)
#define LED_PIN 2
#define BUTTON_PIN 1   // knapp/input

#define BIT_SET(a, b) ((a) |= (1ULL << (b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))


#define BUTTON_IS_CLICKED(PINB,BUTTON_PIN) !BIT_CHECK(PINB,BUTTON_PIN) // knapp/input

//--------------------------------STRUCTS--------------------------------------
typedef struct       
{
    char AdMessage[32];      // Själva reklammeddelande
}
Message;                  // Struct - varje instans innehåller 1 reklammeddelande

typedef struct 
{
    char name[20];        //kundnamn
    Message messages[3];  // Antal Ads kund betalat för (max 3 Ads)
    int messageCount;     // iterationsvariabel för antal Ads given kund är tilldelad
    int paid;             // Total summa kund har betalat
}
Customer;      // Struct - varje instans innehåller 1 kund

typedef struct 
{
    int 
}

//--------------------------------FUNTIONS--------------------------------------
void CGRAM(char *InStr)
{
    int j = 0;
    char UtStr[strlen(InStr)];
    memset(UtStr,0,sizeof(UtStr));
    for (int i = 0; i < strlen(InStr); i++)
    {   
    if      ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0x85) {UtStr[j] = 1;j++; i++;}
    else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0x84) {UtStr[j] = 2;j++; i++;}
    else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0x96) {UtStr[j] = 3;j++; i++;}
    else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0xA5) {UtStr[j] = 4;j++; i++;}
    else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0xA4) {UtStr[j] = 5;j++; i++;}
    else if ((uint8_t)InStr[i] == 0xC3 && (uint8_t)InStr[i+1] == 0xB6) {UtStr[j] = 6;j++; i++;}
    else
    {UtStr[j] = InStr[i];j++;} 
    }
    memset(InStr,0,sizeof(InStr));
    strcpy(InStr,UtStr);
}

void ScrollL2R(HD44780 &lcd,char *inputStrang)
{
    CGRAM(inputStrang);
    int strangLen = strlen(inputStrang);
    char strang[strangLen+1];
    strcpy(strang,inputStrang);
    
    char strangCopy[sizeof(strang)];
    memset(strangCopy,0,sizeof(strang));
    strcpy(strangCopy,strang);
    
    char strangLtR[sizeof(strang)];
    memset(strangLtR,0,sizeof(strang));
    
    int i = sizeof(strang)-1;
    int cnt = 1;
    
    while(1)
    {
        if(cnt<sizeof(strang))
        {   
            strangLtR[0] = strang[i];
            lcd.WriteTextTopRow(strangLtR);
            memmove(strangLtR + 1, strangLtR, sizeof(strangLtR)-1);
            _delay_ms(500);
            lcd.Clear();                      
            i--;
            cnt++;
        }
        else if(cnt>=sizeof(strang))
        {        
            strangCopy[sizeof(strangCopy)-1] = '\0';
            lcd.WriteTextTopRow(strangCopy);
            _delay_ms(500);
            lcd.Clear();
            memmove(strangCopy + 1, strangCopy, sizeof(strangCopy)-1);
            strangCopy[i] = ' ';
            i++;
            if(i==sizeof(strang))
            {
                strcpy(strangCopy,strang);
                memset(strangLtR,0,sizeof(strang));
                i = sizeof(strang)-1;
                cnt=1;
            }
        }
    }
}

//--------------------------------MAIN--------------------------------------
int main(void)
{
    init_serial();
    HD44780 lcd;
    lcd.Initialize(); // Initialize the LCD
    lcd.Clear();      // Clear the LCD
    uint8_t AW[8] = { 0b00100,0b00000,0b01110,0b10001,0b11111,0b10001,0b00000,0b10001 };
    uint8_t AE[8] = { 0b01010,0b00000,0b01110,0b10001,0b11111,0b10001,0b00000,0b10001 };
    uint8_t OO[8] = { 0b01010,0b00000,0b01110,0b10001,0b10001,0b10001,0b01110,0b00000 };
    uint8_t aw[8] = { 0b00100,0b01010,0b00100,0b01110,0b00001,0b01111,0b10001,0b01111 };
    uint8_t ae[8] = { 0b01010,0b00000,0b01110,0b00001,0b01111,0b10001,0b01111,0b00000 };
    uint8_t oo[8] = { 0b01010,0b00000,0b01110,0b10001,0b10001,0b10001,0b01110,0b00000 };
    
    lcd.CreateChar(1, AW); lcd.CreateChar(2, AE); lcd.CreateChar(3, OO);
    lcd.CreateChar(4, aw); lcd.CreateChar(5, ae); lcd.CreateChar(6, oo);

    Customer customers[5]; // array "customers" innehåller 1 element per kund
    
    //strcpy(customers[0].name, "Hederlige Harrys Bilar");
    customers[0].paid = 5000; // 1:a element i array "customers" tilldelas 5000 "lotter"    / Tot. 14500 lotter
    customers[0].messageCount = 3; // etablerar hur många Ads kund tilldelats
    
    // varje element i .messages representerar 1 av max 3 Ad-meddelande 
    strcpy(customers[0].messages[0].AdMessage,"Köp bil hos Harry");            // 0-1666
    strcpy(customers[0].messages[1].AdMessage,"En god bilaffär (för Harry!)"); // 1667-3332
    strcpy(customers[0].messages[2].AdMessage,"Hederlige Harrys Bilar");       // 3333-4999
    
    //strcpy(customers[1].name, "Farmor Ankas Pajer AB");
    customers[1].paid = 3000; 
    customers[1].messageCount = 2; 

    strcpy(customers[1].messages[0].AdMessage,"Köp paj hos Farmor Anka");             // 5000-6499
    strcpy(customers[1].messages[1].AdMessage,"Skynda innan Marten atit alla");//pajer // 6500-7999

   //strcpy(customers[2].name, "Svarte Petters Svartbyggen");
    customers[2].paid = 1500; 
    customers[2].messageCount = 2;
    
    strcpy(customers[2].messages[0].AdMessage,"Låt Petter bygga åt dig");  // 8000-8749
    strcpy(customers[2].messages[1].AdMessage,"Bygga svart? Ring Petter"); // 8750-9499
    
    //strcpy(customers[3].name, "Långbens detektivbyrå"); Dennis
    customers[3].paid = 4000;                   
    customers[3].messageCount = 2;
    
    strcpy(customers[3].messages[0].AdMessage,"Mysterier? Ring Långben");  // 9500-11499
    strcpy(customers[3].messages[1].AdMessage,"Långben fixar biffen");     // 11500-13499
    
    //strcpy(customers[4].name, "IOT:s Reklambyrå");
    customers[4].paid = 1000;                         
    customers[4].messageCount = 1;                           
    
    strcpy(customers[4].messages[0].AdMessage,"Synas här? IOT:s Reklambyrå");  // 13500-14500   

    srand(time(NULL));
    
    //--------------------------------------------------------------------------------
    // ScrollL2R(lcd,customers[3].messages[1].AdMessage);
    
    CGRAM(customers[3].messages[1].AdMessage);


//void FadeIn(char *inputStr) 
lcd.Clear();
char inputStr[sizeof(customers[3].messages[1].AdMessage)];
memset(inputStr,0,sizeof(inputStr));
strcpy(inputStr,customers[3].messages[1].AdMessage);

    // Iteration for printing singular row masks of lcd bitmaps

    for(int i = 0; i < strlen(inputStr); i++)
    {   
         
        uint8_t slicedChar = inputStr[i];  // All 8 rows(array) of currently iterated char 
        uint8_t tmpBit[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; // Blank custom char assigned to CGRAM-slot 0
        for(int j = 0; j < 8; j++)
            {
                //if(tmpBit[j] == 0b00000) 
            tmpBit[0] = (slicedChar >> j) & 0x01;
            lcd.WriteData(0);
            }
        
        lcd.CreateChar(0, tmpBit);
        _delay_ms(300);
        
        lcd.WriteData(customers[3].messages[1].AdMessage[0]);
        lcd.GoTo(0,0);
    }
    lcd.WriteText(customers[3].messages[1].AdMessage);


/*
Jag vill:

lcd.CreateChar(0, tmpBit); // skapa custom som innehåller rad1 av bitmap
lcd.WriteData(0)           // printa den  
lcd.CreateChar(0, tmpBit); // skapa custom (i samma slot) som innehåller rad1+rad2 av bitmap(?)
lcd.WriteData(0)           // printa den
-II- tot. * 7  // repetera tills hela bokstaven är klar
lcd.WriteText  // printa index 0 av sträng
-----------------II------------// Repetera tills hela strängen är utskriven

*/


    // while(1)
    // {
        //     int RandNum = rand() % 14500;
    //     lcd.Clear();    
    //     if(RandNum > 0 && RandNum < 1667)                 // 0-1666
    //         {lcd.WriteText(customers[0].messages[0].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 1666 && RandNum < 3333)         // 1667-3332
    //         {lcd.WriteText(customers[0].messages[1].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 3332 && RandNum < 5000)         // 3333-4999
    //         {lcd.WriteText(customers[0].messages[2].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 4999 && RandNum < 6500)         // 5000-6499
    //         {lcd.WriteText(customers[1].messages[0].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 6499 && RandNum < 8000)         // 6500-7999
    //         {lcd.WriteText(customers[1].messages[1].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 7999 && RandNum < 8750)         // 8000-8749
    //         {lcd.WriteText(customers[2].messages[0].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 8749 && RandNum < 9500)         // 8750-9499
    //         {lcd.WriteText(customers[2].messages[1].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 9499 && RandNum < 11500)         // 9500-11499
    //         {lcd.WriteText(customers[3].messages[0].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 11499 && RandNum < 13500)         // 11500-13499
    //         {lcd.WriteText(customers[3].messages[1].AdMessage);_delay_ms(3000);}
    //     else if(RandNum > 13499 && RandNum < 14500)         // 13500-14500
    //         {lcd.WriteText(customers[4].messages[0].AdMessage);_delay_ms(3000);}         
    // }
    return 0;
}
        /*

        1. En struct med kund: betalning, namn
        2. En struct med reklamtext (text mot rätt kund)
        3. Reklamtid = 20sek (FRÅGA: sleep är olika på olika os..?)
        4. Slumpa fram kund, beroende på betalning. rand().
        5. Om vi har tid: Skrolla/rulla texten.

Hederlige Harrys Bilar:
Betalat 5000. Vill slumpmässigt visa en av tre meddelanden
"Köp bil hos Harry"  (scroll)
"En god bilaffär (för Harry!)" text
"Hederlige Harrys Bilar" text (blinkande)
 
Farmor Ankas Pajer AB:
Betalat 3000. Vill slumpmässigt visa en av två
"Köp paj hos Farmor Anka"  (scroll)
"Skynda innan Mårten ätit alla pajer" text
 
Svarte Petters Svartbyggen:
Betalat 1500. Vill visa
"Låt Petter bygga åt dig"  (scroll) - på jämna minuter
"Bygga svart? Ring Petter" text - på ojämna minuter
 
Långbens detektivbyrå:
Betalat 4000. Vill visa
"Mysterier? Ring Långben"  text 
"Långben fixar biffen" text 
 
Ibland måste vi visa reklam för oss själva:
motsvarande för 1000 kr. 
Meddelande "Synas här? IOT:s Reklambyrå"

        */


    // lcd.WriteText((char *)"Hej hej"); // Writetext är färdig class för att skriva ut reklamtext
    // printf("Hej hej\n");
    // int r = 12;
    // printf("Hej 2 %d\n",r);
    // // //Sätt till INPUT_PULLUP
    // BIT_CLEAR(DDRB,BUTTON_PIN); // INPUT MODE
    // BIT_SET(PORTB,BUTTON_PIN); 

    // DATA DIRECTION = avgör mode
    // om output så skickar vi  1 eller 0 på motsvarande pinne på PORT
    // om input så läser vi  1 eller 0 på motsvarande pinne på PIN
    //bool blinking = false;

    //BIT_SET(DDRB,LED_PIN); //OUTPUT MODE