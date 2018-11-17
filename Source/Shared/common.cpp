#include "common.h"

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

static uavcan_protocol_GetNodeInfoResponse makeNodeInfoMessage()
{
	return uavcan_protocol_GetNodeInfoResponse {
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
		.name = {0,0}
	};
}

int8_t handle_protocol_GetNodeInfo(CanardRxTransfer* transfer)
{
	uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];
	memset(buffer, 0, UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE);
	uavcan_protocol_GetNodeInfoResponse dto = makeNodeInfoMessage();
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

void fail(int8_t reason)
{
	g_nodeStatusMode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OFFLINE;
	g_nodeStatusHealth = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL;
	g_nodeState = NodeState_Error;
	g_failureReason = (FailureReason)(-reason);
	
	uint8_t buffer[UAVCAN_PROTOCOL_PANIC_MAX_SIZE];
	static uint8_t transfer_id = 0;
	
	uavcan_protocol_Panic msg;
	msg.reason_text.len = 1;
	uint8_t reasonText[1] = { (uint8_t)(-reason) };
	msg.reason_text.data = &reasonText[0];
	int16_t len = uavcan_protocol_Panic_encode(&msg, &buffer[0]);
	
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

static void canardCleanupStaleTransfersHandler(uint8_t timerId)
{
	canardCleanupStaleTransfers(&g_canard, GET_MICROS);
}

int8_t setup(void)
{
	int8_t result;
	
	arduino_init();
	
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
	
	canardSetLocalNodeID(&g_canard, CANARD_NODE_ID);
	
	if (canardAVRConfigureAcceptanceFilters(CANARD_NODE_ID) < 0) {
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
		const int tx_res = canardAVRTransmit(txf);
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

void receiveCanard(void)
{
	CanardCANFrame rx_frame;
	int res = canardAVRReceive(&rx_frame);
	if (res)
	{
		canardHandleRxFrame(&g_canard, &rx_frame, GET_MICROS);
	}
}

// This function is called upon a HARDWARE RESET:
void wdt_first(void) __attribute__((naked)) __attribute__((section(".init7")));

void wdt_first(void)
{
 	if ((MCUSR & _BV(WDRF)) == _BV(WDRF))
 	{
		g_failureReason = FailureReason_Watchdog;
		g_nodeState = NodeState_Error;
		
		MCUSR &= ~(_BV(WDRF)); // Clear reset flag.
		wdt_disable();
		// http://www.atmel.com/webdoc/AVRLibcReferenceManual/FAQ_1faq_softreset.html
		// https://www.pocketmagic.net/avr-watchdog/
	}
}