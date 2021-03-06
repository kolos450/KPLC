#include "main_config.h"
#include "canard/canard.h"
#include "Arduino/Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "common.h"

#include "uavcan/kplc/IOState.h"
#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/Panic.h"
#include "uavcan/protocol/param/GetSet.h"

CanardInstance g_canard;              // The canard library instance.
uint8_t g_canard_memory_pool[1024];   // Arena for memory allocation, used by the library.

Timer<4> g_timers;

uint8_t readNodeId()
{
	uint8_t val =	((PINA & _BV(PINA3)) << 2) |
					((PINA & _BV(PINA2)) << 2) |
					((PINA & _BV(PINA1)) << 2) |
					((PINA & _BV(PINA0)) << 2) |
					((PINB & _BV(PINB0)) << 1) |
					((PINB & _BV(PINB1)) >> 1);
	return val ^ 0x3F;
}

static int8_t ValidateIOStateRequest(uavcan_kplc_IOStateRequest request)
{
	for (uint8_t i = 0; i < ARRAY_SIZE(request.state); i++)
	{
		uint8_t val = request.state[i];
		uint8_t inv = ~val;
		if (inv != request.state_inv[i])
			return -FailureReason_CannotDecodeMessage;
	}
	
	return 0;
}

//  0   PA6    4   PC7      8  PC5     12  PD6     16  PD2
//  1   PA5    5   PA7      9  PC0     13  PD7     17  PD3
//  2   PA4	   6   PC3     10  PC1     14  PD4     18  PD0
//  3   PC6	   7   PC4     11  PC2     15  PD5     19  PD1
static void ApplyIOState(uint8_t* state)
{
	uint8_t state_0 = state[0];
	uint8_t state_1 = state[1];
	uint8_t state_2 = state[2];
	
	PORTA &= ~0xF0;
	PORTA |= ((state_0 & 0x1) << 6)
			| ((state_0 & 0x2) << 4)
			| ((state_0 & 0x24) << 2);
	PORTC = ((state_0 & 0x18) << 3)
			| ((state_0 & 0xC0) >> 3)
			| ((state_1 & 0x1) << 5)
			| ((state_1 & 0xE) >> 1);
	PORTD = ((state_1 & 0x30) << 2)
			| ((state_1 & 0xC0) >> 2)
			| ((state_2 & 0x3) << 2)
			| ((state_2 & 0xC) >> 2);
}

static int8_t handle_KPLC_IOState_Request(CanardRxTransfer* transfer)
{
	if (transfer->source_node_id != MAIN_MODULE_NODE_ID) {
		return 0;
	}
	
	uavcan_kplc_IOStateRequest request;
	int8_t ret;
	uint8_t responseStatus = UAVCAN_KPLC_IOSTATE_STATUS_OK;
	ret = uavcan_kplc_IOStateRequest_decode(transfer, transfer->payload_len, &request, NULL);
	if (ret < 0) {
		responseStatus = UAVCAN_KPLC_IOSTATE_STATUS_ERROR_UNKNOWN;
		ret = -FailureReason_CannotDecodeMessage;
	}
	else {
		ret = ValidateIOStateRequest(request);
		if (ret < 0) {
			responseStatus = UAVCAN_KPLC_IOSTATE_STATUS_ERROR_INCONSISTENCY;
		}
		else {
			ApplyIOState(&request.state[0]);
		}
	}
	
	uint8_t buffer[UAVCAN_KPLC_IOSTATE_RESPONSE_MAX_SIZE];
	memset(buffer, 0, sizeof(buffer));
	uavcan_kplc_IOStateResponse response = {
		.status = responseStatus
	};
	uint16_t len = uavcan_kplc_IOStateResponse_encode(&response, &buffer[0]);
	int16_t ret_int16 = canardRequestOrRespond(&g_canard,
		transfer->source_node_id,
		UAVCAN_KPLC_IOSTATE_SIGNATURE,
		UAVCAN_KPLC_IOSTATE_ID,
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
				case UAVCAN_KPLC_IOSTATE_ID:
					handler = handle_KPLC_IOState_Request;
					break;
				case UAVCAN_PROTOCOL_PARAM_GETSET_ID:
					handler = handle_protocol_param_GetSet;
					break;
			}
			break;
		case CanardTransferTypeResponse:
			switch(transfer->data_type_id)
			{
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

ISR(INT2_vect)
{
	handleCanRxInterrupt();
}

void enableCanRxInterrupt()
{
	EIMSK |= _BV(INT2);
}

void disableCanRxInterrupt()
{
	EIMSK &= ~_BV(INT2);
}

void setLed()
{
	PORTB |= _BV(PORTB4);
}

void resetLed()
{
	PORTB &= ~_BV(PORTB4);
}

int main(void)
{
	wdt_enable(WDTO_250MS);
	wdt_reset();
	
	DDRA = _BV(PORTA4) | _BV(PORTA5) | _BV(PORTA6) | _BV(PORTA7);
	DDRB |= _BV(PORTB3) | _BV(PORTB4); // MCP2515 Reset
	DDRC = 0xFF;
	DDRD = 0xFF;
	
	PORTA = _BV(PORTA0) | _BV(PORTA1) | _BV(PORTA2) | _BV(PORTA3);
	PORTB |= _BV(PORTB0) | _BV(PORTB1) | _BV(PORTB3);
	
	int8_t result;
	
	result = setup();
	if (result < 0) {
		fail(result);
	}
	
	g_nodeStatusMode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
	
	// Set up MCP2515 interrupt.
	EICRA |= _BV(ISC21);	// Trigger INT2 on falling edge
	EIMSK |= _BV(INT2);		// Enable INT2
	
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
				
				g_timers.update();
				if (!checkNodeHealth()) {
					continue;
				}
				
				validateMasterNodeState();
				if (!checkNodeHealth()) {
					continue;
				}
				
				break;
			}
			case NodeState_Error:
			{
				// Set all outputs to the low state.
				PORTA &= ~(_BV(PORTA4) | _BV(PORTA5) | _BV(PORTA6) | _BV(PORTA7));
				PORTC = 0;
				PORTD = 0;
				
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

