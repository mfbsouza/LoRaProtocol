#ifndef __LORAPCL_H__
#define __LORAPCL_H__

#include <stdint.h>
#include "lora_interface.h"
#include "timer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum loractl_mode {
	NONE,
	GATEWAY,
	NODE
};

void init_protocol(const LoraInterface_t *lora_driver, const TimerInterface_t *timer, enum loractl_mode op_mode);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LORAPCL_H__ */
