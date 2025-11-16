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
    uint8_t bitmap[8];
}
Bitmaps;

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

void DiscoMan(HD44780 &lcd)
    {                                                                                         // Bitmaps:
    uint8_t dManR[8] =  { 0b00110,0b00110,0b01001,0b01011,0b00110,0b00110,0b00101,0b01101 };    // Discoman right position(startposition)   
    uint8_t dManL[8] =  { 0b01100,0b01100,0b10010,0b10110,0b01100,0b01100,0b01010,0b11011 };    // Discoman left position     
    uint8_t dManOpen[8] =  { 0b00011,0b00011,0b01001,0b01011,0b00110,0b00110,0b00101,0b01101 }; // Discoman opening   
    uint8_t dManFin[8] =  { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 };  // Discoman finale 
    uint8_t slicedCharz[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; // Blank bitmap for storing current j feed of pixelrows
 
    while(1){
            char strSpace[] = "                                "; // Array of 32 spaces for iterating and printing spaces 
            lcd.Clear(); 

                for(int i = 0; i < strlen(strSpace); i++)
                { printf("i count: %d \n",i);

                  int Rowz;  // Variable for controlling displayposition Y    

                    for(int j = 0; j < 8; j++)
                            {  
                                    printf("j: %d ",j); // Errorhandler 

                            if                (i<=28 && i%2==0) { memcpy(&slicedCharz[j], &dManR[j], sizeof(uint8_t)); }     // slicedCharz gets 1 pixel row from desired bitmap per iteration
                            else if(i <  15          && i%2==1) { memcpy(&slicedCharz[j], &dManOpen[j], sizeof(uint8_t)); }  // 1 byte/8 bits per pixelrow
                            else if(i >= 15 && i<=28 && i%2==1) { memcpy(&slicedCharz[j], &dManL[j], sizeof(uint8_t)); }    
                            else if(i> 28)                      { memcpy(&slicedCharz[j], &dManFin[j], sizeof(uint8_t)); }      
                            
                            
                            lcd.CreateChar(0, slicedCharz);   // Assign current iteration content to CGRAM(custom char mem) slot 0 
                            
                            if     (i%2==0) { Rowz = 0; lcd.GoTo(i+1,Rowz); }  // Move position 
                            else if(i%2==1) { Rowz = 1; lcd.GoTo(i+1,Rowz); } 
                            
                            lcd.WriteData(0);                 // Print slot 0
                            }

                                                                                               // Print in zic-zac pattern by:    
                    if     (i%2==0) { Rowz = 0; lcd.GoTo(i,Rowz); lcd.WriteData(strSpace[i]);}     // writing space on top row when cell count = even 
                    else if(i%2==1) { Rowz = 1; lcd.GoTo(i,Rowz); lcd.WriteData(strSpace[i]);}     // writing space on bottom row when cell count = odd        
                }                                                      
            }
    }
    
    //---------------------------------------------------------------START GetBitmap----------------------------------------------------------------------------------
void GetBitmap(char inputChar, char nextChar, uint8_t slicedChar[8]) 
{      
     //A                                                                                         //O                                                                                                                             
    uint8_t AUp[8] = {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b00000}; uint8_t OUp[8] = {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000};
    uint8_t ALo[8] = {0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b10011, 0b01101, 0b00000}; uint8_t OLo[8] = {0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000};
    // B                                                                                         //P                                                                                                                               
    uint8_t BUp[8] = {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110, 0b00000}; uint8_t PUp[8] = {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000, 0b00000};
    uint8_t BLo[8] = {0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b11110, 0b00000}; uint8_t PLo[8] = {0b00000, 0b00000, 0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000};
    // C                                                                                         //Q                                         
    uint8_t CUp[8] = {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110, 0b00000}; uint8_t QUp[8] = {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101, 0b00000};
    uint8_t CLo[8] = {0b00000, 0b01110, 0b10001, 0b10000, 0b10000, 0b10001, 0b01110, 0b00000}; uint8_t QLo[8] = {0b00000, 0b00000, 0b01111, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001};
    // D                                                                                         //R                                            
    uint8_t DUp[8] = {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110, 0b00000}; uint8_t RUp[8] = {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001, 0b00000};
    uint8_t DLo[8] = {0b00001, 0b00001, 0b01101, 0b10011, 0b10001, 0b10001, 0b01111, 0b00000}; uint8_t RLo[8] = {0b00000, 0b00000, 0b10110, 0b11001, 0b10000, 0b10000, 0b10000, 0b00000};
    // E                                                                                         //S                         
    uint8_t EUp[8] = {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111, 0b00000}; uint8_t SUp[8] = {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110, 0b00000};
    uint8_t ELo[8] = {0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b10001, 0b01110, 0b00000}; uint8_t SLo[8] = {0b00000, 0b00000, 0b01110, 0b10000, 0b01110, 0b00001, 0b11110, 0b00000};
    // F                                                                                         //T           
    uint8_t FUp[8] = {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000, 0b00000}; uint8_t TUp[8] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000};
    uint8_t FLo[8] = {0b00110, 0b01001, 0b01000, 0b11100, 0b01000, 0b01000, 0b01000, 0b00000}; uint8_t TLo[8] = {0b01000, 0b01000, 0b11100, 0b01000, 0b01000, 0b01001, 0b00110, 0b00000};
    // G                                                                                         //U                               
    uint8_t GUp[8] = {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110, 0b00000}; uint8_t UUp[8] = {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000};
    uint8_t GLo[8] = {0b00000, 0b01111, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110, 0b00000}; uint8_t ULo[8] = {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b10011, 0b01101, 0b00000};
    // H                                                                                         //V                                 
    uint8_t HUp[8] = {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b00000}; uint8_t VUp[8] = {0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100, 0b00000};
    uint8_t HLo[8] = {0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001, 0b00000}; uint8_t VLo[8] = {0b00000, 0b00000, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100, 0b00000};
    // I                                                                                         //W                           
    uint8_t IUp[8] = {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110, 0b00000}; uint8_t WUp[8] = {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010, 0b00000};
    uint8_t ILo[8] = {0b00100, 0b00000, 0b01100, 0b00100, 0b00100, 0b00100, 0b01110, 0b00000}; uint8_t WLo[8] = {0b00000, 0b00000, 0b10001, 0b10001, 0b10101, 0b10101, 0b01010, 0b00000};
    // J                                                                                         //X                                 
    uint8_t JUp[8] = {0b00111, 0b00001, 0b00001, 0b00001, 0b10001, 0b10001, 0b01110, 0b00000}; uint8_t XUp[8] = {0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b01010, 0b10001, 0b00000};
    uint8_t JLo[8] = {0b00010, 0b00000, 0b00110, 0b00010, 0b00010, 0b10010, 0b01100, 0b00000}; uint8_t XLo[8] = {0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b00000};
    // K                                                                                         //Y                              
    uint8_t KUp[8] = {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001, 0b00000}; uint8_t YUp[8] = {0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000};
    uint8_t KLo[8] = {0b10000, 0b10000, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b00000}; uint8_t YLo[8] = {0b00000, 0b00000, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110, 0b00000};
    // L                                                                                         //Z                                    
    uint8_t LUp[8] = {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111, 0b00000}; uint8_t ZUp[8] = {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111, 0b00000};
    uint8_t LLo[8] = {0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110, 0b00000}; uint8_t ZLo[8] = {0b00000, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111, 0b00000};                                                                                                 
    // M                                                                                         //Å
    uint8_t MUp[8] = {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001, 0b00000}; uint8_t AW[8] = {0b00100, 0b00000, 0b01110, 0b10001, 0b11111, 0b10001, 0b00000, 0b10001};
    uint8_t MLo[8] = {0b00000, 0b00000, 0b11010, 0b10101, 0b10101, 0b10101, 0b10101, 0b00000}; uint8_t aw[8] = {0b00100, 0b01010, 0b00100, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111};
    // N                                                                                         //Ä  
    uint8_t NUp[8] = {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b00000}; uint8_t AE[8] = {0b01010, 0b00000, 0b01110, 0b10001, 0b11111, 0b10001, 0b00000, 0b10001};
    uint8_t NLo[8] = {0b00000, 0b00000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001, 0b00000}; uint8_t ae[8] = {0b01010, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111, 0b00000};
                                                                                                 //Ö
                                                                                                uint8_t OO[8] = {0b01010, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000};
    /*SPACE*/                                                                                   uint8_t oo[8] = {0b01010, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000};
    uint8_t SPACE[8] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}; 

    {   
        if     (inputChar  == 'A'){ memcpy(slicedChar, AUp, sizeof(AUp)); }  else if(inputChar  == 'N'){ memcpy(slicedChar, NUp, sizeof(NUp)); } 
        else if(inputChar  == 'a'){ memcpy(slicedChar, ALo, sizeof(ALo)); }  else if(inputChar  == 'n'){ memcpy(slicedChar, NLo, sizeof(NLo)); }  
        else if(inputChar  == 'B'){ memcpy(slicedChar, BUp, sizeof(BUp)); }  else if(inputChar  == 'O'){ memcpy(slicedChar, OUp, sizeof(OUp)); } 
        else if(inputChar  == 'b'){ memcpy(slicedChar, BLo, sizeof(BLo)); }  else if(inputChar  == 'o'){ memcpy(slicedChar, OLo, sizeof(OLo)); }
        else if(inputChar  == 'C'){ memcpy(slicedChar, CUp, sizeof(CUp)); }  else if(inputChar  == 'P'){ memcpy(slicedChar, PUp, sizeof(PUp)); } 
        else if(inputChar  == 'c'){ memcpy(slicedChar, CLo, sizeof(CLo)); }  else if(inputChar  == 'p'){ memcpy(slicedChar, PLo, sizeof(PLo)); }
        else if(inputChar  == 'D'){ memcpy(slicedChar, DUp, sizeof(DUp)); }  else if(inputChar  == 'Q'){ memcpy(slicedChar, QUp, sizeof(QUp)); } 
        else if(inputChar  == 'd'){ memcpy(slicedChar, DLo, sizeof(DLo)); }  else if(inputChar  == 'q'){ memcpy(slicedChar, QLo, sizeof(QLo)); }
        else if(inputChar  == 'E'){ memcpy(slicedChar, EUp, sizeof(EUp)); }  else if(inputChar  == 'R'){ memcpy(slicedChar, RUp, sizeof(RUp)); } 
        else if(inputChar  == 'e'){ memcpy(slicedChar, ELo, sizeof(ELo)); }  else if(inputChar  == 'r'){ memcpy(slicedChar, RLo, sizeof(RLo)); }
        else if(inputChar  == 'F'){ memcpy(slicedChar, FUp, sizeof(FUp)); }  else if(inputChar  == 'S'){ memcpy(slicedChar, SUp, sizeof(SUp)); } 
        else if(inputChar  == 'f'){ memcpy(slicedChar, FLo, sizeof(FLo)); }  else if(inputChar  == 's'){ memcpy(slicedChar, SLo, sizeof(SLo)); }
        else if(inputChar  == 'G'){ memcpy(slicedChar, GUp, sizeof(GUp)); }  else if(inputChar  == 'T'){ memcpy(slicedChar, TUp, sizeof(TUp)); } 
        else if(inputChar  == 'g'){ memcpy(slicedChar, GLo, sizeof(GLo)); }  else if(inputChar  == 't'){ memcpy(slicedChar, TLo, sizeof(TLo)); }
        else if(inputChar  == 'H'){ memcpy(slicedChar, HUp, sizeof(HUp)); }  else if(inputChar  == 'U'){ memcpy(slicedChar, UUp, sizeof(UUp)); } 
        else if(inputChar  == 'h'){ memcpy(slicedChar, HLo, sizeof(HLo)); }  else if(inputChar  == 'u'){ memcpy(slicedChar, ULo, sizeof(ULo)); }
        else if(inputChar  == 'I'){ memcpy(slicedChar, IUp, sizeof(IUp)); }  else if(inputChar  == 'V'){ memcpy(slicedChar, VUp, sizeof(VUp)); } 
        else if(inputChar  == 'i'){ memcpy(slicedChar, ILo, sizeof(ILo)); }  else if(inputChar  == 'v'){ memcpy(slicedChar, VLo, sizeof(VLo)); }  
        else if(inputChar  == 'J'){ memcpy(slicedChar, JUp, sizeof(JUp)); }  else if(inputChar  == 'W'){ memcpy(slicedChar, WUp, sizeof(WUp)); } 
        else if(inputChar  == 'j'){ memcpy(slicedChar, JLo, sizeof(JLo)); }  else if(inputChar  == 'w'){ memcpy(slicedChar, WLo, sizeof(WLo)); } 
        else if(inputChar  == 'K'){ memcpy(slicedChar, KUp, sizeof(KUp)); }  else if(inputChar  == 'X'){ memcpy(slicedChar, XUp, sizeof(XUp)); } 
        else if(inputChar  == 'k'){ memcpy(slicedChar, KLo, sizeof(KLo)); }  else if(inputChar  == 'x'){ memcpy(slicedChar, XLo, sizeof(XLo)); } 
        else if(inputChar  == 'L'){ memcpy(slicedChar, LUp, sizeof(LUp)); }  else if(inputChar  == 'Y'){ memcpy(slicedChar, YUp, sizeof(YUp)); } 
        else if(inputChar  == 'l'){ memcpy(slicedChar, LLo, sizeof(LLo)); }  else if(inputChar  == 'y'){ memcpy(slicedChar, YLo, sizeof(YLo)); } 
        else if(inputChar  == 'M'){ memcpy(slicedChar, MUp, sizeof(MUp)); }  else if(inputChar  == 'Z'){ memcpy(slicedChar, ZUp, sizeof(ZUp)); } 
        else if(inputChar  == 'm'){ memcpy(slicedChar, MLo, sizeof(MLo)); }  else if(inputChar  == 'z'){ memcpy(slicedChar, ZLo, sizeof(ZLo)); } 
        else if((uint8_t)inputChar == 0xC3 && (uint8_t)nextChar == 0x85) { memcpy(slicedChar, AW, sizeof(AW)); }
        else if((uint8_t)inputChar == 0xC3 && (uint8_t)nextChar == 0x84) { memcpy(slicedChar, aw, sizeof(aw)); }
        else if((uint8_t)inputChar == 0xC3 && (uint8_t)nextChar == 0x96) { memcpy(slicedChar, AE, sizeof(AE)); }
        else if((uint8_t)inputChar == 0xC3 && (uint8_t)nextChar == 0xA5) { memcpy(slicedChar, ae, sizeof(ae)); }
        else if((uint8_t)inputChar == 0xC3 && (uint8_t)nextChar == 0xA4) { memcpy(slicedChar, OO, sizeof(OO)); }
        else if((uint8_t)inputChar == 0xC3 && (uint8_t)nextChar == 0xB6) { memcpy(slicedChar, oo, sizeof(oo)); }
        else if(inputChar == ' '){ memcpy(slicedChar, SPACE, sizeof(SPACE)); }
        else { memcpy(slicedChar, SPACE, sizeof(SPACE)); }
    }    
}
//-----------------------------------------------------------------END GetBitmap-------------------------------------------------------------------

//---------------------------------------------------------------START FadeInString----------------------------------------------------------------------------------
void FadeInString(HD44780 &lcd,char *inputStr) // Expects char[] for input
{
    uint8_t slicedChar[8] = { 0 };  
      
    for(int i = 0; i < strlen(inputStr); i++)   // loopar över string input ex "långben etc....
    {
        uint8_t tmpBit[8] = { 0 };
        GetBitmap(inputStr[i],inputStr[i+1],slicedChar);  // Expects string[i], string[i+1], uint8_t[8]
        
        int Row; int Step = 16;                       //Position variables: Row = Y Step = X
        
        for(int j = 0; j < 8; j++)              
        {    
            tmpBit[j] = slicedChar[j];      // tmpBit recieves 1 pixelrow(per iteration) from sliced char
            lcd.CreateChar(0, tmpBit);     // Stores current state of char to CGRAM(custom char lib), slot 0             
            if     (i<16)  {Row = 0; lcd.GoTo(i,Row); }     // Forces WriteData to remain on current  
            else if(i>=16) {Row = 1; lcd.GoTo(i-Step,Row);}   // pos until all pixelrows are printed
            lcd.WriteData(0);             // Prints tmpBit
        }    
        
        if     (i<16)  {Row = 0; lcd.GoTo(i,Row); }
        else if(i>=16) {Row = 1; lcd.GoTo(i-Step,Row); }
        lcd.WriteData(inputStr[i]);  // Prints char from stringarray        
    }                                                      
}        
//-----------------------------------------------------------------END FadeInString-------------------------------------------------------------------


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

    Customer customers[5]; // 
    
    customers[3].paid = 4000;                   
    customers[3].messageCount = 2;
   
    
    strcpy(customers[3].messages[0].AdMessage,"Mysterier? Ring Långben");  // 9500-11499
    strcpy(customers[3].messages[1].AdMessage,"Långben fixar biffen");     // 11500-13499 
    
   
    srand(time(NULL));
    
    
    
    CGRAM(customers[3].messages[0].AdMessage);
    lcd.Clear();
    
    DiscoMan(lcd);
    
    FadeInString(lcd,customers[3].messages[0].AdMessage);
    





//-----------------------------------------------------------MINA BITMAPS----------------------------------------------------------------------------
// Rita figurer här
//         Discoman-Start(Rshift)        Discoman-Start(Lshift)       Discoman-Start(Prepos) HappyFace                  Sombreroman            Chafa MickeyMouse
//       0    00110                         01100                        01100                 00000                11110                    01010               
//       1    00110                         01100                        01100                 01010                01100                    01110                
//       2    01001                         10010                        10010                 10001                10010                    10100             
//       3    01011                         10110                        10110                 00000                10010                    01100             
//       4    00110                         01100                        01100                 10001                01100                    00110             
//       5    00110                         01100                        01100                 11011                01100                    01110                
//       6    00101                         01010                        01010                 01010                01010                    01010             
//       7    01101                         11011                        11011                 01110                11011                    11010             
//-----------------------------------------------------------MINA BITMAPS----------------------------------------------------------------------------




    return 0;
}





















// inputstring tar emot strängen
// iterera över sträng för att hitta vilken bitmap man vill ha
// slicedchar hämtar hel bitmap(ex Aa)
// slicedchar ger bittmp 1 rad per loop
// bitmap printar rad och sparar den
//__________________HÄR___________________
    // uint8_t bitPix[8] = { 0b10010,0b10010,0b10010,0b00000,0b00000,0b00000,0b00000,0b00000 };
    // lcd.CreateChar(0, bitPix); // Assign to CGRAM slot 0
    
    // uint8_t pixel[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; //slicechar
    // uint8_t pixelTmp[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; //tmpBit
    
    // char inputStr[] = {0,0,0,0,0,0,0,0,'\0'};
    // //for(int j = 0; j<2; j++) 
    // for(int i = 0; i < strlen(inputStr); i++)    
    // {    
    // if     (inputStr[i]  == 0)memcpy(pixel, bitPix, sizeof(bitPix));                                    
    // int Row; int Step = 16;                  

    //         for(int j = 0; j < 8; j++)
    //         {
    //             pixelTmp[j] = pixel[j];
    //             lcd.CreateChar(0, pixelTmp); // Assign to CGRAM slot 0
    //             _delay_ms(400);
    //             lcd.WriteData(0);       // print pixel 
    //             // pixel[j] = pixel[j] >> 1;   // Shift bit 1 step right
    //             // pixel[j+1] = pixel[j]; // Shift element 1 step right

    //         }
    //         lcd.WriteData(inputStr[i]);
    //}//__________________HÄR___________________
   // Långben fixar biffen

    // Iteration for printing singular row masks of lcd bitmaps
 
//void GetBitMap(char *inputStr){}
    //     char *inputStr = customers[3].messages[0].AdMessage; 
    //     uint8_t tmpBit[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; // Blank custom char assigned to CGRAM-slot 0
    //     uint8_t slicedChar[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; // Blank custom char

    // for(int i = 0; i < strlen(inputStr); i++)// om element 1 = A skickar vi hela A-Bitmapen till slicedchar
    // {                                                                                                                     
    // int Row; int step = 16;                         
        
    //         for(int j = 0; j < 8; j++)
    //             {    
    //             tmpBit[j] = slicedChar[j];   // tmpBit tar emot 1 bitpixel i taget från sliced char
    //             lcd.CreateChar(0, tmpBit);   // och printar 1 gång för varje rad den tar emot
    //             if     (i<16) {Row = 0; lcd.GoTo(i,pixel); }
    //             else if(i>=16) {Row = 1; lcd.GoTo(i-step,pixel);}
    //             lcd.WriteData(0); // 0 = tmpBit      // Varje iteration printar  tmpBit
    //             _delay_ms(5);
    //             }    

    //     // if     (i<16) {Row = 0; lcd.GoTo(i,Row); }
    //     // else if(i>=16) {Row = 1; lcd.GoTo(i-step,Row);}
    //     lcd.WriteData(inputStr[i]);
        
    //             // CGRAM är global och behöver därför att jag ger ett annat värde till varje cell innan jag går vidare
    // }


    //------------------------------------------------fungerande backup--------------------------------------------

    //  char strSpace[] = "                                "; // 32 space att iterera över

    // char *strSpace = strSpace; 

    // uint8_t tmpBitz[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; // Blank bitmask array where j char gets stored every loop
    // uint8_t slicedCharz[8] = { 0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000 }; // Blank bitmask array where 1 element of string gets stored
    // // sliceChars tilldelas hela bitmapen och matar 1 pixelrad i taget till tmpBitz som då blir raderna den tilldelats och resten 00000.
    // for(int i = 0; i < strlen(strSpace); i++)// 

    // { printf("CHAR count: %d \n",i);


        // int Rowz;     
        
        // for(int j = 0; j < 8; j++)
                // {  
                
                                    
                // if                (i<28 && i%2==0) {memcpy(slicedCharz[j], dManR, sizeof(dManL));} // Ger bitmap dMan pos right uint8 till slicedCharz
                // else if(i <  13         && i%2==1) {memcpy(slicedCharz, dManPre, sizeof(dManPre));} // Ger bitmap dMan prephase uint8 till slicedCharz     
                // else if(i >= 13 && i<28 && i%2==1) {memcpy(slicedCharz, dManL, sizeof(dManR));} // Ger bitmap dMan pos left uint8 till slicedCharz     
                // else if(          i>=27 ) {memcpy(slicedCharz, dManFin, sizeof(dManL));} // Ger bitmap dMan pos left uint8 till slicedCharz     

                // tmpBitz[j] = slicedCharz[j];   // tmpBit tar emot 1 bitrow[j] i taget från sliced char
                // lcd.CreateChar(0, &tmpBitz);   // tar hela tmpBitz, därför måste tmpBitz få uppdaterad bitmap varje iteration

                // if     (i%2==0 || j == 8) {Rowz = 0; lcd.GoTo(i+1,Rowz); /*dManR[j] <<= 1;*/} // itererar och bitshiftar alla pixelrader konsekutivt åt vänster
                // else if(i%2==1 || j == 8) {Rowz = 1; lcd.GoTo(i+1,Rowz); /*dManR[j] >>= 1;*/} // itererar och bitshiftar alla pixelrader konsekutivt åt höger

                // lcd.WriteData(0); // 0 = tmpBit      // Varje iteration printar  tmpBit
                // }
                
                
        
        // if(i%2==0) 
        // {Rowz = 0; lcd.GoTo(i,Rowz); 
        // lcd.WriteData(strSpace[i]);}    // skriver blanksteg på varannan
        // else if(i%2==1) 
        // {Rowz = 1; lcd.GoTo(i,Rowz); 
        // lcd.WriteData(strSpace[i]);}                
    // }                                         


//---------------------------VARFÖR-FUNKAR-INTE???-------------------------------------------
    // uint8_t dManR[8] =  { 0b00110,0b00110,0b01001,0b01011,0b00110,0b00110,0b00101,0b01101 };

    // for(int i = 0; i < 32; i++)
    //     {

    //         for(int j = 0; j < 8; j++)
    //            {


    //             memcpy(&slicedCharz[j], &dManR[j], sizeof(uint8_t));

    //             lcd.CreateChar(0,slicedCharz);  
                
    //             _delay_ms(500);

    //             lcd.WriteData(0);
    //            }

    //     }
//---------------------------VARFÖR-FUNKAR-INTE???-------------------------------------------