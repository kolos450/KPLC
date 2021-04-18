#include "inputs.h"

#define IOSTATE_LPF_TIME_OFFSET_UNSET 0xFF
#define IOSTATE_LPF_TIME_MS 20
constexpr uint8_t DigitalInputsCount = 64;
constexpr uint8_t DigitalInputsBytes = (DigitalInputsCount + 7) / 8;
constexpr uint8_t AnalogInputsCount = 2;
constexpr uint8_t InputsBufferLength = DigitalInputsBytes + AnalogInputsCount;

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

int8_t sendFrame(uint8_t frameIndex, uint8_t* data, uint8_t length) {	
	uint8_t buffer[KPLC_IOSTATEFRAME_REQUEST_MAX_SIZE];
	static uint8_t transfer_id = 0;
	
	kplc_IOStateFrameRequest request;
	request.frameIndex = frameIndex;
	request.data.data = data;
	request.data.len = length;

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

int8_t FetchDigitalInputs(uint8_t* buffer)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		PORTD = _BV(i);
		DDRD = _BV(i);
		_delay_ms(1);
		buffer[i] = PINC;
	}
	
	DDRD = 0;
	PORTD = 0;
	
	return 0;
}

int8_t DigitalLowPassFilter(uint8_t* data, uint8_t const * previousData)
{
	static uint8_t g_ioStateStaging[DigitalInputsBytes] = { 0 };
	static uint8_t g_ioStateLpfTimeOffsets[DigitalInputsCount] = { 0 };
	static uint32_t g_ioStateLpfTimeReference = 0;
	
	uint32_t now = millis();
	uint32_t timeDiff = now - g_ioStateLpfTimeReference;
	g_ioStateLpfTimeReference = now;
	uint8_t diffIsBig = timeDiff >= IOSTATE_LPF_TIME_MS;
	uint8_t smallDiff = (uint8_t)timeDiff;
	
	int8_t ret = 0;
	
	for (int i = 0; i < DigitalInputsBytes; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			uint8_t current_ij = data[i] & _BV(j);
			uint8_t previous_ij = previousData[i] & _BV(j);
			if (current_ij != previous_ij) {
				data[i] = (data[i] & ~_BV(j)) | previous_ij;
			}
			
			uint8_t digitalInputIndex = i * 8 + j;
			if ((g_ioStateStaging[i] & _BV(j)) != current_ij)
			{
				g_ioStateStaging[i] = (g_ioStateStaging[i] & ~_BV(j)) | current_ij;
				
				if (previous_ij == current_ij)
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
					data[i] = (g_ioStateStaging[i] & ~_BV(j)) | current_ij;
					ret = 1;
				}
				else
				{
					g_ioStateLpfTimeOffsets[digitalInputIndex] = smallDiff + timeOffset;
				}
			}
		}
	}
	
	return ret;
}

int8_t AnalogLowPassFilter(uint8_t* data, uint8_t const * previousData)
{
	static uint8_t g_ioStateStaging[AnalogInputsCount] = { 0 };
	static uint8_t g_ioStateLpfTimeOffsets[AnalogInputsCount] = { 0 };
	static uint32_t g_ioStateLpfTimeReference = 0;
	
	uint32_t now = millis();
	uint32_t timeDiff = now - g_ioStateLpfTimeReference;
	g_ioStateLpfTimeReference = now;
	uint8_t diffIsBig = timeDiff >= IOSTATE_LPF_TIME_MS;
	uint8_t smallDiff = (uint8_t)timeDiff;
	
	int8_t ret = 0;
	
	for (int i = 0; i < AnalogInputsCount; i++)
	{
		uint8_t current = data[i];
		uint8_t previous = previousData[i];
		if (current != previous) {
			data[i] = previous;
		}
		
		if ((g_ioStateStaging[i]) != current)
		{
			g_ioStateStaging[i] = current;
				
			if (previous == current)
			{
				g_ioStateLpfTimeOffsets[i] = IOSTATE_LPF_TIME_OFFSET_UNSET;
			}
			else if (diffIsBig ||
				smallDiff + g_ioStateLpfTimeOffsets[i] > IOSTATE_LPF_TIME_MS)
			{
				g_ioStateLpfTimeOffsets[i] = IOSTATE_LPF_TIME_OFFSET_UNSET;
				data[i] = current;
				ret = 1;
			}
		}
		else
		{
			uint8_t timeOffset = g_ioStateLpfTimeOffsets[i];
			if (timeOffset == IOSTATE_LPF_TIME_OFFSET_UNSET)
			{
				// Nothing to do.
			}
			else if (diffIsBig ||
				smallDiff + timeOffset > IOSTATE_LPF_TIME_MS)
			{
				g_ioStateLpfTimeOffsets[i] = IOSTATE_LPF_TIME_OFFSET_UNSET;
				data[i] = current;
				ret = 1;
			}
			else
			{
				g_ioStateLpfTimeOffsets[i] = smallDiff + timeOffset;
			}
		}
	}
	
	return ret;
}

int8_t SetupAnalogChannel(uint8_t channel) {
	auto admux = ADMUX & ~(_BV(MUX0) | _BV(MUX1) | _BV(MUX2) | _BV(MUX3) | _BV(MUX4));
	switch (channel)
	{
		case 0:
		{
			admux |= _BV(MUX0);
			break;
		}
		case 1:
		{
			break;
		}
		default:
		{
			return -1;
		}
	}
	
	ADMUX = admux;
	
	return 0;
}

int8_t NextAnalogChannel(uint8_t channel) {
	channel++;
	if(channel == AnalogInputsCount) {
		channel = 0;
	}
	return channel;
}

int8_t FetchAnalogInputs(uint8_t* buffer) {
	static uint8_t adcChannel = 0xFF;
	static uint8_t adcBuffer[AnalogInputsCount] = { 0 };
	
	if (adcChannel == 0xFF)
	{
		adcChannel = 0;
		SetupAnalogChannel(adcChannel);
		// Start the conversion.
		ADCSRA |= _BV(ADSC);
	}
	else
	{
		// ADSC is cleared when the conversion finishes.
		if (!(ADCSRA & _BV(ADSC)))
		{
			adcBuffer[adcChannel] = ADCH; // 8 bits, 0 to 255.
			
			// Setup the next channel.
			adcChannel = NextAnalogChannel(adcChannel);
			int8_t ret = SetupAnalogChannel(adcChannel);
			if (ret < 0) {
				return ret;
			}
			
			// Start the conversion.
			ADCSRA |= _BV(ADSC);
		}
	}
	
	memcpy(buffer, &adcBuffer, AnalogInputsCount);
	
	return 0;
}

int8_t SendData(uint8_t* current, uint8_t* previous, bool forced) {
	uint8_t frameIndex = 0;
	for (uint8_t i = 0; i < InputsBufferLength; i+= KPLC_IOSTATEFRAME_REQUEST_DATA_MAX_LENGTH, frameIndex++) {
		auto length = InputsBufferLength - i;
		if (length > KPLC_IOSTATEFRAME_REQUEST_DATA_MAX_LENGTH) {
			length = KPLC_IOSTATEFRAME_REQUEST_DATA_MAX_LENGTH;
		}
		
		if (forced || memcmp(current + i, previous + i, length)) {
			int8_t ret = sendFrame(frameIndex, current + i, length);
			if (ret < 0) {
				return ret;
			}
		}
	}
	
	return 0;
}

int8_t ProcessInputs(bool forced)
{
	static uint8_t data[InputsBufferLength];
	uint8_t current[InputsBufferLength];
	
	int8_t ret;
	
	ret = FetchDigitalInputs(current);
	if (ret < 0) return ret;
	
	ret = FetchAnalogInputs(current + DigitalInputsBytes);
	if (ret < 0) return ret;
	
	ret = DigitalLowPassFilter(current, data);
	if (ret < 0) return ret;
	
	bool hasChanges = ret == 1;
	
 	ret = AnalogLowPassFilter(current + DigitalInputsBytes, data + DigitalInputsBytes);
 	if (ret < 0) return ret;
	
	if (ret == 1) {
		hasChanges = true;
	}
	
	if (hasChanges || forced) {
		 ret = SendData(current, data, forced);
		 if (ret < 0) return ret;
		 
		 memcpy(data, current, InputsBufferLength);
	}
	
	return 0;
}

uint8_t InitializeInputs()
{
	DDRA &= ~(_BV(0) | _BV(1));
	ADMUX =
		_BV(ADLAR) | // ADC Left Adjust Result.
		_BV(REFS0); // AVCC with external capacitor at AREF pin.
	ADCSRA |=
		_BV(ADEN) | // Enable ADC.
		_BV(ADPS2) | _BV(ADPS1); // Division factor = 64.
	
	return 0;
}