#include "main_config.h"
#include "canard/canard.h"
#include "Arduino/Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "plc/plc.h"
#include "common.h"

#include "uavcan/kplc/IOState.h"
#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/Panic.h"
#include "uavcan/protocol/param/GetSet.h"

CanardInstance g_canard;              // The canard library instance.
uint8_t g_canard_memory_pool[1024];   // Arena for memory allocation, used by the library.

Timer<4> g_timers;

static int8_t handle_KPLC_IOState_Request(CanardRxTransfer* transfer)
{
	uavcan_kplc_IOStateRequest request;
	int8_t ret;
	uint8_t responseStatus = UAVCAN_KPLC_IOSTATE_STATUS_OK;
	ret = uavcan_kplc_IOStateRequest_decode(transfer, transfer->payload_len, &request, NULL);
	if (ret < 0) {
		responseStatus = UAVCAN_KPLC_IOSTATE_STATUS_ERROR_UNKNOWN;
		ret = -FailureReason_CannotDecodeMessage;
	}
	else {
		ret = PlcPush(transfer->source_node_id, request);
		if (ret < 0) {
			responseStatus = UAVCAN_KPLC_IOSTATE_STATUS_ERROR_INCONSISTENCY;
			ret = -FailureReason_PLCError;
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
	int8_t ret;
	uavcan_protocol_NodeStatus status;
	ret = uavcan_protocol_NodeStatus_decode(transfer, transfer->payload_len, &status, NULL);
	if (ret < 0) {
		return -FailureReason_CannotDecodeMessage;
	}
	
	uint8_t sourceNodeId = transfer->source_node_id;	
	uint8_t nodeFound = false;
	for (uint8_t i = 0; i < PLC_SLAVE_NODES_LEN; i++) {
		if (g_plcSlaveNodeState[i].NodeId == sourceNodeId) {
			nodeFound = true;
			g_plcSlaveNodeState[i].LastStatusUpdateTime = millis();
		}
	}
	
	if (!nodeFound) {
		return 0;
	}
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
	int8_t ret;
	uavcan_protocol_param_GetSetRequest request;
	ret = uavcan_protocol_param_GetSetRequest_decode(transfer, transfer->payload_len, &request, NULL);
	if (ret < 0) {
		return -FailureReason_CannotDecodeMessage;
	}
	
	switch ((ParamKind)request.index) {
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

int8_t ValidateSlavesState()
{
	uint32_t now = millis();
	
	for (uint8_t i = 0; i < PLC_SLAVE_NODES_LEN; i++) {
		if ((now - g_plcSlaveNodeState[i].LastStatusUpdateTime) > 2 * CANARD_NODESTATUS_PERIOD_MSEC) {
			return -FailureReason_SlavesStateValidationError;
		}
	}
	
	return 0;
}

bool CheckSlavesReadiness()
{
	for (uint8_t i = 0; i < PLC_SLAVE_NODES_LEN; i++) {
		if (g_plcSlaveNodeState[i].LastStatusUpdateTime == 0) {
			return false;
		}
	}
	
	return true;
}

int8_t UpdateSlavesState(NodeState state)
{
	static uint8_t transfer_id = 0; // This variable MUST BE STATIC; refer to the libcanard documentation for the background.
	
	uint8_t buffer[50]; // TODO
	memset(buffer, 0, sizeof(buffer));
	uavcan_protocol_param_GetSetRequest request = {
		.index = ParamKind_NodeState,
	};
	request.value.boolean_value = (uint8_t)state;
	uint16_t len = uavcan_protocol_param_GetSetRequest_encode(&request, &buffer[0]);
	
	for (uint8_t i = 0; i < PLC_SLAVE_NODES_LEN; i++)
	{
		int16_t result = canardRequestOrRespond(&g_canard,
			g_plcSlaveNodeState[i].NodeId,
			UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
			UAVCAN_PROTOCOL_PARAM_GETSET_ID,
			&transfer_id,
			CANARD_TRANSFER_PRIORITY_MEDIUM,
			CanardRequest,
			&buffer[0],
			len);
		
		if (result < 0) {
			return (int8_t)result;
		}
	}
	
	return 0;
}

int8_t UpdateSlavesIOState()
{
	static uint8_t transfer_id = 0; // This variable MUST BE STATIC; refer to the libcanard documentation for the background.
	
	uint8_t len;
	PlcPullItem items[PLC_PULL_ITEMS_MAX];
	int8_t ret = PlcPull(&items[0], &len);
	if (ret < 0)
		return -FailureReason_PLCError;
	if (len == 0)
		return 0;
		
	for (int i = 0; i < len; i++)
	{
		uint8_t buffer[UAVCAN_KPLC_IOSTATE_REQUEST_MAX_SIZE];
		
		uint16_t len = uavcan_kplc_IOStateRequest_encode(&(items[i].State), &buffer[0]);
		
		int16_t result = canardRequestOrRespond(&g_canard,
			items[i].NodeId,
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
	DDRD |= _BV(PORTD5) | _BV(PORTD6) | _BV(PORTD7); // LEDs
	DDRC |= _BV(PORTC3) | _BV(PORTC4); // MCP2515s Reset
	
	PORTC |= _BV(PORTC3);
	
	int8_t result;
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
					continue;;
				}
				
				receiveCanard();
				g_timers.update();
				
				if (CheckSlavesReadiness())
				{
					result = UpdateSlavesState(NodeState_Operational);
					if (result < 0) {
						fail(result);
					}
					g_nodeState = NodeState_Operational;
				}
				
				break;
			}
			case NodeState_Operational:
			{
				result = sendCanard();
				if (result < 0) {
					fail(result);
					continue;
				}
				
				receiveCanard();
				g_timers.update();
				
				result = ValidateSlavesState();
				if (result < 0) {
					fail(result);
					continue;
				}
				
				result = UpdateSlavesIOState();
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

