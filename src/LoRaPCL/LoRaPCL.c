#include "LoRaPCL.h"
#include <string.h>
#include <assert.h>

#define LORACTL_MAX_DATA_SIZE 250

enum lorapcl_states {
	NO_MODE,
	DISCONNECTED,

	IDLE,
	SEND_GW_ID,
	CHECK_FREE_NID,
	SEND_SYNC_SET_ID,
	WAIT_ACK,
	UPDATE_CLIENT_LIST
};

typedef enum {
	SCAN,
	GW_INFO,
	SYNC,
	SYNC_SET_ID,
	ACK,
	GW_FULL
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

/* private functions */
static Packet_Type_t get_packet_type(Packet_Frame_t *packet);
static void build_signal(uint8_t s_uid, uint8_t s_nid, uint8_t d_uid, uint8_t d_nid, uint8_t flag);
static void build_response(Packet_Frame_t *packet, uint8_t flags, uint8_t seq_number);

LoRa_Error_t lorapcl_init(
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

void lorapcl_gateway_fsm()
{
	static uint8_t free_nid, old_nid, timeout_count = 0;
	static unsigned long timer_ref;
	uint8_t recv_nid, pkt_size;

	switch (state) {
	case IDLE:
		if (radio->packet_available()) {
			radio->packet_recv(&recv_nid, (uint8_t *)&buffer, &pkt_size);

			if (get_packet_type(&buffer) == SCAN) {
				state = SEND_GW_ID;
			}
			else if (get_packet_type(&buffer) == SYNC) {
				state = CHECK_FREE_NID;
			}
		}
		break;

	case SEND_GW_ID:
		build_response(&buffer, GW_INFO, 0);
		radio->packet_send(buffer.dest_nid, (uint8_t *)&buffer, (buffer.data_size + 6));
		state = IDLE;
		break;
	
	case CHECK_FREE_NID:
		// TODO: do a real checking
		free_nid = 0x01;
		state = SEND_SYNC_SET_ID;
		break;

	case SEND_SYNC_SET_ID:
		old_nid = buffer.src_nid;
		build_signal(unique_id, net_id, buffer.src_uid, free_nid, SYNC_SET_ID);
		radio->packet_send(old_nid, (uint8_t *)&buffer, (buffer.data_size + 6));
		timer_ref = mTimer->millis();
		state = WAIT_ACK;
		break;

	case WAIT_ACK:
		if((mTimer->millis() - timer_ref) <= 5000) {
			if (radio->packet_available()) {
				radio->packet_recv(&recv_nid, (uint8_t *)&buffer, &pkt_size);

				if (get_packet_type(&buffer) == ACK) {
					// Connection Done
					state = UPDATE_CLIENT_LIST;
				}
			}
		}
		else {
			timeout_count++;
			if (timeout_count < 2) {
				state = SEND_SYNC_SET_ID;
			}
			else if (timeout_count == 2) {
				timeout_count = 0;
				state = IDLE;
			}
		}
		break;
	
	case UPDATE_CLIENT_LIST:
		// TODO: inform all also
		state = IDLE;
		break;
	}
}

LoRa_Error_t lorapcl_scan_gateway(uint8_t *gw_uid, uint8_t *gw_nid)
{
	uint8_t recv_nid, pkt_size, timeout_count = 0;
	unsigned long timer_ref;

	// check states
	if (mode != NODE) return WRONG_MODE;
	if (state != DISCONNECTED) return PCL_ERROR; // TODO: maybe create a NODE state var

SEND_SCAN:
	// build SCAN packet
	build_signal(unique_id, net_id, 0x00, 0x00, SCAN);

	// Send Scan Signal
	radio->packet_send(buffer.dest_nid, (uint8_t *)&buffer, (buffer.data_size + 6));
	timer_ref = mTimer->millis();

WAIT_GW_ID:
	if ((mTimer->millis() - timer_ref) <= 5000) {
		if (radio->packet_available()) {
			radio->packet_recv(&recv_nid, (uint8_t *)&buffer, &pkt_size);

			if (get_packet_type(&buffer) == GW_INFO) {
				*gw_uid = buffer.src_uid;
				*gw_nid = buffer.src_nid;
				return NO_ERROR;
			}
		}
		goto WAIT_GW_ID;
	}
	else {
		timeout_count++;
		if (timeout_count < 2) goto SEND_SCAN;
		else if (timeout_count == 2) return NO_RESPONSE;
	}

	return NOT_EXPECTED;
}

LoRa_Error_t lorapcl_connect(uint8_t gateway_uid, uint8_t gateway_nid)
{
	uint8_t recv_nid, pkt_size, timeout_count = 0;
	unsigned long timer_ref;

	// check states
	if (mode != NODE) return WRONG_MODE;
	// TODO: maybe create a NODE state var
	if (state != DISCONNECTED) return PCL_ERROR;

SEND_SYNC:
	// build SYNC packet
	build_signal(unique_id, net_id, gateway_uid, gateway_nid, SYNC);

	// Send Sync Signal
	radio->packet_send(buffer.dest_nid, (uint8_t *)&buffer, 6);
	timer_ref = mTimer->millis();

WAIT_NEW_ID:
	if ((mTimer->millis() - timer_ref) <= 5000) {
		if (radio->packet_available()) {
			radio->packet_recv(&recv_nid, (uint8_t *)&buffer, &pkt_size);

			if (get_packet_type(&buffer) == SYNC_SET_ID) {
				// update to new NID
				net_id = buffer.dest_nid;
				radio->set_id(net_id);

				// Send Ack to Gateway
				build_signal(unique_id, net_id, gateway_uid, gateway_nid, ACK);
				radio->packet_send(buffer.dest_nid, (uint8_t *)&buffer, 6);

				// Connection Success
				return NO_ERROR;
			}
			else if (get_packet_type(&buffer) == GW_FULL) {
				// Connection Failed: Gateway Full
				return GATEWAY_FULL;
			}
		}
		goto WAIT_NEW_ID;
	}
	else {
		timeout_count++;
		if (timeout_count < 2) goto SEND_SYNC;
		// Connection Failed: No Response From Gateway
		else if (timeout_count == 2) return NO_RESPONSE;
	}

	return NOT_EXPECTED;
}

static Packet_Type_t get_packet_type(Packet_Frame_t *packet)
{
	uint8_t flags;
	flags = (packet->flags_n_seq >> 4)&0x0F;
	return (Packet_Type_t)flags;
}

static void build_signal(uint8_t s_uid, uint8_t s_nid, uint8_t d_uid, uint8_t d_nid, uint8_t flag)
{
	buffer.src_uid = s_uid;
	buffer.src_nid = s_nid;
	buffer.dest_uid = d_uid;
	buffer.dest_nid = d_nid;
	buffer.flags_n_seq = ((flag << 4)&0xF0);
	buffer.data_size = 0;
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
