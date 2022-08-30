#include <string.h>
#include <util/delay.h>
#include "hal/uart.h"
#include "hal/gpio.h"
#include "drivers/LoRaMESH/LoRa.h"

static const SerialInterface_t serial = {
	.available = uart_available,
	.init = uart_init,
	.read = uart_read,
	.rx_handler = uart_rx_handler,
	.write = uart_write
};

//void delay_wrapper(double ms)
//{
//	_delay_ms(ms);
//}

//static const TimerInterface_t timer = {
//	.delay = delay_wrapper
//};

void int_to_char_uart(const void *data, int cnt)
{
	int i;
	uint8_t c;
	for (i = 0; i < cnt; i++) {
		c = ((uint8_t *)data)[i] + '0';
		serial.write(&c, 1);
	}
}

int main ()
{
	uint16_t local_id, local_net;
	uint32_t unique_id;

	gpio_init(GPIO_PB5, OUTPUT);
	gpio_write(GPIO_PB5, 0);
	/* give some time to LoRaMESH boot */
	//timer.delay(2000);
	_delay_ms(3000);

	serial.init(9600);
	lora_init(&serial);

	/* read local lora info */
	if (LocalRead(&local_id, &local_net, &unique_id) != MESH_OK) {

		if (local_net == 3584 || local_net == 14) {
			gpio_write(GPIO_PB5, 1);
		}

		//serial.write("Local ID: ", 10);
		//int_to_char_uart(&local_id, 2);
		//serial.write("\n", 1);

		//serial.write("Local NET: ", 11);
		//int_to_char_uart(&local_net, 2);
		//serial.write("\n", 1);

		//serial.write("Local UID: ", 11);
		//int_to_char_uart(&unique_id, 4);
		//serial.write("\n", 1);
	}
	else {
		serial.write("error reading lora", 18);
	}

	while(1) {
		_delay_ms(10);
	}

	return 0;
}

