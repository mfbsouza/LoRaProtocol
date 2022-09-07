#ifndef __LORA_INTERFACE_H__
#define __LORA_INTERFACE_H__

typedef struct {
	void (*radio_init)();
	void (*set_id)(unsigned char id);
	void (*set_net)(unsigned char net);
	void (*packet_send)(unsigned char id, unsigned char *msg, unsigned char msg_size);
	void (*packet_recv)(unsigned char *id, unsigned char *buf, unsigned char *size);
	int  (*packet_available)();
} LoraInterface_t;

#endif /* __LORA_INTERFACE_H__ */
