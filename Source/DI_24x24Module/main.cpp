#include "main_config.h"
#include "canard/canard.h"
#include "Arduino/Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "common.h"
#include "Arduino/SPI.h"
#include "MCP23S17.h"
#include "MCP23S17_config.h"

#include "uavcan/kplc/IOState.h"
#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/Panic.h"
#include "uavcan/protocol/param/GetSet.h"

#define MAIN_MODULE_NODE_ID 100
#define IOSTATE_MIN_TRANSMIT_INTERVAL_MSEC 500

static uint32_t g_mainModuleLastStatusUpdateTime = 0;

CanardInstance g_canard;              // The canard library instance.
uint8_t g_canard_memory_pool[1024];   // Arena for memory allocation, used by the library.

#define IOSTATE_LPF_TIME_OFFSET_UNSET 0xFF
#define IOSTATE_LPF_TIME_MS 20
static uint8_t g_ioState[3];
static uint8_t g_ioStateStaging[3];
static uint8_t g_ioStateLpfTimeOffsets[8 * 3];
static uint32_t g_ioStateLpfTimeReference = 0;


Timer<4> g_timers;

int8_t ProcessIOState(bool forced = false);

static int8_t handle_KPLC_IOState_Response(CanardRxTransfer* transfer)
{
	uavcan_kplc_IOStateResponse response;
	int8_t ret;
	ret = uavcan_kplc_IOStateResponse_decode(transfer, transfer->payload_len, &response, NULL);
	if (ret < 0) {
		return -FailureReason_CannotDecodeMessage;
	}
	
	if (response.status != 0) {
		return -FailureReason_UnexpectedResponse;
	}
	
	return 0;
}

static int8_t validateMasterNodeStatus(uavcan_protocol_NodeStatus status)
{
	if (status.health != UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK) {
		return -FailureReason_BadNodeStatus;
	}
	if (status.mode != UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL) {
		return -FailureReason_BadNodeStatus;
	}
	if (g_nodeState == NodeState_Operational &&
		status.vendor_specific_status_code != NodeState_Operational) {
		return -FailureReason_MasterStateValidationError;
	}
	
	return 0;
}

static int8_t handle_protocol_NodeStatus(CanardRxTransfer* transfer)
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

static int8_t handle_protocol_param_GetSet(CanardRxTransfer* transfer)
{
	if (transfer->source_node_id != MAIN_MODULE_NODE_ID) {
		return 0;
	}
	
	int8_t ret;
	uavcan_protocol_param_GetSetRequest request;
	ret = uavcan_protocol_param_GetSetRequest_decode(transfer, transfer->payload_len, &request, NULL);
	if (ret < 0) {
		return -FailureReason_CannotDecodeMessage;
	}
	
	ParamKind paramKind;
	if (request.name.data && request.name.len) {
		paramKind = parseParamKind((char*)request.name.data, request.name.len);
	} else {
		paramKind = (ParamKind)request.index;
	}
	
	char paramName[ParamKind_Name_MaxLength + 1];
	uavcan_protocol_param_GetSetResponse response;
	
	response.default_value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
	response.min_value.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY;
	response.max_value.union_tag = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY;
	
	switch (paramKind)
	{
		case ParamKind_NodeState:
		{
			if (request.value.union_tag == UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE)
			{
				g_nodeState = (NodeState)request.value.boolean_value;
			}
			
			const char* paramNamePgm = ParamKind_Names[paramKind];
			int paramKindNameLen = strlen_P(paramNamePgm);
			strcpy_P(&paramName[0], paramNamePgm);
			
			response.name.len = paramKindNameLen;
			response.name.data = (uint8_t*)(&paramName[0]);
			
			response.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE;
			response.value.boolean_value = g_nodeState;
			
			break;
		}
		
		default:
		{
			response.name.len = 0;
			response.name.data = 0;
		}
	}
	
	uint8_t buffer[50]; // TODO
	memset(buffer, 0, sizeof(buffer));
	uint16_t len = uavcan_protocol_param_GetSetResponse_encode(&response, &buffer[0]);
	
	int16_t result = canardRequestOrRespond(&g_canard,
		transfer->source_node_id,
		UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
		UAVCAN_PROTOCOL_PARAM_GETSET_ID,
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
		case UAVCAN_KPLC_IOSTATE_ID:
			*out_data_type_signature = UAVCAN_KPLC_IOSTATE_SIGNATURE;
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
				case UAVCAN_PROTOCOL_PARAM_GETSET_ID:
					handler = handle_protocol_param_GetSet;
					break;
			}
			break;
		case CanardTransferTypeResponse:
			switch(transfer->data_type_id)
			{
				case UAVCAN_KPLC_IOSTATE_ID:
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

int8_t ValidateMasterNodeState()
{
	uint32_t now = millis();
	
	// TODO: decrease multiplier.
	if ((now - g_mainModuleLastStatusUpdateTime) > 3 * CANARD_NODESTATUS_PERIOD_MSEC) {
		return -FailureReason_MasterStateValidationError;
	}
	
	return 0;
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
	MCP23S17_A.initialize();
	MCP23S17_A.writeDataDirection(INPUT);
	MCP23S17_A.writePullup(HIGH);
	
	MCP23S17_B.initialize();
	MCP23S17_B.writeDataDirection(INPUT);
	MCP23S17_B.writePullup(HIGH);	
	
	int8_t result = g_timers.every(IOSTATE_MIN_TRANSMIT_INTERVAL_MSEC, ProcessIOStateTimerCallback);
	if (result < 0) {
		return -FailureReason_CannotInit;
	}
	
	return 0;
}

uint8_t sendIOState()
{
	uint8_t buffer[UAVCAN_KPLC_IOSTATE_REQUEST_MAX_SIZE];
	static uint8_t transfer_id = 0;
	
	uavcan_kplc_IOStateRequest request;
	for (uint8_t i = 0; i < 3; i++)
	{
		uint8_t state = g_ioState[i];
		request.state[i] = state;
		request.state_inv[i] = ~state;
	}

	uint16_t len = uavcan_kplc_IOStateRequest_encode(&request, &buffer[0]);
	
	int16_t result = canardRequestOrRespond(&g_canard,
		MAIN_MODULE_NODE_ID,
		UAVCAN_KPLC_IOSTATE_SIGNATURE,
		UAVCAN_KPLC_IOSTATE_ID,
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

int8_t ProcessIOState(bool forced)
{
	uint16_t raw_a = MCP23S17_A.read();
	uint16_t raw_b = MCP23S17_B.read();
	
	uint8_t current[3] = {
		(uint8_t)raw_a,
		(uint8_t)(raw_a >> 8),
		(uint8_t)(raw_b >> 8)
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
		for (int j = 0; i < 8; j++)
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
	PORTD |= _BV(PORTD6);
}

void resetLed()
{
	PORTD &= ~_BV(PORTD6);
}

bool swapLedState;
void swapLed(uint8_t _)
{
	if (!swapLedState) {
		setLed();
		swapLedState = true;
	}
	else {
		resetLed();
		swapLedState = false;
	}
}

ISR(INT0_vect)
{
	handleCanRxInterrupt();
}

int main(void)
{	
	DDRD |= _BV(PORTD6); // LED
	DDRB |= _BV(PORTB1); // MCP2515 Reset
	
	PORTB |= _BV(PORTB1);
	PORTC |= _BV(PORTC5);
	PORTC |= _BV(PORTD0) | _BV(PORTD1) | _BV(PORTD3) | _BV(PORTD4);
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
	
	g_timers.every(500, swapLed);
	
	// Set up MCP2515 interrupt.
	EICRA = _BV(ISC01);	// Trigger INT0 on falling edge
	EIMSK = _BV(INT0);	// Enable INT0
	
	wdt_enable(WDTO_250MS);
	WDTCSR |= _BV(WDE);
	
	while (1)
	{
		wdt_reset();
		
		switch (g_nodeState)
		{
			case NodeState_Initial:
			{
				result = sendCanard();
				if (result < 0)
				{
					fail(result);
					continue;
				}
 				
 				receiveCanard();
				g_timers.update();
				
				break;
			}
			case NodeState_Operational:
			{
				result = sendCanard();
				if (result < 0)
				{
					fail(result);
					continue;
				}
				
				receiveCanard();
				
				result = ValidateMasterNodeState();
				if (result < 0) {
					fail(result);
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
				wdt_disable();
				cli();
				
				resetLed();
				_delay_ms(1000);
				
				for (int i = 0; i < g_failureReason; i++) {
					setLed();
					_delay_ms(300);
					resetLed();
					_delay_ms(300);
				}

				break;
			}
		}
	}
}

