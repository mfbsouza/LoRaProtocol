#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "common/bitmask.h"

#define PRESCALER 64

static unsigned long millis_cnt = 0;

ISR(TIM0_COMPA_vect)
{
	++millis_cnt;
}

void timer_init()
{
	BIT_SET(TCCR0A, WGM01);
	BIT_SET(TCCR0B, (CS01 | CS00));
	BIT_SET(TIMSK0, OCIE0A);
	OCR0A = ((F_CPU / PRESCALER) / 1000);
}

unsigned long timer_millis_get()
{
	unsigned long ms;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		ms = millis_cnt;
	}
	return ms;
}
