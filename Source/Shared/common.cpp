#include "common.h"
#include "avr-can-lib/can.h"

uint32_t g_mainModuleLastStatusUpdateTime = 0;
uint8_t g_nodeStatusHealth = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
uint8_t g_nodeStatusMode = UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION;
NodeState g_nodeState = NodeState_Initial;
FailureReason g_failureReason;

static uavcan_protocol_NodeStatus makeNodeStatusMessage()
{
	return uavcan_protocol_NodeStatus {
		.uptime_sec = millis() / 1000,
		.health = g_nodeStatusHealth,
		.mode = g_nodeStatusMode,
		.sub_mode = 0,
		.vendor_specific_status_code = g_nodeState,
	};
}

static void broadcastNodeStatus(uint8_t timerId)
{
	uint8_t buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];
	static uint8_t transfer_id = 0; // This variable MUST BE STATIC; refer to the libcanard documentation for the background.
	
	uavcan_protocol_NodeStatus status = makeNodeStatusMessage();
	int16_t len = uavcan_protocol_NodeStatus_encode(&status, &buffer[0]);
	
	int16_t result = canardBroadcast(&g_canard,
		UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
		UAVCAN_PROTOCOL_NODESTATUS_ID,
		&transfer_id,
		CANARD_TRANSFER_PRIORITY_LOW,
		buffer,
		len);
	
	if (result < 0) {
		fail((int8_t)result);
	}
}

int8_t handle_protocol_GetNodeInfo(CanardRxTransfer* transfer)
{
	const int bufferLength = 41 + sizeof(APP_NODE_NAME) - 1;
	uint8_t buffer[bufferLength];
	memset(buffer, 0, bufferLength);

	char appNodeName[] = APP_NODE_NAME;
	
	uavcan_protocol_GetNodeInfoResponse dto = {
		.status = makeNodeStatusMessage(),
		.software_version = {
			.major = APP_VERSION_MAJOR,
			.minor = APP_VERSION_MINOR,
			.optional_field_flags = 0
		},
		.hardware_version = {
			.major = 0,
			.minor = 0,
			.unique_id = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			.certificate_of_authenticity = {0,0}
		},
		.name = {sizeof(APP_NODE_NAME), (uint8_t*)(&appNodeName)}
	};
	
	const uint16_t len = uavcan_protocol_GetNodeInfoResponse_encode(&dto, &buffer[0]);
	int16_t result = canardRequestOrRespond(&g_canard,
		transfer->source_node_id,
		UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,
		UAVCAN_PROTOCOL_GETNODEINFO_ID,
		&transfer->transfer_id,
		transfer->priority,
		CanardResponse,
		&buffer[0],
		(uint16_t)len);
	
	if (result < 0) {
		return (int8_t)result;
	}
	
	return 0;
}

void fail(int8_t reason, uint8_t* message, uint8_t message_length)
{
	g_nodeStatusMode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OFFLINE;
	g_nodeStatusHealth = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL;
	g_nodeState = NodeState_Error;
	g_failureReason = (FailureReason)(-reason);
	
	uavcan_protocol_Panic msg;
	
	char messageBuffer[UAVCAN_PROTOCOL_PANIC_REASON_TEXT_MAX_LENGTH];
	itoa(-reason, messageBuffer, 10);
	auto offset = strlen(messageBuffer);
	messageBuffer[offset++] = ':';
	
	if (message_length != 0)
	{
		auto msgLen = min(message_length, UAVCAN_PROTOCOL_PANIC_REASON_TEXT_MAX_LENGTH - offset);
		memcpy(messageBuffer + offset, message, msgLen);
		offset += msgLen;
	}
	
	uint8_t buffer[UAVCAN_PROTOCOL_PANIC_MAX_SIZE];
	static uint8_t transfer_id = 0;	
	msg.reason_text.len = offset;
	msg.reason_text.data = (uint8_t*)messageBuffer;
	int16_t len = uavcan_protocol_Panic_encode(&msg, buffer);
	
	int16_t result = canardBroadcast(&g_canard,
		UAVCAN_PROTOCOL_PANIC_SIGNATURE,
		UAVCAN_PROTOCOL_PANIC_ID,
		&transfer_id,
		CANARD_TRANSFER_PRIORITY_HIGH,
		buffer,
		len);
	
	if (result < 0) {
		return;
	}
	
	sendCanard();
}

bool checkNodeHealth() {
	return g_nodeState != NodeState_Error;
}

static void canardCleanupStaleTransfersHandler(uint8_t timerId)
{
	canardCleanupStaleTransfers(&g_canard, GET_MICROS);
}

int8_t setup(void)
{
	int8_t result;
	
	arduino_init();
	
	disableCanRxInterrupt();
	
	result = canardAVRInit(CAN_SPEED);
	if (result) {
		return -FailureReason_CannotInit;
	}
	
	canardInit(
		&g_canard,                         // Uninitialized library instance
		g_canard_memory_pool,              // Raw memory chunk used for dynamic allocation
		sizeof(g_canard_memory_pool),      // Size of the above, in bytes
		onTransferReceived,                // Callback, see CanardOnTransferReception
		shouldAcceptTransfer,              // Callback, see CanardShouldAcceptTransfer
		NULL);
		
	uint8_t nodeId = readNodeId();
	
	canardSetLocalNodeID(&g_canard, nodeId);
	
	if (canardAVRConfigureAcceptanceFilters(nodeId) < 0) {
		return -FailureReason_CannotInit;
	}
	
	result = g_timers.every(CANARD_NODESTATUS_PERIOD_MSEC, broadcastNodeStatus);
	if (result < 0) {
		return -FailureReason_CannotInit;
	}
	
	result = g_timers.every(CANARD_RECOMMENDED_STALE_TRANSFER_CLEANUP_INTERVAL_USEC / 1000, canardCleanupStaleTransfersHandler);
	if (result < 0) {
		return -FailureReason_CannotInit;
	}
	
	return 0;
}

int8_t sendCanard(void)
{
	const CanardCANFrame* txf = canardPeekTxQueue(&g_canard);
	while (txf)
	{
		disableCanRxInterrupt();
		const int tx_res = canardAVRTransmit(txf);
		enableCanRxInterrupt();
		if (tx_res < 0)
		{
			// Failure - drop the frame and report.
			return -FailureReason_DriverCannotTransmit;
		}
		if (tx_res > 0)
		{
			canardPopTxQueue(&g_canard);
		}
		txf = canardPeekTxQueue(&g_canard);
	}
	
	return 0;
}

// REM: sizeof(CanardCANFrame) == 13
#define RX_FRAME_BUFFER_LENGTH 16
static CanardCANFrame rx_frame_buffer[RX_FRAME_BUFFER_LENGTH];
static CanardCANFrame rx_frame_buffer_copy[RX_FRAME_BUFFER_LENGTH];
static uint8_t rx_frame_buffer_count = 0;

void handleCanRxInterrupt()
{
	while (1)
	{
		int res = canardAVRReceive(&(rx_frame_buffer[rx_frame_buffer_count]));
		if (res == 0)
		{
			return;
		}
		
		rx_frame_buffer_count++;
		 
		// TODO: 13 bytes are lost here.
		if (rx_frame_buffer_count == RX_FRAME_BUFFER_LENGTH)
		{
			fail(-FailureReason_RXBufferOverflow);
		}
	}
}

int8_t validateTransceiverState()
{
	disableCanRxInterrupt();
	uint8_t canErrorFlags = can_read_error_flags();
	enableCanRxInterrupt();
	
	if (canErrorFlags)
	{
		// Handle a service frame buffer overflow error only.
		if (canErrorFlags & CanErrorFlags_Rx1Overflow)
		{
			return -FailureReason_CAN_DataOverrun;
		}
		else if (canErrorFlags
			& (CanErrorFlags_ErrorWarning
			| CanErrorFlags_RxWarning | CanErrorFlags_TxWarning
			| CanErrorFlags_RxErrorPassive | CanErrorFlags_TxErrorPassive
			| CanErrorFlags_TxBusOff))
		{
			return -FailureReason_CAN_BusError;
		}
	}
	
	return 0;
}

void resetTransceiverState()
{
	disableCanRxInterrupt();
	can_reset_error_flags();
	enableCanRxInterrupt();
}

void receiveCanard(void)
{
	if (rx_frame_buffer_count == 0)
	{
		return;
	}
	
	disableCanRxInterrupt();
	uint8_t framesNum = rx_frame_buffer_count;
	for (uint8_t i = 0; i < framesNum; i++)
	{
		rx_frame_buffer_copy[i] = rx_frame_buffer[i];
	}
	rx_frame_buffer_count = 0;
	enableCanRxInterrupt();
	
	for (uint8_t i = 0; i < framesNum; i++)
	{
		canardHandleRxFrame(&g_canard, &rx_frame_buffer_copy[i], GET_MICROS);
	}
}

// This function is called upon a HARDWARE RESET:
void wdt_first(void) __attribute__((naked)) __attribute__((section(".init7")));

void wdt_first(void)
{
 	if ((MCUSR & _BV(WDRF)) == _BV(WDRF))
 	{
		//g_failureReason = FailureReason_Watchdog;
		//g_nodeState = NodeState_Error;
		
		MCUSR &= ~(_BV(WDRF)); // Clear reset flag.
		//wdt_disable();
		// http://www.atmel.com/webdoc/AVRLibcReferenceManual/FAQ_1faq_softreset.html
		// https://www.pocketmagic.net/avr-watchdog/
	}
}

ParamKind parseParamKind(char* name, int len)
{
	if (len > ParamKind_Name_MaxLength)
		return (ParamKind)-1;
		
	char nameCopy[ParamKind_Name_MaxLength + 1];
	memcpy(&nameCopy, name, len);
	memset(&nameCopy + len, 0, ParamKind_Name_MaxLength + 1 - len);
	
	for (uint8_t i = 0; i <= ParamKind_Max; i++)
	{
		const char* pgmPtr = ParamKind_Names[i];
		uint8_t pgmStringLen = strlen_P(pgmPtr);
		if (pgmStringLen == pgmStringLen &&
			strcmp_P(&nameCopy[0], pgmPtr) == 0)
		{
			return (ParamKind)i;
		}
	}
	
	return (ParamKind)-1;
}

static int8_t validateMasterNodeStatus(uavcan_protocol_NodeStatus status)
{
	switch (g_nodeState)
	{
		case NodeState_Initial:
		case NodeState_Error:
		{
			break;
		}
		
		case NodeState_Operational:
		{
			if (status.health != UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK)
			{
				return -FailureReason_BadNodeStatus;
			}
			if (status.mode != UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL)
			{
				return -FailureReason_BadNodeStatus;
			}
			if (status.vendor_specific_status_code != NodeState_Operational)
			{
				return -FailureReason_MasterStateValidationError;
			}
			break;
		}
		
		default:
		{
			fail(-FailureReason_InvalidArgument);
			break;
		}
	}
	
	return 0;
}

void validateMasterNodeState()
{
	uint32_t now = millis();
	
	// TODO: decrease multiplier.
	if ((now - g_mainModuleLastStatusUpdateTime) > 3 * CANARD_NODESTATUS_PERIOD_MSEC) {
		uint32_t difference = now - g_mainModuleLastStatusUpdateTime;
		char buffer[11];
		itoa(difference, buffer, 10);
		uint8_t len = strlen(buffer);
		if (len > 4) {
			len = 0; // Panic message length restrictions.
		}
		fail(-FailureReason_MasterStatusTimeoutOverflow, (uint8_t*)buffer, len);
	}
}

int8_t handle_protocol_NodeStatus(CanardRxTransfer* transfer)
{
	if (transfer->source_node_id != MAIN_MODULE_NODE_ID) {
		return 0;
	}
	
	int8_t ret;
	uavcan_protocol_NodeStatus status;
	ret = uavcan_protocol_NodeStatus_decode(transfer, transfer->payload_len, &status, NULL);
	if (ret < 0) {
		return -FailureReason_CannotDecodeMessage;
	}
	
	g_mainModuleLastStatusUpdateTime = millis();
	
	ret = validateMasterNodeStatus(status);
	if (ret < 0) {
		return ret;
	}
	
	return 0;
}

void initializeMainModuleStateUpdateTime()
{
	g_mainModuleLastStatusUpdateTime = millis();
}

void delayMsWhileWdtReset(uint16_t time)
{
	while (time)
	{
		wdt_reset();
		if (time >= 100) {
			_delay_ms(100);
			time -= 100;
		}
		else if(time >= 10) {
			_delay_ms(10);
			time -= 10;
		}
		else if(time >= 1) {
			_delay_ms(1);
			time -= 1;
		}
	}
}