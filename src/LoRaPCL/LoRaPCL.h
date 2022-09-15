#ifndef __LORAPCL_H__
#define __LORAPCL_H__

#include <stdint.h>
#include "lora_interface.h"
#include "timer_interface.h"
#ifdef DEBUG
#include "serial_interface.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
	NONE,
	GATEWAY,
	NODE
} LoRa_Mode_t;

typedef enum {
	NO_ERROR,
	NO_RESPONSE,
	GATEWAY_FULL,
	NO_MODE_DEFINED,
	DENIED
} LoRa_Error_t;

#ifndef DEBUG
LoRa_Error_t lorapcl_init(
		uint8_t uid,
		LoRa_Mode_t op_mode,
		const LoraInterface_t *lora,
		const TimerInterface_t *timer
		);
#else
LoRa_Error_t lorapcl_init(
		uint8_t uid,
		LoRa_Mode_t op_mode,
		const LoraInterface_t *lora,
		const SerialInterface_t *serial,
		const TimerInterface_t *timer
		);
#endif

LoRa_Error_t lorapcl_scan_gateway  (uint8_t *gateway_uid);
LoRa_Error_t lorapcl_connect       (uint8_t gateway_uid);
void         lorapcl_gateway_fsm   ();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LORAPCL_H__ */
