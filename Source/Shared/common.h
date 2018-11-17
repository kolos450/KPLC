#pragma once

#include "main_config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "canard/canard_avr.h"
#include "canard/canard.h"
#include "Timer/Timer.h"

#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/Panic.h"

enum NodeState
{
	NodeState_Initial = 0,
	NodeState_Error = 1,
	NodeState_Operational = 2,
};

enum ParamKind
{
	ParamKind_NodeState = 0,
};

enum FailureReason
{
	FailureReason_NoFailure = 0,
	
	FailureReason_Other = 1,
	FailureReason_InvalidArgument = 2,
	FailureReason_OutOfMemory = 3,
	FailureReason_NodeIdNotSet = 4,
	FailureReason_PanicReceived = 5,
	FailureReason_CannotDecodeMessage = 6,
	FailureReason_PLCError = 7,
	FailureReason_UnexpectedResponse = 8,
	FailureReason_CanardInternal = 9,
	FailureReason_BadNodeStatus = 10,
	FailureReason_CannotInit = 11,
	FailureReason_DriverCannotTransmit = 12,
	FailureReason_SlavesStateValidationError = 13,
	FailureReason_MasterStateValidationError = 14,
	FailureReason_Watchdog = 15,
};

#define GET_MICROS (uint64_t)millis() * 1000ULL

extern uint8_t g_nodeStatusHealth;
extern uint8_t g_nodeStatusMode;
extern NodeState g_nodeState;
extern FailureReason g_failureReason;

extern CanardInstance g_canard;              // The canard library instance.
extern uint8_t g_canard_memory_pool[1024];   // Arena for memory allocation, used by the library.

extern Timer<4> g_timers;

typedef int8_t (*uavcanMessageHandler_t)(CanardRxTransfer* transfer);

int8_t setup(void);
void fail(int8_t reason);

int8_t sendCanard(void);
void receiveCanard(void);

bool shouldAcceptTransfer(
	const CanardInstance* ins,
	uint64_t* out_data_type_signature,
	uint16_t data_type_id,
	CanardTransferType transfer_type,
	uint8_t source_node_id);
void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer);

int8_t handle_protocol_GetNodeInfo(CanardRxTransfer* transfer);