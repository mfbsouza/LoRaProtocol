#include "LoRaPCL.h"
#include <string.h>
#include <assert.h>

#define LORACTL_MAX_DATA_SIZE 250

enum lorapcl_states {
	NO_MODE,
	DISCONNECTED,
	SEND_SCAN,
	WAIT_GW_ID,
	BUILD_GW_IDS,
	INFO_SCAN_FAIL,
	INFO_SCAN_OK,
	IDLE,
	SEND_GW_ID
};

typedef enum {
	SCAN,
	GW_INFO
} Packet_Type_t;

typedef struct {
	uint8_t src_uid;
	uint8_t src_nid;
	uint8_t dest_uid;
	uint8_t dest_nid;
	uint8_t flags_n_seq;
	uint8_t data_size;
	uint8_t data[LORACTL_MAX_DATA_SIZE];
} Packet_Frame_t;

/* private variables */
static uint8_t unique_id;
static uint8_t net_id;
static uint8_t state = NO_MODE;
static uint8_t mode = NONE;
static const LoraInterface_t *radio = NULL;
static const TimerInterface_t *mTimer = NULL;
static Packet_Frame_t buffer;

static uint8_t gw_nid, gw_uid;
static uint8_t app_informed = 0;

/* private functions */
static Packet_Type_t get_packet_type(Packet_Frame_t *packet);
static void build_response(Packet_Frame_t *packet, uint8_t flags, uint8_t seq_number);

LoRa_Error_t init_protocol(
		uint8_t uid,
		LoRa_Mode_t op_mode,
		const LoraInterface_t *lora_driver,
		const TimerInterface_t *timer
		)
{
	assert(lora_driver != NULL);
	assert(timer != NULL);

	/* check if a valid mode was passed 
	 * and set the NID in that case */
	switch (op_mode) {
	case GATEWAY:
		net_id = 0x00;
		state = IDLE;
		break;
	case NODE:
		net_id = 0xFF;
		state = DISCONNECTED;
		break;
	default:
		return NO_MODE_DEFINED;
	}

	/* initialize the private variables */
	unique_id = uid;
	mode = op_mode;
	radio = lora_driver;
	mTimer = timer;

	/* set the NID in the LoRa Radio */
	radio->set_id(net_id);

	/* TODO: set_net actually sets the network key 
	 * this maybe should not be used */
	radio->set_net(0x10);

	// ligar fsm??
	
	return NO_ERROR;
}

LoRa_Error_t scan_gateways()
{
	if (mode == NONE) return NO_MODE_DEFINED;
	if (mode == GATEWAY) return WRONG_MODE;
	if (state != DISCONNECTED) return ERROR;
	state = SEND_SCAN;
	return NO_ERROR;
}

LoRa_Error_t lorapcl_status()
{
	if (state == INFO_SCAN_OK) {
		app_informed = 1;
		return NO_ERROR;
	}
	if (state == INFO_SCAN_FAIL) {
		app_informed = 1;
		return ERROR;
	}
	return PROCESSING;
}

void lorapcl_fsm()
{
	static unsigned long time_ref;
	static uint8_t timeout_count = 0;
	uint8_t recv_nid, pkt_size;
	Packet_Type_t msg_type;

	switch (state) {
	case NO_MODE:
		break;
	case DISCONNECTED:
		break;
	case IDLE:
		if (radio->packet_available()) {
			radio->packet_recv(&recv_nid, (uint8_t *)&buffer, &pkt_size);
			msg_type = get_packet_type(&buffer);

			if (msg_type == SCAN) {
				state = SEND_GW_ID;
			}
		}
		break;

	case SEND_GW_ID:
		build_response(&buffer, GW_INFO, 0);
		radio->packet_send(buffer.dest_nid, (uint8_t *)&buffer, (buffer.data_size + 6));
		state = IDLE;
		break;
	
	case SEND_SCAN:
		buffer.src_uid = unique_id;
		buffer.src_nid = net_id;
		buffer.dest_uid = 0X00;
		buffer.dest_nid = 0x00;
		buffer.flags_n_seq = ((SCAN << 4)&0xF0);
		buffer.data_size = 0;
		radio->packet_send(buffer.dest_nid, (uint8_t *)&buffer, (buffer.data_size + 6));
		time_ref = mTimer->millis();
		state = WAIT_GW_ID;
		break;
	
	case WAIT_GW_ID:
		if ((mTimer->millis() - time_ref) <= 5000) {
			if (radio->packet_available()) {
				radio->packet_recv(&recv_nid, (uint8_t *)&buffer, &pkt_size);
				msg_type = get_packet_type(&buffer);
				if (msg_type == GW_INFO) {
					state = BUILD_GW_IDS;
					break;
				}
			}
		}
		else {
			timeout_count++;
			if (timeout_count < 2) {
				state = SEND_SCAN;
			}
			else if (timeout_count == 2) {
				timeout_count = 0;
				state = INFO_SCAN_FAIL;
			}
		}
		break;
	
	case BUILD_GW_IDS:
		gw_nid = buffer.src_nid;
		gw_uid = buffer.src_uid;
		state = INFO_SCAN_OK;
		break;

	case INFO_SCAN_OK:
		if (app_informed) {
			app_informed = 0;
			state = DISCONNECTED;
		}
		break;

	case INFO_SCAN_FAIL:
		if (app_informed) {
			app_informed = 0;
			state = DISCONNECTED;
		}
		break;
	}
}

static Packet_Type_t get_packet_type(Packet_Frame_t *packet)
{
	uint8_t flags;
	flags = (packet->flags_n_seq >> 4)&0x0F;
	return (Packet_Type_t)flags;
}

static void build_response(Packet_Frame_t *packet, uint8_t flags, uint8_t seq_number)
{
	packet->dest_nid = packet->src_nid;
	packet->dest_uid = packet->src_uid;
	packet->src_nid = net_id;
	packet->src_uid = unique_id;
	packet->flags_n_seq = ((flags << 4)&0xF0) | (seq_number&0x0F);
	packet->data_size = 0;
}
