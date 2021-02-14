#include "main_config.h"
#include "canard/canard.h"
#include "Arduino/Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "common.h"

#include "kplc/IOState.h"
#include "kplc/IOState_float16x2.h"
#include "kplc/IOState_uint8x7.h"
#include "uavcan/protocol/GetNodeInfo.h"
#include "uavcan/protocol/NodeStatus.h"
#include "uavcan/protocol/Panic.h"
#include "uavcan/protocol/param/GetSet.h"

#define MAIN_MODULE_NODE_ID 100
#define IOSTATE_MIN_TRANSMIT_INTERVAL_MSEC 500

CanardInstance g_canard;              // The canard library instance.
uint8_t g_canard_memory_pool[1024];   // Arena for memory allocation, used by the library.

#define IOSTATE_LPF_TIME_OFFSET_UNSET 0xFF
#define IOSTATE_LPF_TIME_MS 20
constexpr int InputsCount = 56;
constexpr int InputsBufferLength = InputsCount / 8;
static uint8_t g_ioState[InputsBufferLength];
static uint8_t g_ioStateStaging[InputsBufferLength];
static uint8_t g_ioStateLpfTimeOffsets[InputsCount];
static uint32_t g_ioStateLpfTimeReference = 0;


Timer<4> g_timers;

int8_t ProcessIOState(bool forced = false);

uint8_t readNodeId()
{
	// Charlieplexing.
	// ID0 = PA2
	// ID1 = PA3
	// ID2 = PA4
	
	int result = 0;
	
	// Initialization.
	DDRA &= ~(_BV(PINA2) | _BV(PINA3) | _BV(PINA4));
	PORTA &= ~(_BV(PINA2) | _BV(PINA3) | _BV(PINA4));
	
	// Pull-up A3, A4.
	PORTA |= _BV(PINA3) | _BV(PINA4);
	// Make A2 low.	
	DDRA |= _BV(PINA2);
	// Read bit 1, bit 5.
	if (!(PINA & _BV(PINA3))) result |= _BV(0);
	if (!(PINA & _BV(PINA4))) result |= _BV(4);
	
	// Reset.
	DDRA &= ~(_BV(PINA2) | _BV(PINA3) | _BV(PINA4));
	PORTA &= ~(_BV(PINA2) | _BV(PINA3) | _BV(PINA4));
	
	// Pull-up A2, A4.
	PORTA |= _BV(PINA2) | _BV(PINA4);
	// Make A3 low.
	DDRA |= _BV(PINA3);
	// Read bit 3, bit 2.
	if (!(PINA & _BV(PINA2))) result |= _BV(2);
	if (!(PINA & _BV(PINA4))) result |= _BV(1);
	
	// Reset.
	DDRA &= ~(_BV(PINA2) | _BV(PINA3) | _BV(PINA4));
	PORTA &= ~(_BV(PINA2) | _BV(PINA3) | _BV(PINA4));
	
	// Pull-up A2, A3.
	PORTA |= _BV(PINA2) | _BV(PINA3);
	// Make A4 low.
	DDRA |= _BV(PINA4);
	// Read bit 6, bit 4.
	if (!(PINA & _BV(PINA2))) result |= _BV(5);
	if (!(PINA & _BV(PINA3))) result |= _BV(3);
	
	// Reset.
	DDRA &= ~(_BV(PINA2) | _BV(PINA3) | _BV(PINA4));
	PORTA &= ~(_BV(PINA2) | _BV(PINA3) | _BV(PINA4));
	
	return result;
}

static int8_t ValidateIOStateRequest(kplc_IOStateRequest request)
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

static void ApplyIOState(uint8_t* state)
{
	auto porta = PORTA & ~(_BV(PORTA5) | _BV(PORTA6) | _BV(PORTA7));
	if (state[0] & _BV(0)) porta |= _BV(PORTA7); // McOut0
	if (state[0] & _BV(1)) porta |= _BV(PORTA6); // McOut1
	if (state[0] & _BV(2)) porta |= _BV(PORTA5); // McOut2
	PORTA = porta;
	
	auto portb = PORTB & ~(_BV(PORTA0) | _BV(PORTA1));
	if (state[0] & _BV(3)) portb |= _BV(PORTA1); // McOut3
	if (state[0] & _BV(4)) portb |= _BV(PORTA0); // McOut4
	PORTB = portb;
}

static int8_t handle_KPLC_IOState_Request(CanardRxTransfer* transfer)
{
	if (transfer->source_node_id != MAIN_MODULE_NODE_ID) {
		return 0;
	}
	
	kplc_IOStateRequest request;
	int8_t ret;
	uint8_t responseStatus = KPLC_IOSTATE_RESPONSE_STATUS_OK;
	ret = kplc_IOStateRequest_decode(transfer, transfer->payload_len, &request, NULL);
	if (ret < 0) {
		responseStatus = KPLC_IOSTATE_RESPONSE_STATUS_ERROR_UNKNOWN;
		ret = -FailureReason_CannotDecodeMessage;
	}
	else {
		ret = ValidateIOStateRequest(request);
		if (ret < 0) {
			responseStatus = KPLC_IOSTATE_RESPONSE_STATUS_ERROR_INCONSISTENCY;
		}
		else {
			ApplyIOState(&request.state[0]);
		}
	}
	
	uint8_t buffer[KPLC_IOSTATE_RESPONSE_MAX_SIZE];
	memset(buffer, 0, sizeof(buffer));
	kplc_IOStateResponse response = {
		.status = responseStatus
	};
	uint16_t len = kplc_IOStateResponse_encode(&response, &buffer[0]);
	int16_t ret_int16 = canardRequestOrRespond(&g_canard,
		transfer->source_node_id,
		KPLC_IOSTATE_SIGNATURE,
		KPLC_IOSTATE_ID,
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
	kplc_IOStateResponse response;
	int8_t ret;
	ret = kplc_IOStateResponse_decode(transfer, transfer->payload_len, &response, NULL);
	if (ret < 0) {
		return -FailureReason_CannotDecodeMessage;
	}
	
	if (response.status != 0) {
		return -FailureReason_UnexpectedResponse;
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
		case KPLC_IOSTATE_ID:
			*out_data_type_signature = KPLC_IOSTATE_SIGNATURE;
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
				case KPLC_IOSTATE_ID:
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
				case KPLC_IOSTATE_ID:
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

uint8_t sendIOState()
{
	uint8_t buffer[KPLC_IOSTATE_UINT8X7_REQUEST_MAX_SIZE];
	static uint8_t transfer_id = 0;
	
	kplc_IOState_uint8x7Request request;
	memcpy(request.state, g_ioState, InputsBufferLength);

	uint16_t len = kplc_IOState_uint8x7Request_encode(&request, &buffer[0]);
	
	int16_t result = canardRequestOrRespond(&g_canard,
		MAIN_MODULE_NODE_ID,
		KPLC_IOSTATE_UINT8X7_SIGNATURE,
		KPLC_IOSTATE_UINT8X7_ID,
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

void FetchInputs(uint8_t* buffer)
{
	for (int i = 0; i < 7; i++)
	{
		PORTD = _BV(i);
		DDRD = _BV(i);
		buffer[i] = PINC;
	}
	
	DDRD = 0;
	PORTD = 0;
}

int8_t ProcessDigitalInputs(bool forced)
{
	uint8_t current[InputsBufferLength];
	FetchInputs(&current[0]);
	
	uint8_t result[InputsBufferLength];
	memcpy(&result[0], &g_ioState[0], sizeof(g_ioState));
	
	uint32_t now = millis();
	uint32_t timeDiff = now - g_ioStateLpfTimeReference;
	g_ioStateLpfTimeReference = now;
	uint8_t diffIsBig = timeDiff >= IOSTATE_LPF_TIME_MS;
	uint8_t smallDiff = (uint8_t)timeDiff;
	
	uint8_t resultUpdated = false;
	
	for (int i = 0; i < InputsBufferLength; i++)
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

constexpr uint8_t AdcCount = 2;
static uint8_t g_adcValues[AdcCount];

uint8_t LowPassFilterAnalogInput(uint8_t channel, uint8_t value)
{
	if (g_adcValues[channel] != value)
	{
		g_adcValues[channel] = value;
		return true;
	}
	
	return false;
}

uint8_t SendAnalogInputs()
{
	// TODO
	return -1;
}

uint8_t ProcessAnalogInputs(bool forced) 
{
	static uint8_t adcChannel = 0xFF;
	
	if (adcChannel == 0xFF)
	{
		adcChannel = 0;
		// Start the conversion.
		ADCSRA |= _BV(ADSC);
	}
	else
	{
		// ADSC is cleared when the conversion finishes.
		if (ADCSRA & _BV(ADSC))
		{
			uint8_t adcValue = ADCH; // 8 bits, 0 to 255.
	
			if (LowPassFilterAnalogInput(adcChannel, adcValue))
			{
				auto result = SendAnalogInputs();
				if (result < 0) {
					return result;
				}
			}
	
			// Setup the next channel.
			adcChannel = 1 - adcChannel;
			auto admux = ADMUX & ~(_BV(MUX0) | _BV(MUX1) | _BV(MUX2) | _BV(MUX3) | _BV(MUX4));
			switch (adcChannel)
			{
				case 0:
					break;
				case 1:
					admux |= _BV(MUX0);
					break;
			}
			ADMUX = admux;
	
			// Start the conversion.
			ADCSRA |= _BV(ADSC);
		}
	}
	
	return 0;
}

uint8_t InitializeAnalogInputs()
{
	memset(g_adcValues, 0, AdcCount);
	
	ADMUX |= 
		_BV(ADLAR); // ADC Left Adjust Result.
	ADCSRA |= 
		_BV(ADEN) | // Enable ADC.
		_BV(ADPS1); // Division factor = 4.
		
	return 0;
}

int8_t ProcessIOState(bool forced)
{
	uint8_t ret;
	
	ret = ProcessDigitalInputs(forced);
	if (ret < 0) return ret;
	
	ret = ProcessAnalogInputs(forced);
	
	return ret;
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
	
	DDRA = _BV(PORTA5) | _BV(PORTA6) | _BV(PORTA7); // McOut2..0
	DDRB = 
		_BV(PORTB0) /* McOut4 */ | _BV(PORTB1) /* McOut3 */ |
		_BV(PORTB3) /* MCP2515 Reset */ | _BV(PORTB4) /* MCP2515 CS, Led */ |
		_BV(PORTB5) /* SPI MOSI */ | _BV(PORTB7) /* SPI SCK */;
	
	PORTB |= _BV(PORTB3); // MCP2515 Reset
	
	memset(&g_ioStateLpfTimeOffsets, IOSTATE_LPF_TIME_OFFSET_UNSET, sizeof(g_ioStateLpfTimeOffsets));
	memset(&g_ioState, 0, sizeof(g_ioState));
	memset(&g_ioStateStaging, 0, sizeof(g_ioStateStaging));
	
	int8_t result;
	
	result = InitializeAnalogInputs();
	if (result < 0) {
		fail(result);
	}
	
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
				// Set McOut0..5 low.
				PORTA &= ~(_BV(PORTA5) | _BV(PORTA6) | _BV(PORTA7));
				PORTB &= ~(_BV(PORTB0) | _BV(PORTB1));

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

