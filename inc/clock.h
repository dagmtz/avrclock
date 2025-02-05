#ifndef CLOCK_H_
#define CLOCK_H_

#include <avr/io.h>

#include <stdint.h>

#include <time.h>

#include <util/delay.h>

#define DELAY_PER_DIGIT 1U

#define DISPLAY_DIGITS_PORT PORTB
#define DISPLAY_USS PORTB5
#define DISPLAY_DSS PORTB4
#define DISPLAY_UMM PORTB3
#define DISPLAY_DMM PORTB2
#define DISPLAY_UHH PORTB1
#define DISPLAY_DHH PORTB0

#define DISPLAY_DECODER_PORT PORTC
#define BDC_7S_A PORTC0
#define BDC_7S_B PORTC1
#define BDC_7S_C PORTC2
#define BDC_7S_D PORTC3

#define BUTTONS_PORT PORTD
#define BUTTON_1 PORTD2
#define BUTTON_1_MASK 0x04 /* 0b00000100 */
#define BUTTON_2 PORTD3
#define BUTTON_2_MASK 0x08 /* 0b00001000 */
#define BUTTON_3 PORTD4
#define BUTTON_3_MASK 0x10 /* 0b00010000 */

#define BUTTONS_MASK 0x1C /* 0b00011100 */

typedef struct digits {
    uint8_t d1;
    uint8_t d2;
    uint8_t d3;
    uint8_t d4;
    uint8_t d5;
    uint8_t d6;
} digits__s;

typedef enum clock_state_tag {
    show_time, 
    set_time, 
    show_date, 
    set_date, 
    set_alarm,
    dim
}clock_state__e;

typedef enum clock_buttons_tag {
    invalid,
    function,
    down,
    up
}clock_buttons__e;

void split_time(digits__s *digits, struct tm *time);
void split_date(digits__s *digits, struct tm *time);
void turn_off_digits(void);
void turn_on_digit(uint8_t);
void show_digit(uint8_t);
void display(digits__s *digits);
void read_buttons(void);

#endif /* CLOCK_H_ */
