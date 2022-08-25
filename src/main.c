#include "hal/gpio.h"
#include "hal/uart.h"
#include <util/delay.h>

void on_rx();

int main ()
{
	const char *string = "hello uart!\r\nworking nicely!\r\ntry typing: ";
	const struct serial_ops *serial = uart_driver();

	serial->init(9600);
	serial->rx_callback(on_rx);
	serial->write(string, strlen(string));

	gpio_init(GPIO_PB5, OUTPUT);

	while(1) {
		gpio_write(GPIO_PB5, !gpio_read(GPIO_PB5));
		_delay_ms(250);
	}
	return 0;
}

void on_rx()
{
	const struct serial_ops *serial = uart_driver();
	char c = serial->read_byte();
	if (c == '\r') {
		serial->write("\r\n", 2);
	}
	else if (c == 127) {
		serial->write("\b \b", 3);
	}
	else {
		serial->write(&c, 1);
	}
}
