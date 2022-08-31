#include "uart.h"
#include <assert.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "common/bitmask.h"

/* private variables */
static void (*rx_handler)() = NULL;

ISR(USART_RX_vect)
{
	if (rx_handler != NULL) rx_handler();
}

void uart_init(int baudrate)
{
	cli();
	/* setting uart speed */
	UBRR0L = UBRR(baudrate) & 0xFF;
	UBRR0H = UBRR(baudrate) >> 8;

	/* mode: async usart */
	BIT_CLR(UCSR0C, UMSEL00);
	BIT_CLR(UCSR0C, UMSEL01);
	
	/* frame size: 8bit */
	BIT_SET(UCSR0C, UCSZ00);
	BIT_SET(UCSR0C, UCSZ01);

	/* bit parity: disable */
	BIT_CLR(UCSR0C, UPM00);
	BIT_CLR(UCSR0C, UPM01);

	/* stop bit: 1 bit */
	BIT_CLR(UCSR0C, USBS0);
	
	/* interrupt on RX complete flag */
	BIT_SET(UCSR0B, RXCIE0);

	/* enable transmitter and receiver */
	BIT_SET(UCSR0B, RXEN0);
	BIT_SET(UCSR0B, TXEN0);
	sei();
}

void uart_write(const void *data, int cnt)
{
	assert(data != NULL);
	int i = 0;

	for (i = 0; i < cnt; i++) {
		while (!BIT_GET(UCSR0A, UDRE0));
		UDR0 = ((unsigned char *)data)[i];
	}
}

char uart_read()
{
	return UDR0;
}

int uart_available()
{
	return (int)BIT_GET(UCSR0A, RXC0);
}

void uart_rx_handler(void (*handler)()) 
{
	rx_handler = handler;
}
