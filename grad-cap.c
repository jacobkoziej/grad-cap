/*
 * grad-cap -- bths '21 graduation quad seven segment driver
 * Copyright (C) 2021  Jacob Koziej <jacobkoziej@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Wiring:
 * =======
 *
 * PD0 - segment a
 * PD1 - segment b
 * PD2 - segment c
 * PD3 - segment d
 * PD4 - segment e
 * PD5 - segment f
 * PD6 - segment g
 * PD7 - decimal point
 *
 * PB0 - digit 1
 * PB1 - digit 2
 * PB2 - digit 3
 * PB3 - digit 4
 */


#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stdint.h>


#define DIGIT_CNT 4


int main(void)
{
	// set data direction to output
	DDRB |= 0x0F;
	DDRD  = 0xFF;

	// clear outputs
	PORTB |= 0x0F;
	PORTD  = 0x00;


	// disable unnecessary components
	power_adc_disable();
	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
	power_twi_disable();
	power_usart0_disable();

	set_sleep_mode(SLEEP_MODE_PWR_SAVE);


	// timer setup
	TCCR2A = 0x00;                   // normal operation
	TCCR2B = _BV(CS22) | _BV(CS20);  // scalar for ~1.6ms overflow @20MHz
	TIMSK2 = _BV(TOIE2);             // enable overflow interrupt

	sei();


	while (true) {
		static uint16_t ticks = 0;
		static uint8_t  digit = 0;


		// enable the appropriate digit
		PORTB |= 0x0F;
		PORTB &= ~_BV(digit);


		++ticks;
		++digit;
		digit %= DIGIT_CNT;


		// sleep as much as possible
		sleep_mode();
		sleep_disable();
	}


	return 0;
}


ISR(TIMER2_OVF_vect)
{
	// ISR is just used to get out or power-save mode
}
