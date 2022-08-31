#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "common/bitmask.h"

#define PRESCALER 64

static unsigned long millis_cnt = 0;
static unsigned char initialized = 0;

ISR(TIMER0_COMPA_vect)
{
	++millis_cnt;
}

void timer_init()
{
	if(!initialized) {
		cli();
		BIT_SET(TCCR0A, WGM01);
		BIT_SET(TCCR0B, CS01);
		BIT_SET(TCCR0B, CS00);
		BIT_SET(TIMSK0, OCIE0A);
		OCR0A = ((F_CPU / PRESCALER) / 1000);
		initialized = 1;
		sei();
	}
}

unsigned long timer_millis_get()
{
	unsigned long ms;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		ms = millis_cnt;
	}
	return ms;
}

void timer_delay(double ms)
{
	unsigned long ref = timer_millis_get();

	while (ms > 0) {
		if ((timer_millis_get() - ref) > 1) {
			ms--;
			ref++;
		}
	}
}
