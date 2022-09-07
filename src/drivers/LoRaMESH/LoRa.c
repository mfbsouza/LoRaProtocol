/* ---------------------------------------------------
		Radioenge Equipamentos de Telecomunicações
   ---------------------------------------------------
	This library contains a set of functions to configure
	and operate the EndDevice LoRaMESH Radioenge

	Modified by: mfbsouza.it@gmail.com
	Date: 04/08/22 (dd,mm,yy)

*/

#include "LoRa.h"
#include <string.h>

/* Frame type */
typedef struct
{
	uint8_t buffer[MAX_BUFFER_SIZE];
	uint8_t size;
	uint8_t command;
} Frame_Typedef;

/* ----- Private Variables ----- */

static const SerialInterface_t *hSerialCommand = NULL;
static const TimerInterface_t *mTimer = NULL;
static Frame_Typedef frame;
static uint16_t deviceId = -1;
static uint16_t deviceNet = -1;
static uint32_t deviceUniqueId = -1;

/* ----- Public Variables ----- */

/* ----- Private Functions ----- */

static void SerialFlush(const SerialInterface_t *hSerial)
{
	while (hSerial->available() > 0) {
		hSerial->read();
	}
}

/**
 * @brief Private function - Gets the ID, NET and UNIQUE ID info from the local or remote device
 * @param idIn[in]: Remote device's id
 * @param idOut[out]: Local device's id
 * @param net[out]: Configured NET on local device
 * @param uniqueId[out]: Device Unique ID
 * @retval MESH_OK or MESH_ERROR
 */
MeshStatus_Typedef LocalRemoteRead(uint16_t idIn, uint16_t *idOut, uint16_t *net, uint32_t *uniqueId)
{
	uint8_t bufferPayload[31];
	uint8_t payloadSize;
	uint8_t i = 0;
	uint8_t command;
	uint16_t id = 0;

	/* Asserts parameters */
	if (net == NULL) return MESH_ERROR;
	if (uniqueId == NULL) return MESH_ERROR;
	if (hSerialCommand == NULL) return MESH_ERROR;

	/* Loads dummy bytes */
	for (i = 0; i < 3; i++) {
		bufferPayload[i] = 0x00;
	}

	/* Prepares frame for transmission */
	if ((idOut == NULL) && (idIn < 1023)) {
		PrepareFrameCommand(idIn, CMD_REMOTEREAD, &bufferPayload[0], i);
	}
	else if (idOut != NULL) {
		PrepareFrameCommand(0, CMD_LOCALREAD, &bufferPayload[0], i);
	}
	else {
		return MESH_ERROR;
	}

	/* Sends packet */
	SendPacket();

	/* Flush serial input buffer */
	SerialFlush(hSerialCommand);

	/* Waits for response */
	if (ReceivePacketCommand(&id, &command, &bufferPayload[0], &payloadSize, 5000) != MESH_OK) {
		return MESH_ERROR;
	}

	/* Checks command */
	if ((command != CMD_REMOTEREAD) && (command != CMD_LOCALREAD)) {
		return MESH_ERROR;
	}

	/* Stores the received data */
	if (idOut != NULL) {
		deviceId = id;
		*idOut = id;
	}
	*net = (uint16_t)bufferPayload[0] | ((uint16_t)(bufferPayload[1]) << 8);
	*uniqueId = (uint32_t)bufferPayload[2] |
				((uint32_t)(bufferPayload[3]) << 8) |
				((uint32_t)(bufferPayload[4]) << 16) |
				((uint32_t)(bufferPayload[5]) << 24);

	return MESH_OK;
}

/* ----- Public Function Definitions ----- */

void lora_init(const SerialInterface_t *serial, const TimerInterface_t *timer)
{
	hSerialCommand = serial;
	mTimer = timer;

	/* update values */
	LocalRead(&deviceId, &deviceNet, &deviceUniqueId);
}

void lora_set_id(uint8_t id)
{
	uint8_t payload[10];
	memset(payload, 0, 10);
	memcpy(&(payload[2]), &deviceUniqueId, 4);
	PrepareFrameCommand((uint16_t)id, CMD_WRITECONFIG, payload, 10);
	SendPacket();

	/**
	 * Need to give some time to LoRaMESH update it's ID
	 * TODO: maybe test more delay values, 100 seems to be OK
	 * */
	mTimer->delay(100);

	/* update values */
	LocalRead(&deviceId, &deviceNet, &deviceUniqueId);
}

MeshStatus_Typedef PrepareFrameCommand(uint16_t id, uint8_t command, uint8_t *payload, uint8_t payloadSize)
{
	if (payload == NULL) return MESH_ERROR;
	if (id > 1023) return MESH_ERROR;

	uint16_t crc = 0;

	frame.size = payloadSize + 5;

	/* Loads the target's ID */
	frame.buffer[0] = id & 0xFF;
	frame.buffer[1] = (id >> 8) & 0x03;

	/* Loads the command */
	frame.buffer[2] = command;

	if ((payloadSize >= 0) && (payloadSize < MAX_PAYLOAD_SIZE)) {
		/* Loads the payload */
		memcpy(&(frame.buffer[3]), payload, payloadSize);

		/* Computes CRC */
		crc = ComputeCRC((&frame.buffer[0]), payloadSize + 3);
		frame.buffer[payloadSize + 3] = crc & 0xFF;
		frame.buffer[payloadSize + 4] = (crc >> 8) & 0xFF;
	}
	else {
		/* Invalid payload size */
		memset(&frame.buffer[0], 0, MAX_BUFFER_SIZE);
		return MESH_ERROR;
	}

	frame.command = 1;

	return MESH_OK;
}

MeshStatus_Typedef SendPacket()
{
	if (frame.size == 0)
		return MESH_ERROR;
	if ((hSerialCommand == NULL) && (frame.command))
		return MESH_ERROR;

	if (frame.command)
	{
		hSerialCommand->write(frame.buffer, frame.size);
	}

	return MESH_OK;
}

MeshStatus_Typedef ReceivePacketCommand(uint16_t *id, uint8_t *command, uint8_t *payload, uint8_t *payloadSize, uint32_t timeout)
{
	uint16_t waitNextByte = 500;
	uint8_t i = 0;
	uint16_t crc = 0;

	/* Assert parameters */
	if (id == NULL) return MESH_ERROR;
	if (command == NULL) return MESH_ERROR;
	if (payload == NULL) return MESH_ERROR;
	if (payloadSize == NULL) return MESH_ERROR;
	if (hSerialCommand == NULL) return MESH_ERROR;
	if (mTimer == NULL) return MESH_ERROR;

	/* Waits for reception */
	while (((timeout > 0) || (i > 0)) && (waitNextByte > 0)) {
		if (hSerialCommand->available() > 0) {
			frame.buffer[i++] = hSerialCommand->read();
			waitNextByte = 500;
		}

		if (i > 0) {
			waitNextByte--;
		}

		timeout--;
		mTimer->delay(1);
	}

	/* In case it didn't get any data */
	if ((timeout == 0) && (i == 0)) {
		return MESH_ERROR;
	}

	/* Checks CRC16 */
	crc = (uint16_t)frame.buffer[i - 2] | ((uint16_t)frame.buffer[i - 1] << 8);
	if (ComputeCRC(&frame.buffer[0], i - 2) != crc) {
		return MESH_ERROR;
	}

	/* Copies ID */
	*id = (uint16_t)frame.buffer[0] | ((uint16_t)frame.buffer[1] << 8);
	/* Copies command */
	*command = frame.buffer[2];
	/* Copies payload size */
	*payloadSize = i - 5;
	/* Copies payload */
	memcpy(payload, &frame.buffer[3], i - 5);

	return MESH_OK;
}

MeshStatus_Typedef LocalRead(uint16_t *id, uint16_t *net, uint32_t *uniqueId)
{
	return LocalRemoteRead(0xFFFF, id, net, uniqueId);
}

MeshStatus_Typedef RemoteRead(uint16_t id, uint16_t *net, uint32_t *uniqueId)
{
	return LocalRemoteRead(id, NULL, net, uniqueId);
}

uint16_t ComputeCRC(uint8_t *data_in, uint16_t length)
{
	uint16_t i;
	uint8_t bitbang, j;
	uint16_t crc_calc;

	crc_calc = 0xC181;

	for (i = 0; i < length; i++) {
		crc_calc ^= (((uint16_t)data_in[i]) & 0x00FF);
		for (j = 0; j < 8; j++) {
			bitbang = crc_calc;
			crc_calc >>= 1;
			if (bitbang & 1) {
				crc_calc ^= 0xA001;
			}
		}
	}

	return (crc_calc & 0xFFFF);
}