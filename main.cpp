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

// https://wokwi.com/projects/416241646559459329

#define LED_PIN 2
#define BUTTON_PIN 1

#define BIT_SET(a, b) ((a) |= (1ULL << (b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b)))) 
#define BUTTON_IS_CLICKED(PINB,BUTTON_PIN) !BIT_CHECK(PINB,BUTTON_PIN)

int main(void){
    srand(time(NULL)); //random seed, prevents the same sequence every time the program runs
    init_serial();
    HD44780 lcd;
    lcd.Initialize(); // Initialize the LCD

    // create all (5) customers.
    Customer user[5];
    createCustomers(user);

    // get the sum of all payment (used for rand)
    int sum = totalPaid(user);

    int userToPresent = -1;

    while(1){

       // pick a random customer
        userToPresent = randomCustomer(user, sum, userToPresent);
        int textIndex = rand() % user[userToPresent].messagesCount;

        const char* msg = user[userToPresent].message[textIndex].message;

        printf("Now presenting: %d | Text id: %d\n", userToPresent, textIndex);

        typeAnimation(lcd, msg);  // type + sweep animation
        // SCROLL FUNCTION + delay
        // int cnt = 0;

        // // two 'complete scrolls'
        // while (cnt < 2){
        //     cnt++;
        //     char *txt = user[userToPresent].message[textIndex].message;

        //     // scroll 15 steps each.
        //     for (int i = 0; i < 15; i++){
        //         // Clear the LCD
        //         lcd.Clear();      

        //         // write LCD text
        //         lcd.WriteText(txt+i);
                    
        //         _delay_ms(485);
        //     }        
        // }
    }
    return 0;
}