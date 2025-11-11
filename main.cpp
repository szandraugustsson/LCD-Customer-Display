#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h> //rand()
#include "lcd.h"
#include "uart.h"

// https://wokwi.com/projects/416241646559459329

// PORTB B (digital pin 8 to 13)
// C (analog input pins)
// PORTD D (digital pins 0 to 7)
#define LED_PIN 2
#define BUTTON_PIN 1

#define BIT_SET(a, b) ((a) |= (1ULL << (b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b)))) 

#define BUTTON_IS_CLICKED(PINB,BUTTON_PIN) !BIT_CHECK(PINB,BUTTON_PIN)

typedef struct{
    char message[50];
} Message;

typedef struct{
    Message messages[3];
    int paid;
    int messageCount;
} Customer;

int main(void){
    
    Customer customers[5];

    customers[0].paid = 5000; //Hederlige Harrys Bilar
    strcpy(customers[0].messages[0].message, "Köp bil hos Harry");
    strcpy(customers[0].messages[1].message, "En god bilaffär (för Harry!)");
    strcpy(customers[0].messages[2].message, "Hederlige Harrys Bilar");
    customers[0].messageCount = 3;

    customers[1].paid = 3000; //Farmor Ankas Pajer AB
    strcpy(customers[1].messages[0].message, "Köp paj hos Farmor Anka");
    strcpy(customers[1].messages[1].message, "Skynda innan Mårten ätit alla pajer");
    customers[1].messageCount = 2;

    customers[2].paid = 1500; //Svarte Petters Svartbyggen
    strcpy(customers[2].messages[0].message, "Låt Petter bygga åt dig");
    strcpy(customers[2].messages[1].message, "Bygga svart? Ring Petter");
    customers[2].messageCount = 2;

    customers[3].paid = 4000; //Långbens detektivbyrå
    strcpy(customers[3].messages[0].message, "Mysterier? Ring Långben");
    strcpy(customers[3].messages[1].message, "Långben fixar biffen");
    customers[3].messageCount = 2;

    customers[4].paid = 1000; //för oss själva
    strcpy(customers[4].messages[0].message, "Synas här? IOT:s Reklambyrå");
    customers[4].messageCount = 1;

    init_serial();
    HD44780 lcd;

    lcd.Initialize(); // Initialize the LCD
    lcd.Clear();      // Clear the LCD

    lcd.WriteText((char *)"Hej hej");
    printf("Hej hej\n");
    int r = 12;
    printf("Hej 2 %d\n",r);
    // // //Sätt till INPUT_PULLUP
    // BIT_CLEAR(DDRB,BUTTON_PIN); // INPUT MODE
    // BIT_SET(PORTB,BUTTON_PIN); 

    // DATA DIRECTION = avgör mode
    // om output så skickar vi 1 eller 0 på motsvarande pinne på PORT
    // om input så läser vi  1 eller 0 på motsvarande pinne på PIN
    //bool blinking = false;

    //deklarerar variabeln utanför while-loopen för att det ska minnas
    int lastCustomer;
/*

ewrg
werg
rweg

this is a comment to see what is changed
after editing in github codespace

werg
werg
werg
we
rgwe
rgewr
g
werg
wer
g
*/
    while(1)
    {
        // Get a customer!
        // Ta alla kunder och summera deras betalningar
        // harry har 5000 lotter (0-4999)
        // petter har 2000 lotter (5000-6999)
        // kajsa har 1000 lotter (7000-7999)
        // slumpa fram ett nummer mellan 0 och 7999
        // kolla vilken kund som har det numret

        // Summera betalningar så att vi ska veta maxvärdet
        int sumPaid = 0;
        for (int i = 0; i < 5; i++)
            sumPaid += customers[i].paid;

        // Slumpa fram ett nummer mellan 0 och summan av betalningar
        int randNum = rand() % sumPaid;

        // för att veta vilken kund randNum tillhör
        int selectedCustomer;
        int sum = 0;
        for (int i = 0; i < 5; i++) 
        {
            sum += customers[i].paid;
            if (randNum < sum)
            {
                selectedCustomer = i;
                break;
            }
        }

        //om det slumpar fram samma kund, ska det börja om loopen
        if (selectedCustomer == lastCustomer)
        {
            continue;
        }
        
        // Slumpa fram ett index från selectedCustomer och skriva ut meddelandet som ligger på index messageIndex
        int messageIndex = rand() % customers[selectedCustomer].messageCount;

        lcd.Clear(); //rensa hela display innan ny text
        lcd.WriteText(customers[selectedCustomer].messages[messageIndex].message);
        printf("%s\n", customers[selectedCustomer].messages[messageIndex]);

        // vänta 20 sekunder (20 000 ms)
        // maxvärdet beror på CPU-hastigheten 16 MHz på Arduino Uno
        // _delay_ms() klarar bara 262 ms per anrop, därför använda for loop
        for (int i = 0; i < 100; i++)
        {
            _delay_ms(200); //på AVR-mikrokontroller kan man inte använda sleep()
        }

        lastCustomer = selectedCustomer;
    }
    return 0;
}
