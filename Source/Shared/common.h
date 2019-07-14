#pragma once

#include "main_config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "canard/canard_avr.h"
#include "canard/canard.h"
#include "Timer/Timer.h"

#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/Panic.h"

#define MAIN_MODULE_NODE_ID 100

enum NodeState
{
	NodeState_Initial = 0,
	NodeState_Operational = 1,
	NodeState_Error = 2,
};

enum ParamKind
{
	ParamKind_NodeState = 0,
	
	ParamKind_Max = 0,
};

const char ParamKind_NodeState_Name[] PROGMEM = "ModuleState";
const uint8_t ParamKind_Name_MaxLength = 16;

const char* const ParamKind_Names[] =
{
	ParamKind_NodeState_Name,
};

ParamKind parseParamKind(char* name, int len);

enum FailureReason
{
	FailureReason_NoFailure = 0,
	
	FailureReason_Other = 1,
	FailureReason_Watchdog = 2,
	FailureReason_CAN_BusError = 3,
	FailureReason_OutOfMemory = 4,
	FailureReason_NodeIdNotSet = 5,
	FailureReason_PanicReceived = 6,
	FailureReason_CannotDecodeMessage = 7,
	FailureReason_PLCError = 8,
	FailureReason_UnexpectedResponse = 9,
	FailureReason_CanardInternal = 10,
	FailureReason_BadNodeStatus = 11,
	FailureReason_CannotInit = 12,
	FailureReason_DriverCannotTransmit = 13,
	FailureReason_SlavesStateValidationError = 14,
	FailureReason_MasterStateValidationError = 15,
	FailureReason_RXBufferOverflow = 16,
	FailureReason_LowPower = 17,
	FailureReason_CAN_DataOverrun = 18,
	FailureReason_InvalidArgument = 19,
	FailureReason_MasterStatusTimeoutOverflow = 20,
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
void fail(int8_t reason, uint8_t* message = NULL, uint8_t message_length = 0);
bool checkNodeHealth();

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

void handleCanRxInterrupt();
void enableCanRxInterrupt();
void disableCanRxInterrupt();

uint8_t readNodeId();

int8_t validateMasterNodeStatus(uavcan_protocol_NodeStatus status);

int8_t validateTransceiverState();
void resetTransceiverState();