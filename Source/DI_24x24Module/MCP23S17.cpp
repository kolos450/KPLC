#include "MCP23S17_config.h"

template class MCP23S17<MCP23S17_A_tag>;
template class MCP23S17<MCP23S17_B_tag>;

const SPIClass<SPI1_tag>* MCP23S17_A_tag::SPI = &SPI1;
MCP23S17<MCP23S17_A_tag> MCP23S17_A(0);

const SPIClass<SPI1_tag>* MCP23S17_B_tag::SPI = &SPI1;
MCP23S17<MCP23S17_B_tag> MCP23S17_B(0);