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


/*
 * The IOCON register:
 * 7    | 6      | 5     | 4      | 3    | 2   | 1      | 0
 * BANK | MIRROR | SEQOP | DISSLW | HAEN | ODR | INTPOL | -NC-
 * -----------------------------------------------------------------------
 * 0b01101100
 * BANK: (Controls how the registers are addressed)
 *   1 The registers associated with each port are separated into different banks
 *   0 The registers are in the same bank (addresses are sequential)
 * MIRROR: (INT Pins Mirror bit)
 *   1 The INT pins are internally connected
 *   0 The INT pins are not connected. INTA is associated with PortA and INTB is associated with PortB
 * SEQOP: (Sequential Operation mode bit)
 *   1 Sequential operation disabled, address pointer does not increment
 *   0 Sequential operation enabled, address pointer increments.
 * DISSLW: (Slew Rate control bit for SDA output, only I2C)
 * HAEN: (Hardware Address Enable bit, SPI only)
 *   1 Enables the MCP23S17 address pins
 *   0 Disables the MCP23S17 address pins
 * ODR: (This bit configures the INT pin as an open-drain output)
 *   1 Open-drain output (overrides the INTPOL bit).
 *   0 Active driver output (INTPOL bit sets the polarity).
 * INTPOL: (This bit sets the polarity of the INT output pin)
 *   1 Active high
 *   0 Active low
 */

template <class MCP23S17Tag>
class MCP23S17 {

public:
    /// @param haenAddress Chip address from 0x20 to 0x27.
    MCP23S17(const uint8_t haenAddress);

    /// @param protocolInitOverride SPI will not be initialized when true.
    void initialize(bool protocolInitOverride = false);

    /// @param direction OUTPUT => all out, INPUT => all in, 0xxxx => custom setup.
    void writeDataDirection(uint16_t direction);

    /// @param value HIGH => all Hi, LOW => all Low, 0xxxx => custom setup.
    void write(uint16_t value);

    /// Read the pins state.
    uint16_t read();

    /// Read a byte from chip register.
    uint8_t readRegisterByte(byte reg);

    /// Read a word from chip register.
    uint16_t readRegisterWord(byte reg);

    /// Write a byte in a chip register, optional for both ports.
    /// @param both Write the same register in bank A & B when true.
    void writeRegisterByte(byte reg, byte data, bool both = false);

    /// Write a word in a chip register.
    void writeRegisterWord(byte reg, word data);

    /// @param data HIGH => all pullup, LOW => all pulldown, 0xxxx => custom setup.
    void writePullup(uint16_t data);

    /// Direct access command.
    uint16_t readByAddress(byte addr);



protected:

    inline __attribute__((always_inline))
    void _GPIOstartSend(uint8_t mode)
    {
        MCP23S17Tag::SPI->beginTransaction(SPISettings(_MCPMaxSpeed, MSBFIRST, SPI_MODE0));
        MCP23S17Tag::SetCS();
        mode == 1 ? MCP23S17Tag::SPI->transfer(_readCmd) : MCP23S17Tag::SPI->transfer(_writeCmd);
    }


    inline __attribute__((always_inline))
    void _GPIOendSend(void)
    {
        MCP23S17Tag::ResetCS();

        MCP23S17Tag::SPI->endTransaction();
    }

    inline __attribute__((always_inline))
    void _GPIOwriteByte(byte addr, byte data)
    {
        _GPIOstartSend(0);
        MCP23S17Tag::SPI->transfer(addr);
        MCP23S17Tag::SPI->transfer(data);
        _GPIOendSend();
    }

    inline __attribute__((always_inline))
    void _GPIOwriteWord(byte addr, uint16_t data)
    {
        _GPIOstartSend(0);
        MCP23S17Tag::SPI->transfer(addr);
        MCP23S17Tag::SPI->transfer16(data);
        _GPIOendSend();
    }

private:
    uint8_t  _adrs;
    uint8_t  _useHaen;
    uint8_t  _readCmd;
    uint8_t  _writeCmd;
    uint16_t _gpioDirection;
    uint16_t _gpioState;
};

template <class MCP23S17Tag>
MCP23S17<MCP23S17Tag>::MCP23S17(const uint8_t haenAdrs)
{
    if (haenAdrs > 0x19 && haenAdrs < 0x28)
    {
        _adrs = haenAdrs;
        _useHaen = 1;
    }
    else
    {
        _adrs = 0;
        _useHaen = 0;
    }
    _readCmd = OPCODE | (_adrs << 1) | REG_READ;
    _writeCmd = OPCODE | (_adrs << 1) | REG_WRITE;
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::initialize(bool protocolInitOverride)
{
    if (!protocolInitOverride)
    {
        MCP23S17Tag::SPI->begin();
    }
	
    MCP23S17Tag::InitCS();
	
    _useHaen == 1 ? _GPIOwriteByte(MCP23S17_IOCON, 0b00101000) : _GPIOwriteByte(MCP23S17_IOCON, 0b00100000);
    _gpioDirection = 0xFFFF;//all in
    _gpioState = 0xFFFF;//all low 
}


template <class MCP23S17Tag>
uint16_t MCP23S17<MCP23S17Tag>::readByAddress(byte addr)
{
    _GPIOstartSend(1);
    MCP23S17Tag::SPI->transfer(addr);
    uint16_t temp = MCP23S17Tag::SPI->transfer16(0x0);
    _GPIOendSend();
    return temp;
}


template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::writeDataDirection(uint16_t mode)
{
    if (mode == INPUT)
    {
        _gpioDirection = 0xFFFF;
    }
    else if (mode == OUTPUT)
    {
        _gpioDirection = 0x0000;
        _gpioState = 0x0000;
    }
    else
    {
        _gpioDirection = mode;
    }
    _GPIOwriteWord(MCP23S17_IODIR, _gpioDirection);
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::write(uint16_t value)
{
    if (value == HIGH)
    {
        _gpioState = 0xFFFF;
    }
    else if (value == LOW)
    {
        _gpioState = 0x0000;
    }
    else
    {
        _gpioState = value;
    }
    _GPIOwriteWord(MCP23S17_GPIO, _gpioState);
}

template <class MCP23S17Tag>
uint16_t MCP23S17<MCP23S17Tag>::read()
{
    return readByAddress(MCP23S17_GPIO);
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::writePullup(uint16_t data)
{
    if (data == HIGH)
    {
        _gpioState = 0xFFFF;
    }
    else if (data == LOW)
    {
        _gpioState = 0x0000;
    }
    else
    {
        _gpioState = data;
    }
    _GPIOwriteWord(MCP23S17_GPPU, _gpioState);
}

template <class MCP23S17Tag>
uint8_t MCP23S17<MCP23S17Tag>::readRegisterByte(byte reg)
{
    uint8_t data = 0;
    _GPIOstartSend(1);
    MCP23S17Tag::SPI->transfer(reg);
    data = MCP23S17Tag::SPI->transfer(0);
    _GPIOendSend();
    return data;
}

template <class MCP23S17Tag>
uint16_t MCP23S17<MCP23S17Tag>::readRegisterWord(byte reg)
{
    uint16_t data = 0;
    _GPIOstartSend(1);
    MCP23S17Tag::SPI->transfer(reg);
    data = MCP23S17Tag::SPI->transfer16(0);
    _GPIOendSend();
    return data;
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::writeRegisterByte(byte reg, byte data, bool both)
{
    if (!both)
    {
        _GPIOwriteByte(reg, data);
    }
    else
    {
        _GPIOstartSend(0);
        MCP23S17Tag::SPI->transfer(reg);
        MCP23S17Tag::SPI->transfer(data);
        MCP23S17Tag::SPI->transfer(data);
        _GPIOendSend();
    }
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::writeRegisterWord(byte reg, word data)
{
    _GPIOwriteWord(reg, (word)data);
}