#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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
    char message[36];
} Message;

typedef struct {
    Message* messages;
    int messageCount;
    int paid;
} Customer;

/*
\xE0 -> å

\xE1 -> ä

\xEF -> ö
*/

    // Hederlige Harrys Bilar
Message harryMessages[] = {
    {"K\xEFp bil hos Harry"},
    {"En god bilaff\xE1r (f\xEFr Harry!)"},
    {"Hederlige Harrys Bilar"}
};
Customer harry = {harryMessages, 3, 5000}; //messages array, number of messages, paid amount
    
// Farmor Ankas Pajer AB
Message ankaMessages[] = {
    {"K\xEFp paj hos Farmor Anka"},
    {"Skynda innan M\xE0rten \xE1tit alla pajer"}
};
Customer anka = {ankaMessages, 2, 3000};

// Svarte Petters Svartbyggen
Message petterMessages[] = {
    {"L\xE0t Petter bygga \xE0t dig"},
    {"Bygga svart? Ring Petter"}
};
Customer petter = {petterMessages, 2, 1500};

// Långbens detektivbyrå
Message langbenMessages[] = {
    {"Mysterier? Ring L\xE0ngben"},
    {"L\xE0ngben fixar biffen"}
};
Customer langben = {langbenMessages, 2, 4000};

// IOT:s Reklambyrå
Message iotMessages[] = {
    {"Synas h\xE1r? IOT:s Reklambyr\xE0"}
};
Customer iot = {iotMessages, 1, 1000};

//instead of hardcoding inside elif, cleaner code
int customer1 = harry.paid;
int customer2 = customer1 + anka.paid;
int customer3 = customer2 + petter.paid;
int customer4 = customer3 + langben.paid;


int lastCustomer = -1; //no customer selected at start

int main(void){
    init_serial();
    HD44780 lcd;

    lcd.Initialize(); // Initialize the LCD
    lcd.Clear();      // Clear the LCD

    while (1) {
        Customer* selectedCustomer;
        int customerIndex;
        lastCustomer = customerIndex;

        do {
//===========testing with more characters than screen can show============
            // lcd.WriteText(ankaMessages[1].message);
            // _delay_ms(5000);
            
            int r = rand() % 14500; //generate random number between 0 and total paid amount (14500)
            printf("Random number: %d\n", r); //display generated number for debugging (and curiosity)
            if (r < customer1) {
                selectedCustomer = &harry;
                customerIndex = 0;
            } 
            else if (r < customer2) {
                selectedCustomer = &anka;
                customerIndex = 1;
            } 
            else if (r < customer3) {
                selectedCustomer = &petter;
                customerIndex = 2;
            } 
            else if (r < customer4) {
                selectedCustomer = &langben;
                customerIndex = 3;
            } 
            else {
                selectedCustomer = &iot;
                customerIndex = 4;
            }
        } while (customerIndex == lastCustomer);//prevents repeating the same customer twice in a row

        int messageIndex = rand() % selectedCustomer->messageCount;
        char* message = selectedCustomer->messages[messageIndex].message;
        
        lcd.WriteText(message);

        _delay_ms(5000); //TEMPORARY DELAY SO I DONT HAVE TO WAIT 20 FKJQWEBNFLQKWE SECONDS FOR THE MESSAGE TO CHANGE
        lcd.Clear();
    }
return 0;
}
        