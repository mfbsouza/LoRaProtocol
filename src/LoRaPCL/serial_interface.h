#ifndef __SERIAL_INTERFACE_H__
#define __SERIAL_INTERFACE_H__

typedef struct {
	void (*init)(int baudrate);
	void (*write)(const void *buf, int size);
	char (*read)();
	int  (*available)();
	void (*rx_handler)(void (*)());
} SerialInterface_t;

#endif /* __SERIAL_INTERFACE_H__ */
