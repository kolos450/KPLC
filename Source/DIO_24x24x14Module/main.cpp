#include "main_config.h"
#include "canard/canard.h"
#include "Arduino/Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "common.h"
#include "Arduino/SPI.h"
#include "MCP23S17/MCP23S17.h"
#include "MCP23S17_config.h"

#include "kplc/IOStateFrame.h"
#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/Panic.h"
#include "uavcan/protocol/param/GetSet.h"

#define MAIN_MODULE_NODE_ID 100
#define IOSTATE_MIN_TRANSMIT_INTERVAL_MSEC 500

CanardInstance g_canard;              // The canard library instance.
uint8_t g_canard_memory_pool[CANARD_MEMORY_POOL_SIZE];   // Arena for memory allocation, used by the library.

#define IOSTATE_LPF_TIME_OFFSET_UNSET 0xFF
#define IOSTATE_LPF_TIME_MS 20
static uint8_t g_ioState[3];
static uint8_t g_ioStateStaging[3];
static uint8_t g_ioStateLpfTimeOffsets[8 * 3];
static uint32_t g_ioStateLpfTimeReference = 0;


Timer<4> g_timers;

int8_t ProcessIOState(bool forced = false);

uint8_t readNodeId()
{
	uint8_t val =	((PINC & _BV(PINC5))) |
					((PIND & _BV(PIND0)) << 4) |
					((PIND & _BV(PIND1)) << 2) |
					((PIND & _BV(PIND3)) >> 1) |
					((PIND & _BV(PIND4)) >> 3) |
					((PINE & _BV(PINE0)));
	return val ^ 0x3F;
}

static void ApplyIOState(uint8_t* state)
{
	uint8_t state_0 = state[0];
	uint8_t state_1 = state[1];
	
	uint8_t stateB = ((state_0 & 0x1) << 7)
					| ((state_0 & 0x80) >> 7)
					| ((state_1 & 0x17) << 1)
					| ((state_1 & 0x8) << 3)
					| ((state_1 & 0x20) >> 1);
		
	PORTB &= ~0x1;
	PORTB |= ((state_0 & 0x2) >> 1);
	PORTC &= ~0x18;
	PORTC |= ((state_0 & 0x60) >> 2);
	PORTD &= ~0xE0;
	PORTD |= ((state_0 & 0x4) << 5)
			| ((state_0 & 0x8) << 3)
			| ((state_0 & 0x10) << 1);
	
	MCP23S17_B.write(stateB);
}

static int8_t handle_KPLC_IOStateFrame_Request(CanardRxTransfer* transfer)
{
	if (transfer->source_node_id != MAIN_MODULE_NODE_ID) {
		return 0;
	}
	
	kplc_IOStateFrameRequest request;
	int8_t ret;
	uint8_t responseStatus = KPLC_IOSTATEFRAME_RESPONSE_STATUS_OK;
	
	uint8_t dynamicArrayBuffer[KPLC_IOSTATEFRAME_REQUEST_DATA_MAX_LENGTH];
	uint8_t* dynamicArrayBufferPtr = dynamicArrayBuffer;
	ret = kplc_IOStateFrameRequest_decode(transfer, transfer->payload_len, &request, &dynamicArrayBufferPtr);
	if (ret < 0) {
		responseStatus = KPLC_IOSTATEFRAME_RESPONSE_STATUS_ERROR_UNKNOWN;
		ret = -FailureReason_CannotDecodeMessage;
	}
	else if (request.frameIndex != 0 || request.data.len != 2) {
		responseStatus = KPLC_IOSTATEFRAME_RESPONSE_STATUS_ERROR_INCONSISTENCY;
	}
	else {
		ApplyIOState(request.data.data);
	}
	
	uint8_t buffer[KPLC_IOSTATEFRAME_RESPONSE_MAX_SIZE];
	memset(buffer, 0, sizeof(buffer));
	kplc_IOStateFrameResponse response = {
		.status = responseStatus
	};
	uint16_t len = kplc_IOStateFrameResponse_encode(&response, &buffer[0]);
	int16_t ret_int16 = canardRequestOrRespond(&g_canard,
		transfer->source_node_id,
		KPLC_IOSTATEFRAME_SIGNATURE,
		KPLC_IOSTATEFRAME_ID,
		&transfer->transfer_id,
		transfer->priority,
		CanardResponse,
		&buffer[0],
		(uint16_t)len);
	
	if (ret < 0) {
		return ret;
	}
	
	if (ret_int16 < 0) {
		return (int8_t)ret_int16;
	}
	
	return 0;
}

static int8_t handle_KPLC_IOState_Response(CanardRxTransfer* transfer)
{
	kplc_IOStateFrameResponse response;
	int8_t ret;
	ret = kplc_IOStateFrameResponse_decode(transfer, transfer->payload_len, &response, NULL);
	if (ret < 0) {
		return -FailureReason_CannotDecodeMessage;
	}
	
	if (response.status != 0) {
		return -FailureReason_UnexpectedResponse;
	}
	
	return 0;
}

bool shouldAcceptTransfer(
	const CanardInstance* ins,
	uint64_t* out_data_type_signature,
	uint16_t data_type_id,
	CanardTransferType transfer_type,
	uint8_t source_node_id)
{
	switch (data_type_id)
	{
		case UAVCAN_PROTOCOL_GETNODEINFO_ID:
			*out_data_type_signature = UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE;
			return true;
		case UAVCAN_PROTOCOL_NODESTATUS_ID:
			*out_data_type_signature = UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE;
			return true;
		case KPLC_IOSTATEFRAME_ID:
			*out_data_type_signature = KPLC_IOSTATEFRAME_SIGNATURE;
			return true;
		case UAVCAN_PROTOCOL_PARAM_GETSET_ID:
			*out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
			return true;
		case UAVCAN_PROTOCOL_PANIC_ID:
			*out_data_type_signature = UAVCAN_PROTOCOL_PANIC_SIGNATURE;
			return true;
		default:
			return false;
	}
}

static int8_t handle_protocol_Panic(CanardRxTransfer* transfer)
{
	return -FailureReason_PanicReceived;
}

void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer)
{
	uavcanMessageHandler_t handler = NULL;
	
	switch (transfer->transfer_type)
	{
		case CanardTransferTypeRequest:
			switch(transfer->data_type_id)
			{
				case UAVCAN_PROTOCOL_GETNODEINFO_ID:
					handler = handle_protocol_GetNodeInfo;
					break;
				case KPLC_IOSTATEFRAME_ID:
					handler = handle_KPLC_IOStateFrame_Request;
					break;
				case UAVCAN_PROTOCOL_PARAM_GETSET_ID:
					handler = handle_protocol_param_GetSet;
					break;
			}
			break;
		case CanardTransferTypeResponse:
			switch(transfer->data_type_id)
			{
				case KPLC_IOSTATEFRAME_ID:
					handler = handle_KPLC_IOState_Response;
					break;
			}
			break;
		case CanardTransferTypeBroadcast:
			switch(transfer->data_type_id)
			{
				case UAVCAN_PROTOCOL_NODESTATUS_ID:
					handler = handle_protocol_NodeStatus;
					break;
				case UAVCAN_PROTOCOL_PANIC_ID:
					handler = handle_protocol_Panic;
					break;
			}
			break;
	}
	
	if (handler != NULL)
	{
		int8_t result = handler(transfer);
		if (result < 0)
		{
			fail(result);
		}
	}
}

void ProcessIOStateTimerCallback(uint8_t _)
{
	if (g_nodeState == NodeState_Operational)
	{
		ProcessIOState(true);
	}
}

int8_t setupMCP23S17()
{
	if (MCP23S17_A.initialize() < 0)
	{
		return -FailureReason_CannotInit;
	}
	MCP23S17_A.writeDataDirection(0xFFFF);
	MCP23S17_A.writePullup(0xFFFF);
	
	if (MCP23S17_B.initialize() < 0)
	{
		return -FailureReason_CannotInit;
	}
	MCP23S17_B.writeDataDirection(0xFF00);
	MCP23S17_B.writePullup(0xFF00);
	
	int8_t result = g_timers.every(IOSTATE_MIN_TRANSMIT_INTERVAL_MSEC, ProcessIOStateTimerCallback);
	if (result < 0)
	{
		return -FailureReason_CannotInit;
	}
	
	return 0;
}

uint8_t sendIOState()
{
	uint8_t buffer[KPLC_IOSTATEFRAME_REQUEST_MAX_SIZE];
	static uint8_t transfer_id = 0;
	
	kplc_IOStateFrameRequest request = {
		.frameIndex = 0,
		.data = {
			.len = sizeof(g_ioState),
			.data = g_ioState
		}
	};

	uint16_t len = kplc_IOStateFrameRequest_encode(&request, &buffer[0]);
	
	int16_t result = canardRequestOrRespond(&g_canard,
		MAIN_MODULE_NODE_ID,
		KPLC_IOSTATEFRAME_SIGNATURE,
		KPLC_IOSTATEFRAME_ID,
		&transfer_id,
		CANARD_TRANSFER_PRIORITY_MEDIUM,
		CanardRequest,
		&buffer[0],
		(uint16_t)len);
	
	if (result < 0) {
		return (int8_t)result;
	}
	
	return 0;
}

uint8_t reverse(uint8_t b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

int8_t ProcessIOState(bool forced)
{
	uint16_t raw_a = MCP23S17_A.read();
	uint16_t raw_b = MCP23S17_B.read();
	
	uint8_t current[3] = {
		(uint8_t)~reverse(raw_b >> 8),
		(uint8_t)~reverse(raw_a),
		(uint8_t)~reverse(raw_a >> 8),
	};
	
	uint8_t result[3];
	memcpy(&result[0], &g_ioState[0], sizeof(g_ioState));
	
	uint32_t now = millis();
	uint32_t timeDiff = now - g_ioStateLpfTimeReference;
	g_ioStateLpfTimeReference = now;
	uint8_t diffIsBig = timeDiff >= IOSTATE_LPF_TIME_MS;
	uint8_t smallDiff = (uint8_t)timeDiff;
	
	uint8_t resultUpdated = false;
	
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			uint8_t current_ij = current[i] & _BV(j);
			if ((g_ioStateStaging[i] & _BV(j)) != current_ij)
			{
				g_ioStateStaging[i] = (g_ioStateStaging[i] & ~_BV(j)) | current_ij;
				
				if ((result[i] & _BV(j)) == current_ij)
				{
					g_ioStateLpfTimeOffsets[i * 8 + j] = IOSTATE_LPF_TIME_OFFSET_UNSET;
				}
				else
				{
					g_ioStateLpfTimeOffsets[i * 8 + j] = 0;
				}
			}
			else
			{
				uint8_t timeOffset = g_ioStateLpfTimeOffsets[i * 8 + j];
				if (timeOffset == IOSTATE_LPF_TIME_OFFSET_UNSET)
				{
					// Nothing to do.
				}
				else if (diffIsBig ||
					smallDiff + timeOffset > IOSTATE_LPF_TIME_MS)
				{
					g_ioStateLpfTimeOffsets[i * 8 + j] = IOSTATE_LPF_TIME_OFFSET_UNSET;
					result[i] = (g_ioStateStaging[i] & ~_BV(j)) | current_ij;
					resultUpdated = true;
				}
				else
				{
					g_ioStateLpfTimeOffsets[i * 8 + j] = smallDiff + timeOffset;
				}
			}
		}
	}
	
	if (forced || resultUpdated)
	{
		memcpy(&g_ioState[0], &result[0], sizeof(g_ioState));		
		int8_t ret = sendIOState();
		if (ret < 0)
		{
			return ret;
		}
	}
	
	return 0;
}

void setLed()
{
	PORTE |= _BV(PORTE1);
}

void resetLed()
{
	PORTE &= ~_BV(PORTE1);
}

ISR(INT0_vect)
{
	handleCanRxInterrupt();
}

void enableCanRxInterrupt()
{
	EIMSK |= _BV(INT0);
}

void disableCanRxInterrupt()
{
	EIMSK &= ~_BV(INT0);
}

void enableCanController() {
	PORTB |= _BV(PORTB1);
}

void disableCanController() {
	PORTB &= ~_BV(PORTB1);
}

int main(void)
{	
	wdt_enable(WDTO_250MS);
	wdt_reset();
	
	DDRE |= _BV(PORTE1); // LED
	
	// McOut[8..13]
	DDRB |= _BV(PORTB0);
	DDRC |= _BV(PORTC3) | _BV(PORTC4);
	DDRD |= _BV(PORTD5) | _BV(PORTD6) | _BV(PORTD7);
	
	PORTB |= _BV(PORTB1);
	PORTC |= _BV(PORTC5);
	PORTD |= _BV(PORTD0) | _BV(PORTD1) | _BV(PORTD3) | _BV(PORTD4);
	PORTE |= _BV(PORTE0);
	
	memset(&g_ioStateLpfTimeOffsets, IOSTATE_LPF_TIME_OFFSET_UNSET, sizeof(g_ioStateLpfTimeOffsets));
	memset(&g_ioState, 0, sizeof(g_ioState));
	memset(&g_ioStateStaging, 0, sizeof(g_ioStateStaging));
	
	int8_t result;
	
	result = setup();
	if (result < 0) {
		fail(result);
	}
	
	result = setupMCP23S17();
	if (result < 0) {
		fail(result);
	}
	
	g_nodeStatusMode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
	
	setLed();
	
	// Set up MCP2515 interrupt.
	EICRA = _BV(ISC01);	// Trigger INT0 on falling edge
	EIMSK = _BV(INT0);	// Enable INT0
	
	sei();
	
	resetTransceiverState();
	
	initializeMainModuleStateUpdateTime();
	
	while (1)
	{
		wdt_reset();
		
		switch (g_nodeState)
		{
			case NodeState_Initial:
			{
				result = validateTransceiverState();
				if (result < 0) {
					resetCanController();
				}
				
				result = sendCanard();
				if (result < 0) {
					fail(result);
					continue;
				}
 				
 				receiveCanard();
				if (!checkNodeHealth()) {
					continue;
				}
				
				g_timers.update();
				
				break;
			}
			case NodeState_Operational:
			{
				result = validateTransceiverState();
				if (result < 0) {
					fail(result);
					continue;
				}
				
				result = sendCanard();
				if (result < 0) {
					fail(result);
					continue;
				}
				
				receiveCanard();
				if (!checkNodeHealth()) {
					continue;
				}
				
				validateMasterNodeState();
				if (!checkNodeHealth()) {
					continue;
				}
				
				result = ProcessIOState();
				if (result < 0) {
					fail(result);
					continue;
				}
				
				g_timers.update();
				
				break;
			}
			case NodeState_Error:
			{
				// Set all outputs to the low state.
				PORTB &= ~_BV(PORTB0);
				PORTC &= ~(_BV(PORTC3) | _BV(PORTC4));
				PORTD &= ~(_BV(PORTD5) | _BV(PORTD6) | _BV(PORTD7));
				MCP23S17_B.write(0);

				cli();
				
				resetLed();
				delayMsWhileWdtReset(1000);
				
				for (int i = 0; i < (uint8_t)g_failureReason; i++) {
					setLed();
					delayMsWhileWdtReset(300);
					resetLed();
					delayMsWhileWdtReset(300);
				}

				break;
			}
			default:
			{
				fail(-FailureReason_InvalidArgument);
				break;
			}
		}
	}
}

