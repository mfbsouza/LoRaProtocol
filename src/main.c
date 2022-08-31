#include <string.h>
#include "hal/uart.h"
#include "hal/timer.h"
#include "hal/gpio.h"
#include "drivers/LoRaMESH/LoRa.h"

static const SerialInterface_t serial = {
	.available = uart_available,
	.init = uart_init,
	.read = uart_read,
	.rx_handler = uart_rx_handler,
	.write = uart_write
};

static const TimerInterface_t timer = {
	.init = timer_init,
	.millis = timer_millis_get,
	.delay = timer_delay
};

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
	timer.delay(2000);

	serial.init(9600);
	timer.init();
	lora_init(&serial, &timer);

	/* read local lora info */
	if (LocalRead(&local_id, &local_net, &unique_id) != MESH_OK) {

		if (local_net == 5376 || local_net == 21) {
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
		//serial.write("error reading lora", 18);
	}

	while(1) {
		timer.delay(10);
	}

	return 0;
}

