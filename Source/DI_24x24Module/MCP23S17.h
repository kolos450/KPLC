/*
 ___  _   _  _ __ ___    ___  | |_  ___   _   _ 
/ __|| | | || '_ ` _ \  / _ \ | __|/ _ \ | | | |
\__ \| |_| || | | | | || (_) || |_| (_) || |_| |
|___/ \__,_||_| |_| |_| \___/  \__|\___/  \__, |
                                          |___/ 
										  
	MCP23S17 - A complete library for Microchip MCP23S17 for many MCU's.
	
model:			company:		pins:		protocol:		Special Features:
---------------------------------------------------------------------------------------------------------------------
mcp23s17		Microchip		 16			SPI					INT/HAEN
---------------------------------------------------------------------------------------------------------------------
Version history:
0.9 Fixed an example, added getInterruptNumber function.
0.95 Added compatibility with ESP8266
---------------------------------------------------------------------------------------------------------------------
		Copyright (c) 2013-2014, s.u.m.o.t.o.y [sumotoy(at)gmail.com]
---------------------------------------------------------------------------------------------------------------------

    MCP23S17 Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MCP23S17 Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

*/

/* ------------------------------ MCP23S17 WIRING ------------------------------------
This chip has a very useful feature called HAEN that allow you to share the same CS pin trough
8 different addresses. Of course chip has to be Microchip and should be assigned to different addresses!

Basic Address:  00100 A2 A1 A0 (from 0x20 to 0x27)
A2,A1,A0 tied to ground = 0x20
				__ __
		IOB-0 [|  U  |] IOA-7
		IOB-1 [|     |] IOA-6
		IOB-2 [|     |] IOA-5
		IOB-3 [|     |] IOA-4
		IOB-4 [|     |] IOA-3
		IOB-5 [|     |] IOA-2
		IOB-6 [|     |] IOA-1
		IOB-7 [|     |] IOA-0
		++++  [|     |] INT-A
		GND   [|     |] INT-B
		CS    [|     |] RST (connect to +)
		SCK   [|     |] A2
		MOSI  [|     |] A1
		MISO  [|_____|] A0
*/
#pragma once

#include <inttypes.h>

#include "Arduino/SPI.h"

#if defined (SPI_HAS_TRANSACTION)
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328PB__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega324PA__)
const static uint32_t _MCPMaxSpeed = 8000000UL;
#else
const static uint32_t _MCPMaxSpeed = 2000000UL;
#endif
#endif


/*   The IOCON register!
		 7		6     5	     4     3   2     1    0
IOCON = BANK MIRROR SEQOP DISSLW HAEN ODR INTPOL -NC-
-----------------------------------------------------------------------
0b01101100
BANK: (Controls how the registers are addressed)
1 The registers associated with each port are separated into different banks
0 The registers are in the same bank (addresses are sequential)
MIRROR: (INT Pins Mirror bit)
1 The INT pins are internally connected
0 The INT pins are not connected. INTA is associated with PortA and INTB is associated with PortB
SEQOP: (Sequential Operation mode bit)
1 Sequential operation disabled, address pointer does not increment
0 Sequential operation enabled, address pointer increments.
DISSLW: (Slew Rate control bit for SDA output, only I2C)
HAEN: (Hardware Address Enable bit, SPI only)
1 Enables the MCP23S17 address pins
0 Disables the MCP23S17 address pins
ODR: (This bit configures the INT pin as an open-drain output)
1 Open-drain output (overrides the INTPOL bit).
0 Active driver output (INTPOL bit sets the polarity).
INTPOL: (This bit sets the polarity of the INT output pin)
1 Active high
0 Active low
*/

#include "MCP23S17_registers.h"

template <class MCP23S17Tag>
class MCP23S17 {

public:
	
	MCP23S17(const uint8_t haenAdrs); // Any pin, 0x20....0x27.

	void 			begin(bool protocolInitOverride=false); // protocolInitOverride=true	will not init the SPI.
    
	void 			gpioPinMode(uint16_t mode);					// OUTPUT=all out, INPUT=all in, 0xxxx=you choose.
	void 			gpioPinMode(uint8_t pin, bool mode);		// Set a unique pin as IN(1) or OUT (0).
	void 			gpioPort(uint16_t value);					// HIGH=all Hi, LOW=all Low,0xxxx=you choose witch low or hi.
	void			gpioPort(byte lowByte, byte highByte);		// Same as abowe but uses 2 separate bytes.
	uint16_t 		readGpioPort();								// Read the state of the pins (all).					
	
	void 			gpioDigitalWrite(uint8_t pin, bool value);  // Write data to one pin.
	int 			gpioDigitalRead(uint8_t pin);				// Read data from one pin.
	uint8_t		 	gpioRegisterReadByte(byte reg);					// Read a byte from chip register.
	uint16_t		gpioRegisterReadWord(byte reg);					// Read a word from chip register.
	void 			gpioRegisterWriteByte(byte reg,byte data,bool both=false); // Write a byte in a chip register, optional for both ports.
					// If both=true it will write the same register in bank A & B.
	void 			gpioRegisterWriteWord(byte reg,word data);		// Write a word in a chip register.
	void			portPullup(uint16_t data);						// HIGH=all pullup, LOW=all pulldown, 0xxxx=you choose.
//	int 			getInterruptNumber(byte pin);
	// Direct access commands
	uint16_t 		gpioReadAddress(byte addr);

	

protected:
	
	
	inline __attribute__((always_inline))
	void _GPIOstartSend(uint8_t mode) {
	#if defined (SPI_HAS_TRANSACTION)
		MCP23S17Tag::SPI->beginTransaction(SPISettings(_MCPMaxSpeed, MSBFIRST, SPI_MODE0));
	#endif

		MCP23S17Tag::SetCS();
		mode == 1 ? MCP23S17Tag::SPI->transfer(_readCmd) : MCP23S17Tag::SPI->transfer(_writeCmd);
	}
	
	
	inline __attribute__((always_inline))
	void _GPIOendSend(void) {
	
		MCP23S17Tag::ResetCS();

	#if defined (SPI_HAS_TRANSACTION)
		MCP23S17Tag::SPI->endTransaction();
	#endif
	}
	
	inline __attribute__((always_inline))
	void _GPIOwriteByte(byte addr, byte data){
		_GPIOstartSend(0);
		MCP23S17Tag::SPI->transfer(addr);
		MCP23S17Tag::SPI->transfer(data);
		_GPIOendSend();
	}
	
	inline __attribute__((always_inline))
	void _GPIOwriteWord(byte addr, uint16_t data){
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

//return 255 if the choosed pin has no INT, otherwise return INT number
//if there's support for SPI transactions it will use SPI.usingInterrupt(intNum);
//to prevent problems from interrupt
/*USE:
  int intNumber = mcp.getInterruptNumber(gpio_int_pin);
  if (intNumber < 255){
    attachInterrupt(intNumber, keypress, FALLING);//attack interrupt
  } else {
    Serial.println("sorry, pin has no INT capabilities!");
  }
 */

// template <class MCP23S17Tag>
// int MCP23S17<MCP23S17Tag>::getInterruptNumber(byte pin) {
// 	int intNum = digitalPinToInterrupt(pin);
// 	if (intNum != NOT_AN_INTERRUPT) {
// 		#if defined (SPI_HAS_TRANSACTION)
// 			SPI1.usingInterrupt(intNum);
// 		#endif
// 		return intNum;
// 	}
// 	return 255;
// }

template <class MCP23S17Tag>
MCP23S17<MCP23S17Tag>::MCP23S17(const uint8_t haenAdrs)
{
	if (haenAdrs > 0x19 && haenAdrs < 0x28){//HAEN works between 0x20...0x27
		_adrs = haenAdrs;
		_useHaen = 1;
		} else {
		_adrs = 0;
		_useHaen = 0;
	}
	_readCmd =  (_adrs << 1) | 1;
	_writeCmd = _adrs << 1;
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::begin(bool protocolInitOverride) {
	if (!protocolInitOverride){
		MCP23S17Tag::SPI->begin();
		#if !defined (SPI_HAS_TRANSACTION)
			MCP23S17Tag::SPI->setClockDivider(SPI_CLOCK_DIV4); // 4 MHz (half speed)
			MCP23S17Tag::SPI->setBitOrder(MSBFIRST);
			MCP23S17Tag::SPI->setDataMode(SPI_MODE0);
		#endif
	}
	MCP23S17Tag::InitCS();
	delay(100);
	
	_useHaen == 1 ? _GPIOwriteByte(MCP23S17_IOCON,0b00101000) : _GPIOwriteByte(MCP23S17_IOCON,0b00100000);
	_gpioDirection = 0xFFFF;//all in
	_gpioState = 0xFFFF;//all low 
}



template <class MCP23S17Tag>
uint16_t MCP23S17<MCP23S17Tag>::gpioReadAddress(byte addr){
	_GPIOstartSend(1);
	MCP23S17Tag::SPI->transfer(addr);
	uint16_t temp = MCP23S17Tag::SPI->transfer16(0x0);
	_GPIOendSend();
	return temp;
}


template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::gpioPinMode(uint16_t mode){
	if (mode == INPUT){
		_gpioDirection = 0xFFFF;
	} else if (mode == OUTPUT){	
		_gpioDirection = 0x0000;
		_gpioState = 0x0000;
	} else {
		_gpioDirection = mode;
	}
	_GPIOwriteWord(MCP23S17_IODIR,_gpioDirection);
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::gpioPinMode(uint8_t pin, bool mode){
	if (pin < 16){//0...15
		mode == INPUT ? _gpioDirection |= (1 << pin) :_gpioDirection &= ~(1 << pin);
		_GPIOwriteWord(MCP23S17_IODIR,_gpioDirection);
	}
}


template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::gpioPort(uint16_t value){
	if (value == HIGH){
		_gpioState = 0xFFFF;
	} else if (value == LOW){	
		_gpioState = 0x0000;
	} else {
		_gpioState = value;
	}
	_GPIOwriteWord(MCP23S17_GPIO,_gpioState);
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::gpioPort(byte lowByte, byte highByte){
	_gpioState = highByte | (lowByte << 8);
	_GPIOwriteWord(MCP23S17_GPIO,_gpioState);
}


template <class MCP23S17Tag>
uint16_t MCP23S17<MCP23S17Tag>::readGpioPort(){
	return gpioReadAddress(MCP23S17_GPIO);
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::portPullup(uint16_t data) {
	if (data == HIGH){
		_gpioState = 0xFFFF;
	} else if (data == LOW){	
		_gpioState = 0x0000;
	} else {
		_gpioState = data;
	}
	_GPIOwriteWord(MCP23S17_GPPU, _gpioState);
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::gpioDigitalWrite(uint8_t pin, bool value){
	if (pin < 16){//0...15
		value == HIGH ? _gpioState |= (1 << pin) : _gpioState &= ~(1 << pin);
		_GPIOwriteWord(MCP23S17_GPIO,_gpioState);
	}
}

template <class MCP23S17Tag>
int MCP23S17<MCP23S17Tag>::gpioDigitalRead(uint8_t pin){
	if (pin < 16) return (int)(gpioReadAddress(MCP23S17_GPIO) & 1 << pin);
	return 0;
}

template <class MCP23S17Tag>
uint8_t MCP23S17<MCP23S17Tag>::gpioRegisterReadByte(byte reg){
  uint8_t data = 0;
    _GPIOstartSend(1);
    MCP23S17Tag::SPI->transfer(reg);
    data = MCP23S17Tag::SPI->transfer(0);
    _GPIOendSend();
  return data;
}

template <class MCP23S17Tag>
uint16_t MCP23S17<MCP23S17Tag>::gpioRegisterReadWord(byte reg){
  uint16_t data = 0;
    _GPIOstartSend(1);
    MCP23S17Tag::SPI->transfer(reg);
	data = MCP23S17Tag::SPI->transfer16(0);
    _GPIOendSend();
  return data;
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::gpioRegisterWriteByte(byte reg,byte data,bool both){
	if (!both){
		_GPIOwriteByte(reg,(byte)data);
	} else {
		_GPIOstartSend(0);
		MCP23S17Tag::SPI->transfer(reg);
		MCP23S17Tag::SPI->transfer(data);
		MCP23S17Tag::SPI->transfer(data);
		_GPIOendSend();
	}
}

template <class MCP23S17Tag>
void MCP23S17<MCP23S17Tag>::gpioRegisterWriteWord(byte reg,word data){
	_GPIOwriteWord(reg,(word)data);
}