#ifndef __LORAPCL_H__
#define __LORAPCL_H__

#include <stdint.h>
#include "lora_interface.h"
#include "timer_interface.h"

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
	ERROR,
	PROCESSING,
	NO_RESPONSE,
	NO_MODE_DEFINED,
	WRONG_MODE
} LoRa_Error_t;

LoRa_Error_t init_protocol(
		uint8_t uid,
		LoRa_Mode_t op_mode,
		const LoraInterface_t *lora_driver,
		const TimerInterface_t *timer
		);

void lorapcl_fsm();
LoRa_Error_t lorapcl_status();
LoRa_Error_t scan_gateways();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LORAPCL_H__ */
