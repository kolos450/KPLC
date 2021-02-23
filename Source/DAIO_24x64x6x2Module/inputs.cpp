#include "inputs.h"

#define IOSTATE_LPF_TIME_OFFSET_UNSET 0xFF
#define IOSTATE_LPF_TIME_MS 20
constexpr int InputsCount = 64;
constexpr int InputsBufferLength = InputsCount / 8;
static uint8_t g_ioState[InputsBufferLength];
static uint8_t g_ioStateStaging[InputsBufferLength];
static uint8_t g_ioStateLpfTimeOffsets[InputsCount];
static uint32_t g_ioStateLpfTimeReference = 0;

enum IOStateFrameIndex : uint8_t {
	IOStateFrameIndex_None = 0,
	IOStateFrameIndex_0 = 1 << 0,
	IOStateFrameIndex_1 = 1 << 1,
	IOStateFrameIndex_Max = IOStateFrameIndex_1,
	IOStateFrameIndex_All = IOStateFrameIndex_0 | IOStateFrameIndex_1,
};

IOStateFrameIndex getFrameIndex(uint8_t digitalInputIndex) {
	return (IOStateFrameIndex)(1 << (digitalInputIndex / 8 / KPLC_IOSTATEFRAME_REQUEST_DATA_MAX_LENGTH));
}

int8_t handle_KPLC_IOStateFrame_Response(CanardRxTransfer* transfer)
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

uint8_t sendDigitalInputsState(IOStateFrameIndex frameIndex) {
	uint8_t frameIndexId = 0;
	while(true) {
		auto frameIndexCandidate = (IOStateFrameIndex)(1 << frameIndexId);
		if (frameIndex == frameIndexCandidate) {
			break;
		}
		frameIndexId++;
		if(frameIndexCandidate == IOStateFrameIndex_Max) {
			return -FailureReason_Other;
		}
	}
	
	uint8_t bufferOffset = frameIndexId * KPLC_IOSTATEFRAME_REQUEST_DATA_MAX_LENGTH;
	uint8_t bufferLength = KPLC_IOSTATEFRAME_REQUEST_DATA_MAX_LENGTH;
	if (bufferOffset + bufferLength >= InputsBufferLength) {
		bufferLength = InputsBufferLength - bufferOffset;
	}
	
	uint8_t buffer[KPLC_IOSTATEFRAME_REQUEST_MAX_SIZE];
	static uint8_t transfer_id = 0;
	
	kplc_IOStateFrameRequest request;
	request.frameIndex = frameIndexId;
	request.data.data = g_ioState + bufferOffset;
	request.data.len = bufferLength;

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
	
	IOStateFrameIndex resultUpdated = IOStateFrameIndex_None;
	
	for (int i = 0; i < InputsBufferLength; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			uint8_t current_ij = current[i] & _BV(j);
			uint8_t digitalInputIndex = i * 8 + j;
			if ((g_ioStateStaging[i] & _BV(j)) != current_ij)
			{
				g_ioStateStaging[i] = (g_ioStateStaging[i] & ~_BV(j)) | current_ij;
				
				if ((result[i] & _BV(j)) == current_ij)
				{
					g_ioStateLpfTimeOffsets[digitalInputIndex] = IOSTATE_LPF_TIME_OFFSET_UNSET;
				}
				else
				{
					g_ioStateLpfTimeOffsets[digitalInputIndex] = 0;
				}
			}
			else
			{
				uint8_t timeOffset = g_ioStateLpfTimeOffsets[digitalInputIndex];
				if (timeOffset == IOSTATE_LPF_TIME_OFFSET_UNSET)
				{
					// Nothing to do.
				}
				else if (diffIsBig ||
				smallDiff + timeOffset > IOSTATE_LPF_TIME_MS)
				{
					g_ioStateLpfTimeOffsets[digitalInputIndex] = IOSTATE_LPF_TIME_OFFSET_UNSET;
					result[i] = (g_ioStateStaging[i] & ~_BV(j)) | current_ij;
					resultUpdated = (IOStateFrameIndex)(resultUpdated | getFrameIndex(digitalInputIndex));
				}
				else
				{
					g_ioStateLpfTimeOffsets[digitalInputIndex] = smallDiff + timeOffset;
				}
			}
		}
	}
	
	if (forced) {
		resultUpdated = IOStateFrameIndex_All;
	}
	
	if (resultUpdated) {
		memcpy(&g_ioState[0], &result[0], sizeof(g_ioState));
		
		for (IOStateFrameIndex i = IOStateFrameIndex_0; i <= IOStateFrameIndex_Max;	i = (IOStateFrameIndex)(i << 1)) {
			if (resultUpdated & i) {
				int8_t ret = sendDigitalInputsState(i);
				if (ret < 0) {
					return ret;
				}
			}
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

int8_t ProcessInputs(bool forced)
{
	uint8_t ret;
	
	ret = ProcessDigitalInputs(forced);
	if (ret < 0) return ret;
	
	ret = ProcessAnalogInputs(forced);
	
	return ret;
}

uint8_t InitializeInputs()
{
	memset(&g_ioStateLpfTimeOffsets, IOSTATE_LPF_TIME_OFFSET_UNSET, sizeof(g_ioStateLpfTimeOffsets));
	memset(&g_ioState, 0, sizeof(g_ioState));
	memset(&g_ioStateStaging, 0, sizeof(g_ioStateStaging));
	
	memset(&g_adcValues, 0, AdcCount);
	
	ADMUX |=
	_BV(ADLAR); // ADC Left Adjust Result.
	ADCSRA |=
	_BV(ADEN) | // Enable ADC.
	_BV(ADPS1); // Division factor = 4.
	
	return 0;
}