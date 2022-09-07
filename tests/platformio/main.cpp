/** 
 * Testing with arduino framework for ESP32 and platformio
 * by mfbsouza
*/

#include <Arduino.h>
#include <LoRa.h>

#define RXD2 16
#define TXD2 17

int avail_wrapper()
{
	return (int)Serial2.available();
}

char read_wrapper()
{
	char c = Serial2.read();
	return c;
}

void write_wrapper(const void *buf, int size)
{
	Serial2.write((char *)buf, size);
}

void timer_delay_wrapper(double ms)
{
	delay((uint32_t)ms);
}

static const SerialInterface_t serial = {
	.write = write_wrapper,
	.read = read_wrapper,
	.available = avail_wrapper};

static const TimerInterface_t timer = {
	.millis = millis,
	.delay = timer_delay_wrapper};

void setup()
{
	Serial.begin(115200);						 // debug serial
	Serial2.begin(9600, SERIAL_8E1, RXD2, TXD2); // lora serial
	delay(3000);
	Serial.printf("DEBUG: esp init OK\r\n");

	// init loramesh
	lora_init(&serial, &timer);
	Serial.printf("DEBUG: lora init OK\r\n");

	// debuggin
	uint16_t id, net;
	uint32_t uid;

	/**
	 * LOCAL READ TEST :
	 * send LocalRead through TX with PrepareFrameCommand() and SendPacket()
	 * recv Data through RX with ReceivePacketCommand()
	 * */
	lora_set_id(0);
	LocalRead(&id, &net, &uid);
	Serial.printf("ID:  0x%02X\r\nNET: 0x%02X\r\nUID: 0x%X\r\n", id, net, uid);

	/**
	 * SET ID TEST :
	 * send 0xCA command through TX with PrepareFrameCommand() and SendPacket()
	 * recv Data through RX with ReceivePacketCommand()
	 * */
	lora_set_id(15);
	LocalRead(&id, &net, &uid);
	Serial.printf("ID:  0x%02X\r\nNET: 0x%02X\r\nUID: 0x%X\r\n", id, net, uid);

	/**
	 * SET NET TEST :
	 * send 0xCD command through TX with PrepareFrameCommand() and SendPacket()
	 * recv Data through RX with ReceivePacketCommand()
	 * */
	lora_set_net(0x10);
	LocalRead(&id, &net, &uid);
	Serial.printf("ID:  0x%02X\r\nNET: 0x%02X\r\nUID: 0x%X\r\n", id, net, uid);

	// master recv test
	lora_set_id(0);
	LocalRead(&id, &net, &uid);
	Serial.printf("ID:  0x%02X\r\nNET: 0x%02X\r\nUID: 0x%X\r\n", id, net, uid);
}

void loop()
{
	static uint8_t recv_id, pkt_size;
	static uint8_t buffer[MAX_PAYLOAD_SIZE];

	if (lora_packet_available()) {
		lora_recv_packet(&recv_id, buffer, &pkt_size);
		Serial.printf("new data: ");
		Serial.print(buffer[0], pkt_size);
		Serial.printf("\r\n");
	}
	delay(10);
}