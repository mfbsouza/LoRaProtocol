#include "LoRaPCL.h"
#include <string.h>
#include <assert.h>

enum lorapcl_states {
	MODE_SELECT,
	INIT_RADIO,
	SET_ID_NODE,
	SET_ID_GATEWAY,
	SET_NET,
	IDLE
};

static uint8_t state = MODE_SELECT;
static uint8_t mode = NONE;
static const LoraInterface_t *radio = NULL;
static const TimerInterface_t *mTimer = NULL;

void init_protocol(const LoraInterface_t *lora_driver, const TimerInterface_t *timer, enum loractl_mode op_mode)
{
	assert(lora_driver != NULL);
	assert(timer != NULL);

	radio = lora_driver;
	mTimer = timer;
	mode = op_mode;

	// ligar fsm??
}

void lorapcl_fsm()
{
	switch(state) {
	case MODE_SELECT:
		// do nothing?
		if (mode != NONE) {
			state = INIT_RADIO;
		}
		break;
	
	case INIT_RADIO:
		// liga o radio
		if (mode == GATEWAY) {
			state = SET_ID_GATEWAY;
		}
		else if (mode == NODE) {
			state = SET_ID_NODE;
		}
		break;
	
	case SET_ID_GATEWAY:
		// setar id do radio para 0
		state = SET_NET;
		break;
	
	case SET_ID_NODE:
		// set id do radio para 0xff
		state = SET_NET;
		break;
	
	case SET_NET:
		// set net do radio
		state = IDLE;
		break;
	}
}
