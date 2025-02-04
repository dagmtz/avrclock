/*
 * File:            main.c
 * Author:          Daniel Martinez
 *                  dagmtzs@gmail.com
 * Date:            Mon Feb  3 01:13:46 AM CST 2025
 * Target:          ATmega328P
 * Description:     This is a clock :)
 */

/************************************************
 *       << Area for includes >>                *
 ************************************************/
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include <util/delay.h>

#include <clock.h>
#include <usart.h>
#include <i2c.h>
#include <ds3231.h>

/************************************************
 *       << Area for macro definitions >>       *
 ************************************************/

/* Size for usartBuffer used in main() */
#define BUFFER_SIZE 30U

/* Select heartbeat source via macro: 0 - No source, 1 - Internal int., 2 - External int. */
#define __HEARTBEAT_SOURCE 1

/* Select time source via macro: 0 - No source, 1 - Internal EEPROM, 2 - External RTC */
#define __TIME_SOURCE 1

/* EEPROM address where time is stored */
#define EEPROM_TIME_ADDR 0x40

/* Uncomment next line if you want to set up the time into EEPROM once, then compile and flash, then comment back and compile and flash again.*/
/* #define EEPROM_TIME_INIT */

#define __HALT while(1U){;}
#define UART_INFO uart_puts_P("\e[1;33m[INFO]: ")
#define UART_ERROR uart_puts_P("\e[1;31m[ERROR]: ")
#define UART_FCLEAN uart_puts_P("\e[0m")

/************************************************
 *       << Area for globals >>                 *
 ************************************************/
volatile bool g_heartbeat_1s = false;
volatile time_t __time = 0U;

#ifdef EEPROM_TIME_INIT
static const struct tm __initial_time_tm = {.tm_year = 125U, .tm_mon = 1U, .tm_mday = 3U, .tm_hour = 17U, .tm_min = 35U, .tm_sec = 0U};
static const time_t __initial_time_tag = 791683200U;
#endif

/************************************************
 *       << Area for custom datatypes >>        *
 ************************************************/


/************************************************
 *       << Area for function prototypes >>     *
 ************************************************/
void initialize(void);


/************************************************
 *       << Main function >>                    *
 ************************************************/
void main(void){	    

    static time_t system_time = 0U;
    static struct tm rtc_time = {0U}; 
    static digits__s digits = {0U};

#ifdef EEPROM_TIME_INIT
    /* eeprom_update_dword((uint32_t *)EEPROM_TIME_ADDR, (uint32_t)__initial_time_tag); */
    rtc_time = __initial_time_tm;
    system_time = mktime(&rtc_time);
    eeprom_update_dword((uint32_t *)EEPROM_TIME_ADDR, (uint32_t)system_time);
#endif

#if __TIME_SOURCE == 1
    __time = eeprom_read_dword((uint32_t *)EEPROM_TIME_ADDR);
    UART_INFO;
	uart_puts_P( "Time retrieved from internal EEPROM \r\n" );
#elif __TIME_SOURCE == 2
    UART_INFO;
	uart_puts_P( "Heartbeat set from external interrupt \r\n" );
#else 
    UART_ERROR;
	uart_puts_P( "No heartbeat set, halting.\r\n" );
    __HALT
#endif
    

    system_time = __time;
    set_system_time(__time);    
    set_zone(-6 * ONE_HOUR);
    localtime_r(&system_time, &rtc_time); 

#ifdef UART_BUFFER
    char usartBuffer[BUFFER_SIZE];
#endif

    /* Initialization */
    initialize();

    /* Clean format for UART text */
    UART_FCLEAN;

    /* Main loop */
    while (1U) 
    {
        if(g_heartbeat_1s == true){
            system_time = time(NULL);
            localtime_r(&system_time, &rtc_time);
            split_time(&digits, &rtc_time);

#if __TIME_SOURCE == 1
            if((rtc_time.tm_hour == 0) && (rtc_time.tm_min == 0) && (rtc_time.tm_sec == 0)){                
                eeprom_update_dword((uint32_t *)EEPROM_TIME_ADDR, (uint32_t)system_time);
            }else{
                /* Intentionally empty */
            }
#endif

            uart_puts_P( "\r\e[2K" );
            printf("%s", ctime(&system_time));

            PORTD ^= (1 << PORTD2);
            g_heartbeat_1s = false;
        }else{
            /* Intentionally empty */
        }
        display(&digits);
        read_buttons();
        /* update_mode(); */
    }
    
    return;
}

void initialize()
{
    /* Set outputs and inputs */
    DDRB |= (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5);
    DDRC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3);
    DDRD &= ~(1 << PORTD2);
    DDRD |= (1 << PORTD3);

    uart_init( BAUD_CALC( BAUD ) ); // 8n1 transmission is set as default
	
	stdout = &uart0_io; // attach uart stream to stdout & stdin
	stdin = &uart0_io; // uart0_in and uart0_out are only available if NO_USART_RX or NO_USART_TX is defined

    uart_puts_P( "\e[2J\e[H" );
    UART_INFO;
	uart_puts_P( "USART ready\r\n" );

    UART_INFO;
	uart_puts_P( "Configuring heartbeat source \r\n" );

#if __HEARTBEAT_SOURCE == 1
    /* Set Timer1 to CTC mode */
    TCCR1B |= (1U << WGM12);

    // Set the prescaler to 64
    TCCR1B |= (1U << CS12) | (1U << CS10);

    /* Compare match value for 1-second delay */
    OCR1A = 15624U;

    /* Enable Timer1 compare match interrupt */
    TIMSK1 |= (1U << OCIE1A);

    UART_INFO;
	uart_puts_P( "Heartbeat set from internal interrupt \r\n" );
#elif __HEARTBEAT_SOURCE == 2
    /* Set up external interrupt at PORTD2 (1Hz from RTC) */
	EICRA = ( _BV( ISC01 ) | _BV( ISC00 ) );

    /* Enable INT0 external interrupt */
	EIMSK = _BV( INT0 );

    UART_INFO;
	uart_puts_P( "Heartbeat set from external interrupt \r\n" );
#else 
    UART_ERROR;
	uart_puts_P( "No heartbeat set, halting.\r\n" );
    __HALT
#endif

#if __HEARTBEAT_SOURCE == 2
    /* Enable global interrupts */
    sei();
    UART_INFO;
	uart_puts_P( "Global interrupts enabled\r\n" );

	i2c_init();
    UART_INFO;
	uart_puts_P( "I2C Initialized\r\n" );


    UART_INFO;
    uart_puts_P( "Initializing DS3231 RTC\r\n" );

    struct tm rtc_time = {0U}; 
    uint8_t __u8buffer = 0;
	
	rtc_setByte(DS3231_CONTROL, 0x18);
    rtc_setByte(DS3231_STATUS, 0x00);

    do{
        UART_INFO;
	    uart_puts_P( "Waiting for module response.\r\n" );
        __u8buffer = rtc_getByte(DS3231_STATUS);
        printf("\tStatus register should now be 0x00 or 0x04, is: 0x%02x\r\n", __u8buffer);
        __u8buffer = rtc_getByte(DS3231_CONTROL);
        printf("\tControl register should now be 0x18, is: 0x%02x\r\n", __u8buffer);
    }while (__u8buffer != 0x18);

    rtc_setByte(DS3231_CONTROL, 0x00);
    __u8buffer = rtc_getByte(DS3231_CONTROL);

    rtc_getTime(&rtc_time);
    time_t rtc_time_tag = mktime(&rtc_time);

    UART_INFO;
    uart_puts_P( "Module initialized, time: " );
    printf("%s\r\n", ctime(&rtc_time_tag));
    printf("\tControl register set to: 0x%02x\r\n   ", rtc_getByte(DS3231_CONTROL));
#endif

}

/* TIMER1 Compare A interrupt service routine */
ISR(TIMER1_COMPA_vect) 
{
    system_tick();
    g_heartbeat_1s = true;
}

ISR( INT0_vect )
{
    system_tick();
    PORTD ^= _BV(PORTD3);
	g_heartbeat_1s = true;
}