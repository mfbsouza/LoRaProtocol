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

//int main()
//{
//	unsigned long ref;
//
//	/* init led and turn it off */
//	gpio_init(GPIO_PB5, OUTPUT);
//	gpio_write(GPIO_PB5, 0);
//
//	/* give some time to LoRaMESH boot */
//	timer.init();
//	timer.delay(2000);
//
//	/* flash led to tell that delay was finished */
//	gpio_write(GPIO_PB5, 1);
//
//	/* turn on the uart driver */
//	serial.init(9600);
//
//	/* initialize the loramesh driver */
//	lora_init(&serial, &timer);
//
//	/* testing set a id to the loramesh */
//	//timer.delay(1);
//	//lora_set_id(0);
//
//	/* blink every 1 sec in the loop() */
//	ref = timer.millis();
//	while(1) {
//		if (timer.millis() - ref >= 1000) {
//			gpio_write(GPIO_PB5, !gpio_read(GPIO_PB5));
//			ref = timer.millis();
//		}
//		timer.delay(10);
//	}
//
//	return 0;
//}

//int main()
//{
//	serial.init(9600);
//	timer.init();
//
//	while (1) {
//		if (serial.available() > 0) {
//			char c = serial.read();
//			serial.write(&c, 1);
//		}
//		//while ((UCSR0A & (1 << RXC0)) == 0); 
//		//char c = UDR0;
//		//serial.write(&c, 1);
//		//timer.delay(10);
//	}
//}

void on_rx()
{
	char c = serial.read();
	serial.write(&c, 1);
}

int main()
{
	serial.init(9600);
	serial.rx_handler(on_rx);
	timer.init();
	timer.delay(2000);

	unsigned char msg[] = { 0x00, 0x00, 0xE2, 0x00, 0x00, 0x00, 0x15, 0xB8 };

	serial.write(msg, 8);
	timer.delay(6000);
	while (1) {
		//if (serial.available() > 0) {
		//	char c = serial.read();
		//	serial.write(&c, 1);
		//}
		timer.delay(100);
	}
}
