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

#define TICKS_SCROLL 666


/*
 * All the available messages to display.
 *
 * NOTE: I am aware that there are repeating values.  Since all I'm doing is
 * displaying a few messages, I'm fine with being a bit sloppy with memory ;)
 */
typedef struct {
	uint8_t *msg;
	uint8_t  siz;
} msg_t;


uint8_t bths_21[] = {
	0b01111100,  // b
	0b01111000,  // t
	0b01110110,  // H
	0b01101101,  // S
	0b00000000,  // none
	0b01011011,  // 2
	0b00000110   // 1
};

uint8_t cooper_ee25[] = {
	0b00111001,  // C
	0b00111111,  // O
	0b00111111,  // O
	0b01110011,  // P
	0b01111001,  // E
	0b01010000,  // r
	0b00000000,  // none
	0b01111001,  // E
	0b01111001,  // E
	0b01011011,  // 2
	0b01101101   // 5
};

uint8_t giganerd[] = {
	0b01101111,  // g
	0b00000110,  // I
	0b01101111,  // g
	0b01110111,  // A
	0b01010100,  // n
	0b01111001,  // E
	0b01010000,  // r
	0b01011110,  // d
};

uint8_t las_pretty_eh[] = {
	0b00111000,  // L
	0b01110111,  // A
	0b01101101,  // S
	0b00000000,  // none
	0b01110011,  // P
	0b01010000,  // r
	0b01111001,  // E
	0b01111000,  // t
	0b01111000,  // t
	0b01101110,  // y
	0b00000000,  // none
	0b01111001,  // E
	0b01110100,  // h
};

uint8_t calc_is_bae[] = {
	0b00111001,  // C
	0b01110111,  // A
	0b00111000,  // L
	0b00111001,  // C
	0b00000000,  // none
	0b00000110,  // I
	0b01101101,  // S
	0b00000000,  // none
	0b01111100,  // b
	0b01110111,  // A
	0b01111001,  // E
};

uint8_t oh_thats_cap[] = {
	0b01011100,  // o
	0b01110100,  // h
	0b00000000,  // none
	0b01111000,  // t
	0b01110110,  // H
	0b01110111,  // A
	0b01111000,  // t
	0b01101101,  // S
	0b00000000,  // none
	0b00111001,  // C
	0b01110111,  // A
	0b01110011,  // P
};

uint8_t its_not_a_bomb[] = {
	0b00000100,  // i
	0b01111000,  // t
	0b01101101,  // S
	0b00000000,  // none
	0b01010100,  // n
	0b01011100,  // o
	0b01111000,  // t
	0b00000000,  // none
	0b01110111,  // A
	0b00000000,  // none
	0b01111100,  // b
	0b01011100,  // o
	0b01010100,  // m
	0b01111100   // b
};


msg_t msg[] = {
	{bths_21,        sizeof(bths_21)},
	{cooper_ee25,    sizeof(cooper_ee25)},
	{giganerd,       sizeof(giganerd)},
	{las_pretty_eh,  sizeof(las_pretty_eh)},
	{calc_is_bae,    sizeof(calc_is_bae)},
	{oh_thats_cap,   sizeof(oh_thats_cap)},
	{its_not_a_bomb, sizeof(its_not_a_bomb)}
};


void display_msg(uint8_t digit, bool next)
{
	/*
	 * Since a nice scrolling display is preffered, the letter index is
	 * defaulted to a negative value. In doing sowe have some padding while
	 * we're transitioning between messages.
	 */
	static uint8_t index = 0;
	static int8_t  lttr  = -DIGIT_CNT + 1;


	if (next) {
		++lttr;
		return;  // prevent partial refreshes
	}


	if (lttr >= msg[index].siz) {
		lttr = -DIGIT_CNT + 1;
		++index;
		index %= sizeof(msg) / sizeof(msg_t);
	}


	PORTD = (lttr + digit >= msg[index].siz || lttr + digit < 0)
		? 0
		: msg[index].msg[lttr + digit];
}


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


	while (true) {
		static uint16_t ticks = 0;
		static uint8_t  digit = 0;

		cli();


		// enable the appropriate digit
		PORTB |= 0x0F;
		display_msg(digit, false);
		PORTB &= ~_BV(digit);


		// display scrolling
		if (ticks >= TICKS_SCROLL) {
			display_msg(digit, true);
			ticks = 0;
		}


		++ticks;
		++digit;
		digit %= DIGIT_CNT;


		sei();

		// sleep as much as possible
		sleep_mode();
		sleep_disable();
	}


	return 0;
}


ISR(TIMER2_OVF_vect)
{
	// ISR is just used to get out of power-save mode
}
