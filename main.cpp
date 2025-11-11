#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"
#include "uart.h"

// https://wokwi.com/projects/416241646559459329

#define LED_PIN 2
#define BUTTON_PIN 1

#define BIT_SET(a, b) ((a) |= (1ULL << (b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b)))) 
#define BUTTON_IS_CLICKED(PINB,BUTTON_PIN) !BIT_CHECK(PINB,BUTTON_PIN)

typedef struct{
    char message[40];
}Message;

typedef struct{   
    Message message[3];
    int messagesCount;
    int paid;
}Customer;


int main(void){

    init_serial();
    HD44780 lcd;
    lcd.Initialize(); // Initialize the LCD

    // create all (5) customers.
    Customer user[5];

    //Hedelige Harry
    user[0].paid = 5000;
    user[0].messagesCount = 3;
    strcpy(user[0].message[0].message, "K\xEFp bil hos Harry");
    strcpy(user[0].message[1].message, "En god bilaff\xE1r (f\xEFr Harry!)");
    strcpy(user[0].message[2].message, "Hederlige Harrys Bilar");

    //Farmor Anka
    user[1].paid = 3000;
    user[1].messagesCount = 2;
    strcpy(user[1].message[0].message, "K\xEFp paj hos Farmor Anka");
    strcpy(user[1].message[1].message, "Skynda innan Marten \xE1tit alla pajer");

    //Svarte Petter
    user[2].paid = 1500;
    user[2].messagesCount = 2;
    strcpy(user[2].message[0].message, "Lat Petter bygga at dig");
    strcpy(user[2].message[1].message, "Bygga svart? Ring Petter");

    //Långbens detektivbyrå
    user[3].paid = 4000;
    user[3].messagesCount = 2;
    strcpy(user[3].message[0].message, "Mysterier? Ring Langben");
    strcpy(user[3].message[1].message, "Langben fixar biffen");
    
    //IoTs reklambyrå
    user[4].paid = 1000;
    user[4].messagesCount = 1;
    strcpy(user[4].message[0].message, "Synas h\xE1r? IOT:s Reklambyra");

    // get the sum of all payment (used for rand)
    int sum = 0;
    for (int i = 0; i < 5; i++){
        sum += user[i].paid;
    }

    int userToPresent = NULL;

    while(1){

        // get a NEW random customer
        int lastPresented = userToPresent;
        while (userToPresent == lastPresented){

            // retunerar: 0-14499.
            int randCustomer = rand() % sum; 
            int paidSoFar = 0;
            
            for (int i = 0; i < 5; i++){
                paidSoFar += user[i].paid;

                // om randCustomer är mindre än paidSoFar - lagras userToPresent.
                if (randCustomer < paidSoFar){
                    userToPresent = i;
                    break;
                }
            }
        }
        
        // get a random text (index)
        int textIndex = rand() % user[userToPresent].messagesCount;
        printf("Now presenting: %d | Text id: %d\n", userToPresent, textIndex);


        // SCROLL FUNCTION + delay
        int cnt = 0;

        // two complete scrolls
        while (cnt < 2){
            cnt++;
            char *txt = user[userToPresent].message[textIndex].message;

            // scroll 20 steps
            for (int i = 0; i < (strlen(user[userToPresent].message[textIndex].message)+15); i++){

                if (i < 15){
                    // set LCD position
                    lcd.GoTo(15-i,0);
                } else {
                    // Clear the LCD
                    lcd.Clear();
                    txt++;
                }

                // write LCD text
                lcd.WriteText((char *)txt);
                _delay_ms(150);
            
            }
        }
    }
    return 0;
}