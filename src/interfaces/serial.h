#ifndef __SERIAL_H__
#define __SERIAL_H__

struct serial_ops {
	const char *name;
	void (*init)(int baudrate);
	void (*write)(const char *buf, int size);
	char (*read_byte)();
	void (*rx_callback)(void (*)());
};

#endif /* __SERIAL_H__ */
