#include "main_config.h"
#include "canard/canard.h"
#include "Arduino/Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>
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

static uint16_t g_ioStateA = 0, g_ioStateB = 0;

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
	
	if (status.health != UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK) {
		return -FailureReason_BadNodeStatus;
	}
	if (status.mode != UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL) {
		return -FailureReason_BadNodeStatus;
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
	
	switch ((ParamKind)request.index) {
		case ParamKind_NodeState:
		{
			g_nodeState = (NodeState)request.value.boolean_value;
			
			uint8_t buffer[50]; // TODO
			memset(buffer, 0, sizeof(buffer));
			uavcan_protocol_param_GetSetResponse response;
			response.value.boolean_value = g_nodeState;
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
			
			break;
		}
		
		default:
		{
			return -FailureReason_InvalidArgument;
		}
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
	
	if ((now - g_mainModuleLastStatusUpdateTime) > 2 * CANARD_NODESTATUS_PERIOD_MSEC) {
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

int8_t ProcessIOState(bool forced)
{
	uint16_t a = MCP23S17_A.read();
	uint16_t b = MCP23S17_B.read();
	
	if (forced || a != g_ioStateA || b != g_ioStateB)
	{
		uint8_t buffer[UAVCAN_KPLC_IOSTATE_REQUEST_MAX_SIZE];
		static uint8_t transfer_id = 0;
		
		uavcan_kplc_IOStateRequest request;
		request.state[0] = (byte)a;
		request.state[1] = (byte)(a >> 8);
		request.state[2] = (byte)b;
		request.state_inv[0] = ~request.state[0];
		request.state_inv[1] = ~request.state[1];
		request.state_inv[2] = ~request.state[2];
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
	}
	
	return 0;
}

int main(void)
{
	DDRD |= _BV(PORTD6); // LED
	DDRB |= _BV(PORTB1); // MCP2515 Reset
	
	PORTB |= _BV(PORTB1);
	PORTC |= _BV(PORTC5);
	PORTC |= _BV(PORTD0) | _BV(PORTD1) | _BV(PORTD3) | _BV(PORTD4);
	PORTE |= _BV(PORTE0);
	
	int8_t result;
	
	result = setupMCP23S17();
	if (result < 0) {
		fail(result);
	}
	
	result = setup();
	if (result < 0) {
		fail(result);
	}
	
	g_nodeStatusMode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
	
	while (1)
	{
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
				g_timers.update();
				
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
				
				break;
			}
			case NodeState_Error:
			{
				cli();
				break;
			}
		}
	}
}

