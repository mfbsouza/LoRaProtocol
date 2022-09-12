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
	PCL_ERROR,
	NO_RESPONSE,
	GATEWAY_FULL,
	NOT_EXPECTED,
	NO_MODE_DEFINED,
	WRONG_MODE
} LoRa_Error_t;

LoRa_Error_t lorapcl_init(
		uint8_t uid,
		LoRa_Mode_t op_mode,
		const LoraInterface_t *lora_driver,
		const TimerInterface_t *timer
		);

LoRa_Error_t lorapcl_scan_gateway  (uint8_t *gw_uid, uint8_t *gw_nid);
LoRa_Error_t lorapcl_connect       (uint8_t gateway_uid, uint8_t gateway_nid);
void         lorapcl_gateway_fsm   ();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LORAPCL_H__ */
