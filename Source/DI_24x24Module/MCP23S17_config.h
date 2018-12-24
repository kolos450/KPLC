#include "main_config.h"
#include <avr/io.h>
#include "Arduino/SPI.h"
#include "MCP23S17/MCP23S17.h"

struct MCP23S17_A_tag
{
	static void InitCS()
	{
		DDRC |= _BV(PORTC2);
		PORTC |= _BV(PORTC2);
	}
	
	static void SetCS()
	{
		PORTC &= ~_BV(PORTC2);
	}
	
	static void ResetCS()
	{
		PORTC |= _BV(PORTC2);
	}
	
	static const SPIClass<SPI1_tag>* SPI;
};

struct MCP23S17_B_tag
{
	static void InitCS()
	{
		DDRC |= _BV(PORTC3);
		PORTC |= _BV(PORTC3);
	}
	
	static void SetCS()
	{
		PORTC &= ~_BV(PORTC3);
	}
	
	static void ResetCS()
	{
		PORTC |= _BV(PORTC3);
	}
	
	static const SPIClass<SPI1_tag>* SPI;
};

extern template class MCP23S17<MCP23S17_A_tag>;
extern template class MCP23S17<MCP23S17_B_tag>;

extern MCP23S17<MCP23S17_A_tag> MCP23S17_A;
extern MCP23S17<MCP23S17_B_tag> MCP23S17_B;