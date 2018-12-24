/*
 * MCP23S17 - A library for Microchip MCP23S17.
 *
 * -------------------------------------------------------------------------
 * 		Copyright (c) 2013-2014, s.u.m.o.t.o.y [sumotoy(at)gmail.com]
 * -------------------------------------------------------------------------
 *
 * MCP23S17 Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCP23S17 Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

 /*
  * MCP23S17 Wiring.
  * Basic Address:  00100 A2 A1 A0 (from 0x20 to 0x27)
  * A2, A1, A0 tied to ground = 0x20
  */

#pragma once

#include <inttypes.h>
#include "Arduino/SPI.h"
#include "MCP23S17_registers.h"

const static uint32_t _MCPMaxSpeed = 8000000UL;

#define REG_READ 1
#define REG_WRITE 0
#define OPCODE 0x40

template <class MCP23S17Tag>
class MCP23S17 {

public:
	MCP23S17();
    /// @param haenAddress Chip address from 0x20 to 0x27.
    MCP23S17(const uint8_t haenAddress);

    /// @param protocolInitOverride SPI will not be initialized when true.
    int8_t initialize(bool protocolInitOverride = false);

    /// @param direction OUTPUT => all out, INPUT => all in, 0xxxx => custom setup.
    void writeDataDirection(uint16_t direction);

    /// @param value HIGH => all Hi, LOW => all Low, 0xxxx => custom setup.
    void write(uint16_t value);

    /// Read the pins state.
    uint16_t read();

    /// Write a word in a chip register.
    void writeRegisterWord(byte reg, word data);

    /// @param data HIGH => all pullup, LOW => all pulldown, 0xxxx => custom setup.
    void writePullup(uint16_t data);


protected:
	static SPISettings const spiSettings_;

    void _spiBegin(uint8_t mode)
    {		
        MCP23S17Tag::SPI->beginTransaction(spiSettings_);
        MCP23S17Tag::SetCS();
        mode == 1 ? MCP23S17Tag::SPI->transfer(_readCmd) : MCP23S17Tag::SPI->transfer(_writeCmd);
    }


    void _spiEnd(void)
    {
        MCP23S17Tag::ResetCS();

        MCP23S17Tag::SPI->endTransaction();
    }

    void _spiWriteByte(byte addr, uint8_t data)
    {
        _spiBegin(0);
        MCP23S17Tag::SPI->transfer(addr);
        MCP23S17Tag::SPI->transfer(data);
        _spiEnd();
    }

    void _spiWriteWord(byte addr, uint16_t data)
    {
        _spiBegin(0);
        MCP23S17Tag::SPI->transfer(addr);
        MCP23S17Tag::SPI->transfer16(data);
        _spiEnd();
    }
	
	uint16_t _spiReadWord(uint8_t reg)
	{
		uint16_t data = 0;
		_spiBegin(1);
		MCP23S17Tag::SPI->transfer(reg);
		data = MCP23S17Tag::SPI->transfer16(0);
		_spiEnd();
		return data;
	}
	
	uint8_t _spiReadByte(byte reg)
	{
		uint8_t data = 0;
		_spiBegin(1);
		MCP23S17Tag::SPI->transfer(reg);
		data = MCP23S17Tag::SPI->transfer(0);
		_spiEnd();
		return data;
	}

private:
    uint8_t  _useHaen;
    uint8_t  _readCmd;
    uint8_t  _writeCmd;
};

template <class MCP23S17Tag>
MCP23S17<MCP23S17Tag>::MCP23S17(const uint8_t haenAdrs)
{
    _useHaen = true;
	
    _readCmd = OPCODE | (haenAdrs << 1) | REG_READ;
    _writeCmd = OPCODE | (haenAdrs << 1) | REG_WRITE;
}

template <class MCP23S17Tag>
MCP23S17<MCP23S17Tag>::MCP23S17()
{
	_useHaen = false;
	
	_readCmd = OPCODE | REG_READ;
	_writeCmd = OPCODE | REG_WRITE;
}

template <class MCP23S17Tag>
int8_t MCP23S17<MCP23S17Tag>::initialize(bool protocolInitOverride)
{
    if (!protocolInitOverride)
    {
        MCP23S17Tag::SPI->begin();
    }
	
    MCP23S17Tag::InitCS();
	
	uint8_t ioconValue = _BV(MCP23S17_IOCON_INTPOL);
	if (_useHaen)
	{
		ioconValue |= _BV(MCP23S17_IOCON_HAEN);
	}
	
    _spiWriteByte(MCP23S17_IOCON, ioconValue);
	
	if (ioconValue != _spiReadByte(MCP23S17_IOCON))
	{
		return -1;
	}

	return 0;
}


template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::writeDataDirection(uint16_t mode)
{
    _spiWriteWord(MCP23S17_IODIR, mode);
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::write(uint16_t value)
{
    _spiWriteWord(MCP23S17_GPIO, value);
}

template <class MCP23S17Tag>
uint16_t MCP23S17<MCP23S17Tag>::read()
{
    return _spiReadWord(MCP23S17_GPIO);
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::writePullup(uint16_t data)
{
    _spiWriteWord(MCP23S17_GPPU, data);
}

template <class MCP23S17Tag>
SPISettings const MCP23S17<MCP23S17Tag>::spiSettings_ = SPISettings(_MCPMaxSpeed, MSBFIRST, SPI_MODE0);