#include "LoRaPCL.h"
#include <string.h>

/* Protocol Macros */

#ifdef DEBUG
	#define LORAPCL_TIMER_THLD 5000 // 5s
#else
	#define LORAPCL_TIMER_THLD 3000 // 3s
#endif

#define LORAPCL_MAX_DATA_SIZE 250 // 250 bytes

/* Protocol States */

enum lorapcl_states {
	NO_MODE,
	IDLE,
	NODE_DISCONNECTED,
	NODE_CONNECTED,

	SEND_GW_ID,
	CHECK_FREE_NID,
	SEND_GW_FULL,
	SEND_SYNC_SET_ID,
	WAIT_ACK,
	UPDATE_CLIENT_LIST
};

/* Protocol Flags */

typedef enum {
	SCAN,
	GW_INFO,
	SYNC,
	SYNC_SET_ID,
	ACK,
	GW_FULL
} Packet_Type_t;

/* Protocol Frame Struct */

typedef struct {
	uint8_t src_uid;
	uint8_t src_nid;
	uint8_t dest_uid;
	uint8_t dest_nid;
	uint8_t flags_n_seq;
	uint8_t data_size;
	uint8_t data[LORAPCL_MAX_DATA_SIZE];
} Packet_Frame_t;

/* private variables */

static uint8_t	       unique_id;
static uint8_t	       net_id;
static uint8_t	       state = NO_MODE;
static uint8_t	       mode = NONE;
// lot of memory
static Packet_Frame_t  buffer;
static Packet_Frame_t  data_packages[4];
static uint8_t         gw_client_list[250];

static const LoraInterface_t  *radio  = NULL;
static const TimerInterface_t *mTimer = NULL;
#ifdef DEBUG
static const SerialInterface_t *debug = NULL;
#endif

/* private functions */

static uint8_t        get_free_nid     ();
static Packet_Type_t  get_packet_type  (Packet_Frame_t *packet);
static LoRa_Error_t   wait_signal      (Packet_Type_t signal);

static void send_signal(
		uint8_t dest_uid,
		uint8_t dest_nid,
		Packet_Type_t signal
		);

static void send_packet(
		uint8_t dest_uid,
		uint8_t dest_nid,
		const void *data,
		uint8_t size
		);

static void build_signal(
		uint8_t s_uid,
		uint8_t s_nid,
		uint8_t d_uid,
		uint8_t d_nid,
		uint8_t flag
		);

/* API Functions code */

#ifndef DEBUG
LoRa_Error_t lorapcl_init(
		uint8_t uid,
		LoRa_Mode_t op_mode,
		const LoraInterface_t *lora,
		const TimerInterface_t *timer
		)
#else
LoRa_Error_t lorapcl_init(
		uint8_t uid,
		LoRa_Mode_t op_mode,
		const LoraInterface_t *lora,
		const SerialInterface_t *serial,
		const TimerInterface_t *timer
		)
#endif
{

	/* check if a valid mode was passed 
	 * and set the NID in that case */
	switch (op_mode) {
	case GATEWAY:
		net_id = 0x00;
		gw_client_list[net_id] = uid;
		state = IDLE;
		break;
	case NODE:
		net_id = 0xFF;
		state = NODE_DISCONNECTED;
		break;
	default:
		return NO_MODE_DEFINED;
	}

	/* initialize the private variables */
	unique_id = uid;
	mode      = op_mode;
	radio     = lora;
	mTimer    = timer;
#ifdef DEBUG
	debug     = serial;
#endif

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
		send_signal(buffer.src_uid, buffer.src_nid, GW_INFO);
		state = IDLE;
		break;
	
	case CHECK_FREE_NID:
		free_nid = get_free_nid();
		if (0x00 != free_nid) {
			state = SEND_SYNC_SET_ID;
		} else {
			state = SEND_GW_FULL;
		}
		break;
	
	case SEND_GW_FULL:
		send_signal(buffer.src_uid, buffer.src_nid, GW_FULL);
		state = IDLE;
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
		gw_client_list[free_nid] = buffer.src_uid;
		state = IDLE;
		break;
	}
}

LoRa_Error_t lorapcl_scan_gateway(uint8_t *gateway_uid)
{
	uint8_t timeout = 0;

	// check states
	if (mode != NODE) return DENIED;
	if (state != NODE_DISCONNECTED) return DENIED;


	while (timeout < 2) {
		send_signal(0x00, 0x00, SCAN);
		if (wait_signal(GW_INFO) == NO_ERROR) {
			// copy gateway info to app data
			*gateway_uid = buffer.src_uid;
			return NO_ERROR;
		}
		timeout++;
	}
	return NO_RESPONSE;
}

LoRa_Error_t lorapcl_connect(uint8_t gateway_uid)
{
	uint8_t recv_nid, pkt_size;
	uint8_t timeout = 0;
	unsigned long timer_ref;

	// check states
	if (mode != NODE) return DENIED;
	if (state != NODE_DISCONNECTED) return DENIED;

	while (timeout < 2) {
		send_signal(gateway_uid, 0x00, SYNC);
		timer_ref = mTimer->millis();
		while (mTimer->millis() - timer_ref <= LORAPCL_TIMER_THLD) {
			if(radio->packet_available()) {
				radio->packet_recv(
						&recv_nid,
						(uint8_t *)&buffer,
						&pkt_size
						);
				if (get_packet_type(&buffer) == SYNC_SET_ID) {
					// update node to new NID
					net_id = buffer.dest_nid;
					radio->set_id(net_id);

					// Acknowladge the gateway
					send_signal(
							gateway_uid,
							0x00,
							ACK
							);

					// Connection was successeful
					state = NODE_CONNECTED;
					return NO_ERROR;
				}
				else if (get_packet_type(&buffer)
						== GW_FULL) {
					//Connection Failed: Gateway is full
					return GATEWAY_FULL;
				}
			}
		}
	}
	return NO_RESPONSE;
}

/* private functions code */

static LoRa_Error_t wait_signal(Packet_Type_t signal)
{
	unsigned long timer_ref = mTimer->millis();
	uint8_t recv_nid, pkt_size;

	while (mTimer->millis() - timer_ref <= LORAPCL_TIMER_THLD) {
		if (radio->packet_available()) {
			radio->packet_recv(
					&recv_nid,
					(uint8_t *)&buffer,
					&pkt_size
					);
			if (get_packet_type(&buffer) == signal) 
				return NO_ERROR;
		}
	}

	return NO_RESPONSE;
}

static uint8_t get_free_nid()
{
	uint8_t nid, full = 0;

	for(nid = 0; nid < 250; nid++) {
		if (0x00 == gw_client_list[nid]) return nid;
	}

	return full;
}

static Packet_Type_t get_packet_type(Packet_Frame_t *packet)
{
	uint8_t flags;
	flags = (packet->flags_n_seq >> 4)&0x0F;
	return (Packet_Type_t)flags;
}

static void send_signal(
		uint8_t dest_uid,
		uint8_t dest_nid,
		Packet_Type_t signal
		)
{
	build_signal(unique_id, net_id, dest_uid, dest_nid, signal);
	radio->packet_send(buffer.dest_nid, (uint8_t *)&buffer, 6);
}

static void build_signal(
		uint8_t s_uid,
		uint8_t s_nid,
		uint8_t d_uid,
		uint8_t d_nid,
		uint8_t flag
		)
{
	buffer.src_uid = s_uid;
	buffer.src_nid = s_nid;
	buffer.dest_uid = d_uid;
	buffer.dest_nid = d_nid;
	buffer.flags_n_seq = ((flag << 4)&0xF0);
	buffer.data_size = 0;
}
