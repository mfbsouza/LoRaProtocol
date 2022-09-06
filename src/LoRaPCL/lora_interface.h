#ifndef __LORA_INTERFACE_H__
#define __LORA_INTERFACE_H__

typedef struct {
	void (*radio_init)();
	void (*set_id)(unsigned char id);
	void (*set_net)(unsigned char net);
	void (*packet_send)();
	void (*packet_recv)();
} LoraInterface_t;

#endif /* __LORA_INTERFACE_H__ */
