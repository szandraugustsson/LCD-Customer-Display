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

// char GetBitmap(HD44780 &lcd, char inChar)
// {
//     if(inChar  == 'A'){lcd.CreateChar(0,AUp);return AUp;}
// }

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

//A                                                                                         //N 
uint8_t AUp[8] = { 0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b00000 };uint8_t NUp[8] = { 0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b00000 };
uint8_t ALo[8] = { 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b10011, 0b01101, 0b00000 };uint8_t NLo[8] = { 0b00000, 0b00000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001, 0b00000 };
//B                                                                                         //O                                           
uint8_t BUp[8] = { 0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110, 0b00000 };uint8_t OUp[8] = { 0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000 };
uint8_t BLo[8] = { 0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b11110, 0b00000 };uint8_t OLo[8] = { 0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000 };
//C                                                                                         //P                                              
uint8_t CUp[8] = { 0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110, 0b00000 };uint8_t PUp[8] = { 0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000, 0b00000 };
uint8_t CLo[8] = { 0b00000, 0b01110, 0b10001, 0b10000, 0b10000, 0b10001, 0b01110, 0b00000 };uint8_t PLo[8] = { 0b00000, 0b00000, 0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000 };
//D                                                                                         //Q                                          
uint8_t DUp[8] = { 0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110, 0b00000 };uint8_t QUp[8] = { 0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101, 0b00000 };
uint8_t DLo[8] = { 0b00001, 0b00001, 0b01101, 0b10011, 0b10001, 0b10001, 0b01111, 0b00000 };uint8_t QLo[8] = { 0b00000, 0b00000, 0b01111, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001 };
//E                                                                                         //R                                            
uint8_t EUp[8] = { 0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111, 0b00000 };uint8_t RUp[8] = { 0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001, 0b00000 };
uint8_t ELo[8] = { 0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b10001, 0b01110, 0b00000 };uint8_t RLo[8] = { 0b00000, 0b00000, 0b10110, 0b11001, 0b10000, 0b10000, 0b10000, 0b00000 };
//F                                                                                         //S                         
uint8_t FUp[8] = { 0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000, 0b00000 };uint8_t SUp[8] = { 0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110, 0b00000 };
uint8_t FLo[8] = { 0b00110, 0b01001, 0b01000, 0b11100, 0b01000, 0b01000, 0b01000, 0b00000 };uint8_t SLo[8] = { 0b00000, 0b00000, 0b01110, 0b10000, 0b01110, 0b00001, 0b11110, 0b00000 };
//G                                                                                         //T            
uint8_t GUp[8] = { 0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110, 0b00000 };uint8_t TUp[8] = { 0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000 };
uint8_t GLo[8] = { 0b00000, 0b01111, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110, 0b00000 };uint8_t TLo[8] = { 0b01000, 0b01000, 0b11100, 0b01000, 0b01000, 0b01001, 0b00110, 0b00000 };
//H                                                                                         //U                               
uint8_t HUp[8] = { 0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b00000 };uint8_t UUp[8] = { 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000 };
uint8_t HLo[8] = { 0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001, 0b00000 };uint8_t ULo[8] = { 0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b10011, 0b01101, 0b00000 };
//I                                                                                         //V                                 
uint8_t IUp[8] = { 0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110, 0b00000 };uint8_t VUp[8] = { 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100, 0b00000 };
uint8_t ILo[8] = { 0b00100, 0b00000, 0b01100, 0b00100, 0b00100, 0b00100, 0b01110, 0b00000 };uint8_t VLo[8] = { 0b00000, 0b00000, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100, 0b00000 };
//J                                                                                         //W                           
uint8_t JUp[8] = { 0b00111, 0b00001, 0b00001, 0b00001, 0b10001, 0b10001, 0b01110, 0b00000 };uint8_t WUp[8] = { 0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010, 0b00000 };
uint8_t JLo[8] = { 0b00010, 0b00000, 0b00110, 0b00010, 0b00010, 0b10010, 0b01100, 0b00000 };uint8_t WLo[8] = { 0b00000, 0b00000, 0b10001, 0b10001, 0b10101, 0b10101, 0b01010, 0b00000 };
//K                                                                                         //X                                 
uint8_t KUp[8] = { 0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001, 0b00000 };uint8_t XUp[8] = { 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b01010, 0b10001, 0b00000 };
uint8_t KLo[8] = { 0b10000, 0b10000, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b00000 };uint8_t XLo[8] = { 0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b00000 };
//L                                                                                         //Y                              
uint8_t LUp[8] = { 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111, 0b00000 };uint8_t YUp[8] = { 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000 };
uint8_t LLo[8] = { 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110, 0b00000 };uint8_t YLo[8] = { 0b00000, 0b00000, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110, 0b00000 };
//M                                                                                         //Z                                    
uint8_t MUp[8] = { 0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001, 0b00000 };uint8_t ZUp[8] = { 0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111, 0b00000 };
uint8_t MLo[8] = { 0b00000, 0b00000, 0b11010, 0b10101, 0b10101, 0b10101, 0b10101, 0b00000 };uint8_t ZLo[8] = { 0b00000, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111, 0b00000 };
//SPACE
uint8_t SPACE[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000 };

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
    strcpy(customers[3].messages[1].AdMessage,"ABabbb");     // 11500-13499 Långben fixar biffen
    
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
// char inputStr[sizeof(customers[3].messages[1].AdMessage)];   // Långben fixar biffen
// memset(inputStr,0,sizeof(inputStr));
// strcpy(inputStr,customers[3].messages[1].AdMessage);

    // Iteration for printing singular row masks of lcd bitmaps

    uint8_t tmpBit[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; // Blank custom char assigned to CGRAM-slot 0
    uint8_t slicedChar[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; // Blank custom char assigned to CGRAM-slot 8

    char inputStr[] = "Hej din feta fan";

    for(int i = 0; i < strlen(inputStr); i++)// om element 1 = A skickar vi hela A-Bitmapen till slicedchar
    {   
        if     (inputStr[i]  == 'A'){ memcpy(slicedChar, AUp, sizeof(AUp)); }  else if(inputStr[i]  == 'N'){ memcpy(slicedChar, NUp, sizeof(NUp)); } 
        else if(inputStr[i]  == 'a'){ memcpy(slicedChar, ALo, sizeof(ALo)); }  else if(inputStr[i]  == 'n'){ memcpy(slicedChar, NLo, sizeof(NLo)); }  
        else if(inputStr[i]  == 'B'){ memcpy(slicedChar, BUp, sizeof(BUp)); }  else if(inputStr[i]  == 'O'){ memcpy(slicedChar, OUp, sizeof(OUp)); } 
        else if(inputStr[i]  == 'b'){ memcpy(slicedChar, BLo, sizeof(BLo)); }  else if(inputStr[i]  == 'o'){ memcpy(slicedChar, OLo, sizeof(OLo)); }
        else if(inputStr[i]  == 'C'){ memcpy(slicedChar, CUp, sizeof(CUp)); }  else if(inputStr[i]  == 'P'){ memcpy(slicedChar, PUp, sizeof(PUp)); } 
        else if(inputStr[i]  == 'c'){ memcpy(slicedChar, CLo, sizeof(CLo)); }  else if(inputStr[i]  == 'p'){ memcpy(slicedChar, PLo, sizeof(PLo)); }
        else if(inputStr[i]  == 'D'){ memcpy(slicedChar, DUp, sizeof(DUp)); }  else if(inputStr[i]  == 'Q'){ memcpy(slicedChar, QUp, sizeof(QUp)); } 
        else if(inputStr[i]  == 'd'){ memcpy(slicedChar, DLo, sizeof(DLo)); }  else if(inputStr[i]  == 'q'){ memcpy(slicedChar, QLo, sizeof(QLo)); }
        else if(inputStr[i]  == 'E'){ memcpy(slicedChar, EUp, sizeof(EUp)); }  else if(inputStr[i]  == 'R'){ memcpy(slicedChar, RUp, sizeof(RUp)); } 
        else if(inputStr[i]  == 'e'){ memcpy(slicedChar, ELo, sizeof(ELo)); }  else if(inputStr[i]  == 'r'){ memcpy(slicedChar, RLo, sizeof(RLo)); }
        else if(inputStr[i]  == 'F'){ memcpy(slicedChar, FUp, sizeof(FUp)); }  else if(inputStr[i]  == 'S'){ memcpy(slicedChar, SUp, sizeof(SUp)); } 
        else if(inputStr[i]  == 'f'){ memcpy(slicedChar, FLo, sizeof(FLo)); }  else if(inputStr[i]  == 's'){ memcpy(slicedChar, SLo, sizeof(SLo)); }
        else if(inputStr[i]  == 'G'){ memcpy(slicedChar, GUp, sizeof(GUp)); }  else if(inputStr[i]  == 'T'){ memcpy(slicedChar, TUp, sizeof(TUp)); } 
        else if(inputStr[i]  == 'g'){ memcpy(slicedChar, GLo, sizeof(GLo)); }  else if(inputStr[i]  == 't'){ memcpy(slicedChar, TLo, sizeof(TLo)); }
        else if(inputStr[i]  == 'H'){ memcpy(slicedChar, HUp, sizeof(HUp)); }  else if(inputStr[i]  == 'U'){ memcpy(slicedChar, UUp, sizeof(UUp)); } 
        else if(inputStr[i]  == 'h'){ memcpy(slicedChar, HLo, sizeof(HLo)); }  else if(inputStr[i]  == 'u'){ memcpy(slicedChar, ULo, sizeof(ULo)); }
        else if(inputStr[i]  == 'I'){ memcpy(slicedChar, IUp, sizeof(IUp)); }  else if(inputStr[i]  == 'V'){ memcpy(slicedChar, VUp, sizeof(VUp)); } 
        else if(inputStr[i]  == 'i'){ memcpy(slicedChar, ILo, sizeof(ILo)); }  else if(inputStr[i]  == 'v'){ memcpy(slicedChar, VLo, sizeof(VLo)); }  
        else if(inputStr[i]  == 'J'){ memcpy(slicedChar, JUp, sizeof(JUp)); }  else if(inputStr[i]  == 'W'){ memcpy(slicedChar, WUp, sizeof(WUp)); } 
        else if(inputStr[i]  == 'j'){ memcpy(slicedChar, JLo, sizeof(JLo)); }  else if(inputStr[i]  == 'w'){ memcpy(slicedChar, WLo, sizeof(WLo)); } 
        else if(inputStr[i]  == 'K'){ memcpy(slicedChar, KUp, sizeof(KUp)); }  else if(inputStr[i]  == 'X'){ memcpy(slicedChar, XUp, sizeof(XUp)); } 
        else if(inputStr[i]  == 'k'){ memcpy(slicedChar, KLo, sizeof(KLo)); }  else if(inputStr[i]  == 'x'){ memcpy(slicedChar, XLo, sizeof(XLo)); } 
        else if(inputStr[i]  == 'L'){ memcpy(slicedChar, LUp, sizeof(LUp)); }  else if(inputStr[i]  == 'Y'){ memcpy(slicedChar, YUp, sizeof(YUp)); } 
        else if(inputStr[i]  == 'l'){ memcpy(slicedChar, LLo, sizeof(LLo)); }  else if(inputStr[i]  == 'y'){ memcpy(slicedChar, YLo, sizeof(YLo)); } 
        else if(inputStr[i]  == 'M'){ memcpy(slicedChar, MUp, sizeof(MUp)); }  else if(inputStr[i]  == 'Z'){ memcpy(slicedChar, ZUp, sizeof(ZUp)); } 
        else if(inputStr[i]  == 'm'){ memcpy(slicedChar, MLo, sizeof(MLo)); }  else if(inputStr[i]  == 'z'){ memcpy(slicedChar, ZLo, sizeof(ZLo)); } 
        
        else if(inputStr[i]  == ' '){ memcpy(slicedChar, SPACE, sizeof(SPACE)); }
         
        //uint8_t slicedChar[8];  // All 8 rows(array) of currently iterated char 

            for(int j = 0; j < 8; j++)
                {     
                tmpBit[j] = slicedChar[j];   // tmpBit tar emot 1 bitrow i taget från sliced char
                lcd.CreateChar(0, tmpBit);   // och printar 1 gång för varje rad den tar emot
                lcd.GoTo(i,0);
                lcd.WriteData(0); // 0 = tmpBit      // Varje iteration printar  tmpBit
                _delay_ms(250);
                }                     

        lcd.GoTo(i,0);
        lcd.WriteData(inputStr[i]);
        
                // CGRAM är global och behöver därför att jag ger ett annat värde till varje cell innan jag går vidare
    }
    
    // _delay_ms(5000);
    // Break row på bra ställe funktion
    // Blinka efter utskrift(ev. speedupblink)

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



    return 0;
}