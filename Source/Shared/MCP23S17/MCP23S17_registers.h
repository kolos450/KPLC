#pragma once

#include <stdint.h>
//------------------------- REGISTERS

const static byte			MCP23S17_IODIR = 	0x00; // I/O DIRECTION REGISTER
const static byte			MCP23S17_IPOL = 	0x02; // INPUT POLARITY REGISTER
const static byte			MCP23S17_GPINTEN =  0x04; // INTERRUPT-ON-CHANGE CONTROL REGISTER
const static byte			MCP23S17_DEFVAL = 	0x06; // DEFAULT COMPARE REGISTER FOR INTERRUPT-ON-CHANGE
const static byte			MCP23S17_INTCON = 	0x08; // INTERRUPT CONTROL REGISTER
const static byte			MCP23S17_IOCON = 	0x0A; // CONFIGURATION REGISTER
const static byte			MCP23S17_GPPU = 	0x0C; // GPIO PULL-UP RESISTOR REGISTER
const static byte			MCP23S17_INTF = 	0x0E; // INTERRUPT FLAG REGISTER
const static byte			MCP23S17_INTCAP = 	0x10; // INTERRUPT CAPTURED VALUE FOR PORT REGISTER
const static byte			MCP23S17_GPIO = 	0x12; // GENERAL PURPOSE I/O PORT REGISTER
const static byte			MCP23S17_OLAT = 	0x14; // OUTPUT LATCH REGISTER

#define MCP23S17_IOCON_BANK		7
#define MCP23S17_IOCON_MIRROR	6
#define MCP23S17_IOCON_SEQOP	5
#define MCP23S17_IOCON_DISSLW	4
#define MCP23S17_IOCON_HAEN		3
#define MCP23S17_IOCON_ORD		2
#define MCP23S17_IOCON_INTPOL	1