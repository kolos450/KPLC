#include "main_config.h"
#include "canard/canard.h"
#include "Arduino/Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "common.h"

#include "kplc/IOStateFrame.h"
#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/Panic.h"
#include "uavcan/protocol/param/GetSet.h"

CanardInstance g_canard;              // The canard library instance.
uint8_t g_canard_memory_pool[CANARD_MEMORY_POOL_SIZE];   // Arena for memory allocation, used by the library.

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
	else if (request.frameIndex != 0 || request.data.len != 3) {
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
					resetTransceiverState();
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

