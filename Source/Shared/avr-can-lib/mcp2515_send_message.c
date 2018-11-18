// ----------------------------------------------------------------------------
/*
 * Copyright (c) 2007 Fabian Greif, Roboterclub Aachen e.V.
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: mcp2515_send_message.c 7224 2009-01-25 20:19:32Z fabian $
 */
// ----------------------------------------------------------------------------

#include "mcp2515_private.h"
#ifdef	SUPPORT_FOR_MCP2515__

#include <util/delay.h>

/** current transmit buffer priority */
uint8_t txprio = 3;

// ----------------------------------------------------------------------------
uint8_t mcp2515_send_message(const can_t *msg)
{
	// Status des MCP2515 auslesen
	uint8_t status = mcp2515_read_status(SPI_READ_STATUS);
	uint8_t ctrlreg;
	
	/* Statusbyte:
	 *
	 * Bit	Funktion
	 *  2	TXB0CNTRL.TXREQ
	 *  4	TXB1CNTRL.TXREQ
	 *  6	TXB2CNTRL.TXREQ
	 */
	uint8_t address;
	
	// Do some priority fiddling to get FIFO behavior.
	switch (status & 0x54) {
		
		case 0x00:
			// all three buffers free
			ctrlreg = TXB2CTRL;
			address = 0x04;
			txprio = 3;
			break;
		
		case 0x40:
		case 0x44:
			ctrlreg = TXB1CTRL;
			address = 0x02;
			break;
		
		case 0x10:
		case 0x50:
			ctrlreg = TXB0CTRL;
			address = 0x00;
			break;
		
		case 0x04:
		case 0x14:
			ctrlreg = TXB2CTRL;
			address = 0x04;
		
			if (txprio == 0) {
				// Set priority of buffer 1 and buffer 0 to highest.
				mcp2515_bit_modify(TXB1CTRL, 0x03, 0x03);
				mcp2515_bit_modify(TXB0CTRL, 0x03, 0x03);
				txprio = 2;
			} else {
				txprio--;
			}
			break;
		
		default:
			// No free transmit buffer.
			return 0;
	}
	
	RESET(MCP2515_CS);
	spi_putc(SPI_WRITE_TX | address);
	#if SUPPORT_EXTENDED_CANID
		mcp2515_write_id(&msg->id, msg->flags.extended);
	#else
		mcp2515_write_id(&msg->id);
	#endif
	uint8_t length = msg->length & 0x0f;
	
	// Ist die Nachricht ein "Remote Transmit Request" ?
	if (msg->flags.rtr)
	{
		// Ein RTR hat zwar eine Laenge,
		// enthaelt aber keine Daten
		
		// Nachrichten Laenge + RTR einstellen
		spi_putc((1<<RTR) | length);
	}
	else
	{
		// Nachrichten Laenge einstellen
		spi_putc(length);
		
		// Daten
		for (uint8_t i=0;i<length;i++) {
			spi_putc(msg->data[i]);
		}
	}
	SET(MCP2515_CS);
	
	_delay_us(1);
	
	// Set buffer priority.
	mcp2515_bit_modify(ctrlreg, 0x03, txprio);
	
	// CAN Nachricht verschicken
	// die letzten drei Bit im RTS Kommando geben an welcher
	// Puffer gesendet werden soll.
	RESET(MCP2515_CS);
	address = (address == 0) ? 1 : address;
	spi_putc(SPI_RTS | address);
	SET(MCP2515_CS);
	
	CAN_INDICATE_TX_TRAFFIC_FUNCTION;
	
	return address;
}

#endif	// SUPPORT_FOR_MCP2515__
