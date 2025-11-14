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

// https://wokwi.com/projects/416241646559459329

#define LED_PIN 2
#define BUTTON_PIN 1
#define DISPLEN 16

#define BIT_SET(a, b) ((a) |= (1ULL << (b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b)))) 
#define BUTTON_IS_CLICKED(PINB,BUTTON_PIN) !BIT_CHECK(PINB,BUTTON_PIN)

int main(void){

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
        userToPresent = randomCustomer(user, sum, userToPresent);
        
        // get a random text (index)
        int textIndex = rand() % user[userToPresent].messagesCount;
        printf("Now presenting: %d | Text id: %d\n", userToPresent, textIndex);

        // SCROLL FUNCTION + delay
        int cnt = 0;

        // two 'complete scrolls'
        while (cnt < 2){
            cnt++;
            char txt_R[45];
            char *txt_L = user[userToPresent].message[textIndex].message;

            // scroll to txt ends (roll out of screen)
            for (int i = 0; i < (strlen(txt_L)+DISPLEN+1); i++){

                if (i <= DISPLEN){
                    snprintf(txt_R,i+1,txt_L);
                    lcd.GoTo(DISPLEN-i,0);
                    lcd.WriteText((char *)txt_R);    

                } else {
                    lcd.Clear();
                    lcd.WriteText((char *)txt_L+i-DISPLEN);
                }
                _delay_ms(130);
            }
        }
    }
    return 0;
}