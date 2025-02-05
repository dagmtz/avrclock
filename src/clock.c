
#include <clock.h>

void split_time(digits__s *digits, struct tm *time){
    digits->d6 = (uint8_t)(time->tm_hour / 10U);
    digits->d5 = (uint8_t)(time->tm_hour % 10U);
    
    digits->d4 = (uint8_t)(time->tm_min / 10U);
    digits->d3 = (uint8_t)(time->tm_min % 10U);

    digits->d2 = (uint8_t)(time->tm_sec / 10U);
    digits->d1 = (uint8_t)(time->tm_sec % 10U);
}

void split_date(digits__s *digits, struct tm *time){
    digits->d6 = (uint8_t)(time->tm_mday / 10U);
    digits->d5 = (uint8_t)(time->tm_mday % 10U);
    
    digits->d4 = (uint8_t)(time->tm_mon / 10U);
    digits->d3 = (uint8_t)(time->tm_mon % 10U);

    digits->d2 = (uint8_t)((time->tm_year - 100U) / 10U);
    digits->d1 = (uint8_t)((time->tm_year - 100U) % 10U);
}

void turn_off_digits()
{
    DISPLAY_DIGITS_PORT = 0U;
}

void turn_on_digit(uint8_t digit)
{
    DISPLAY_DIGITS_PORT = (1U << digit);
    _delay_ms(DELAY_PER_DIGIT);
    turn_off_digits();
}

void show_digit(uint8_t digit)
{
    DISPLAY_DECODER_PORT = ((DISPLAY_DECODER_PORT & 0xF0) | (digit & 0x0F));
}

void display(digits__s *digits){
    show_digit(digits->d1);
    turn_on_digit(5U);
    show_digit(digits->d2);
    turn_on_digit(4U);
    show_digit(digits->d3);
    turn_on_digit(3U);
    show_digit(digits->d4);
    turn_on_digit(2U);
    show_digit(digits->d5);
    turn_on_digit(1U);
    show_digit(digits->d6);
    turn_on_digit(0U);
}

void read_buttons(){
    static clock_buttons__e current_button = invalid;
    static clock_buttons__e last_button = invalid;
    static uint16_t button_timer = 0;

    switch(PIND & BUTTONS_MASK){
        case BUTTON_1_MASK:
            current_button = function;
        break;

        case BUTTON_2_MASK:
            current_button = down;
        break;

        case BUTTON_3_MASK:
            current_button = up;
        break;

        default:
            current_button = invalid;
            button_timer = 0;
        break;
    }

    if(last_button == current_button){
        button_timer++;
    }else{
        button_timer = 0;
        current_button = invalid;
    }

    last_button = current_button;
}
