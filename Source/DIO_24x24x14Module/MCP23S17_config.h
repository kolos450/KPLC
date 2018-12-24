#include "main_config.h"
#include <avr/io.h>
#include "Arduino/SPI.h"
#include "MCP23S17/MCP23S17.h"

struct MCP23S17_tag
{
	static void InitCS()
	{
		DDRE |= _BV(PORTE2);
		PORTE |= _BV(PORTE2);
	}
	
	static void SetCS()
	{
		PORTE &= ~_BV(PORTE2);
	}
	
	static void ResetCS()
	{
		PORTE |= _BV(PORTE2);
	}
	
	static const SPIClass<SPI1_tag>* SPI;
};

extern template class MCP23S17<MCP23S17_tag>;

extern MCP23S17<MCP23S17_tag> MCP23S17_A;
extern MCP23S17<MCP23S17_tag> MCP23S17_B;