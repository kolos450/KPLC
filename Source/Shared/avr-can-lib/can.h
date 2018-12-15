// coding: utf-8
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
 */
// ----------------------------------------------------------------------------
/**
 * \file    can.h
 * \brief   Header-Datei für das allgemeine CAN Interface
 */
// ----------------------------------------------------------------------------

#ifndef CAN_H
#define CAN_H

#if defined (__cplusplus)
	extern "C" {
#endif

// ----------------------------------------------------------------------------
/**
 * \ingroup		communication
 * \defgroup 	can_interface Universelles CAN Interface
 * \brief		allgemeines CAN Interface für MCP2515
 *
 * \author 		Fabian Greif <fabian.greif@rwth-aachen.de>
 * \author      Roboterclub Aachen e.V. (http://www.roboterclub.rwth-aachen.de)
 * 
 * can_sleep() and can_wakeup() functions by Frédéric Lamorce.
 * 
 * \version		$Id: can.h 8086 2009-07-14 14:08:25Z fabian $
 */
// ----------------------------------------------------------------------------

#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>

#include "can_config.h"

// ----------------------------------------------------------------------------
/** \ingroup	can_interface
 *  \name		Bit Definitions
 */
//@{
#define	ONLY_NON_RTR		2
#define	ONLY_RTR			3
//@}

/** \ingroup	can_interface
 *  \brief		CAN bitrates 
 */
typedef enum {
	BITRATE_10_KBPS	= 0,	// untested
	BITRATE_20_KBPS	= 1,	// untested
	BITRATE_50_KBPS	= 2,	// untested
	BITRATE_100_KBPS = 3,	// untested
	BITRATE_125_KBPS = 4,
	BITRATE_250_KBPS = 5,	// untested
	BITRATE_500_KBPS = 6,	// untested
	BITRATE_1_MBPS = 7,		// untested
} can_bitrate_t;

/**
 * \ingroup	    can_interface
 * \brief		All filters access shortcut
 */
#define	CAN_ALL_FILTER		0xff

/**
 * \ingroup	    can_interface
 * \brief		Enable support for Extended IDs
 */
#ifndef	SUPPORT_EXTENDED_CANID
	#define	SUPPORT_EXTENDED_CANID	1
#endif

/**
 * \ingroup	    can_interface
 * \name		Bits des Filters fuer den MCP2515 umformatieren
 *
 * \code
 *  prog_uint8_t can_filter[] =
 *  {
 *  	MCP2515_FILTER_EXTENDED(0),	// Filter 0
 *  	MCP2515_FILTER_EXTENDED(0),	// Filter 1
 *  	
 *  	MCP2515_FILTER_EXTENDED(0),	// Filter 2
 *  	MCP2515_FILTER_EXTENDED(0),	// Filter 3
 *  	MCP2515_FILTER_EXTENDED(0),	// Filter 4
 *  	MCP2515_FILTER_EXTENDED(0),	// Filter 5
 *  	
 *  	MCP2515_FILTER_EXTENDED(0),	// Maske 0
 *  	MCP2515_FILTER_EXTENDED(0),	// Maske 1
 *  };
 * \endcode
 *
 * \see			can_static_filter()
 *
 * \~german
 * \warning		Dieses Makro sollte nur Werte verwendet die schon zur
 *				Compile-Zeit bekannt sind. Der Code sollte ansonsten zwar trotzdem
 *				funktionieren, wird danner aber sehr groß.
 *
 * \~english
 * \warning		Do not use this Makro for Variables, only for static values
 *				known at compile-time.
 */
//@{
#if defined(__DOXYGEN__)

	#define	MCP2515_FILTER_EXTENDED(id)
	#define	MCP2515_FILTER(id)

#else

	#if SUPPORT_EXTENDED_CANID	
		#define MCP2515_FILTER_EXTENDED(id)	\
				(uint8_t)  ((uint32_t) (id) >> 21), \
				(uint8_t)((((uint32_t) (id) >> 13) & 0xe0) | (1<<3) | \
					(((uint32_t) (id) >> 16) & 0x3)), \
				(uint8_t)  ((uint32_t) (id) >> 8), \
				(uint8_t)  ((uint32_t) (id))
	#endif
	
	#define	MCP2515_FILTER(id) \
			(uint8_t)((uint32_t) id >> 3), \
			(uint8_t)((uint32_t) id << 5), \
			0, \
			0
#endif
//@}
// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Datenstruktur zum Aufnehmen von CAN Nachrichten
 */
typedef struct
{
	#if SUPPORT_EXTENDED_CANID	
		uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
		struct {
			int rtr : 1;			//!< Remote-Transmit-Request-Frame?
			int extended : 1;		//!< extended ID?
		} flags;
	#else
		uint16_t id;				//!< ID der Nachricht (11 Bit)
		struct {
			int rtr : 1;			//!< Remote-Transmit-Request-Frame?
		} flags;
	#endif
	
	uint8_t length;				//!< Anzahl der Datenbytes
	uint8_t data[8];			//!< Die Daten der CAN Nachricht
} can_t;



// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Data structure for CAN filters setup.
 *
 * \code
 *  ext | Function
 * -----|------
 *  00  | accept all
 *  01  | reserved
 *  10  | accept standart frames only
 *  11  | accept extended frames only
 * \endcode
 *
 */

typedef struct
{
	#if	SUPPORT_EXTENDED_CANID
		uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
		uint32_t mask;				//!< Maske
		struct {
			uint8_t extended : 2;	//!< extended ID
		} flags;
	#else
		uint16_t id;				//!< ID der Nachricht 11 Bits
		uint16_t mask;				//!< Maske
	#endif
} can_filter_t;


// ----------------------------------------------------------------------------
/**
 * \ingroup can_interface
 * \brief   Inhalt der Fehler-Register
 */
typedef struct {
	uint8_t rx;				//!< Empfangs-Register
	uint8_t tx;				//!< Sende-Register
} can_error_register_t;

// ----------------------------------------------------------------------------
/**
 * \ingroup can_interface
 * \brief   Modus des CAN Interfaces
 */
typedef enum {
	LISTEN_ONLY_MODE,		//!< der CAN Contoller empfängt nur und verhält sich völlig passiv
	LOOPBACK_MODE,			//!< alle Nachrichten direkt auf die Empfangsregister umleiten ohne sie zu senden
	NORMAL_MODE,				//!< normaler Modus, CAN Controller ist aktiv
	SLEEP_MODE					// sleep mode
} can_mode_t;

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Initialisierung des CAN Interfaces
 *
 * \param	bitrate	Gewuenschte Geschwindigkeit des CAN Interfaces
 *
 * \return	false falls das CAN Interface nicht initialisiert werden konnte,
 *			true ansonsten.
 */
extern bool
can_init(can_bitrate_t bitrate);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * 
 * \~english
 * \brief	Put CAN interface to sleep and wake up
 * 
 * MCP2515 active : 5mA
 * MCP2515 sleep  : 1µA
 * 
 * MCP2551 active : 10mA+
 * MCP2551 sleep  : 400µA
 * 
 * \code
 * // before we are going to sleep, enable the interrupt that will wake us up
// attach interrupt 1 to the routine
EICRA = 0;  // int on low level
// Enable the interrupt1
EIMSK = _BV(INT1);

// put the MCP2515 to sleep
can_sleep();

// enable atmega sleep mode
cli();
sleep_enable();
// turn off BOD
sleep_bod_disable();
// and we go to sleep
sei();
sleep_cpu();
	
// here int1 has been executed and we are woken up
sleep_disable();
	
// disable int1
EIMSK = 0;
	
// re-enable 2515 and 2551
can_wake();
 * \endcode
 * 
 * \warning	Only implemented for the MCP2515
 */
extern void
can_sleep(void);

extern void
can_wakeup(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzen eines Filters
 * 
 * Für einen MCP2515 sollte die Funktion can_static_filter() bevorzugt werden.
 *
 * \param	number	Position des Filters
 * \param	filter	zu setzender Filter
 *
 * \return	false falls ein Fehler auftrat, true ansonsten
 */
extern bool
can_set_filter(uint8_t number, const can_filter_t *filter);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzt die Werte für alle Filter
 *
 * \code
 * // Filter und Masken-Tabelle anlegen
 * prog_char can_filter[] = {
 * 	MCP2515_FILTER_EXTENDED(0),	// Filter 0
 * 	MCP2515_FILTER_EXTENDED(0),	// Filter 1
 * 	
 * 	MCP2515_FILTER_EXTENDED(0),	// Filter 2
 * 	MCP2515_FILTER_EXTENDED(0),	// Filter 3
 * 	MCP2515_FILTER_EXTENDED(0),	// Filter 4
 * 	MCP2515_FILTER_EXTENDED(0),	// Filter 5
 * 	
 * 	MCP2515_FILTER_EXTENDED(0),	// Maske 0
 * 	MCP2515_FILTER_EXTENDED(0),	// Maske 1
 * };
 * 
 * ...
 *
 * // Filter und Masken-Tabelle laden
 * can_static_filter(can_filter);
 * \endcode
 *
 * \param	*filter_array	Array im Flash des AVRs mit den Initialisierungs-
 *							werten für die Filter des MCP2515
 * 
 * \see		MCP2515_FILTER_EXTENDED()
 * \see		MCP2515_FILTER()
 * \warning	Wird nur vom MCP2515 unterstuetzt.
 */
extern void
can_static_filter(const uint8_t *filter_array);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * 
 * \~german
 * \brief	Filterdaten auslesen
 *
 * \param	number	Nummer des Filters dessen Daten man haben moechte
 * \param	*filter	Pointer in den die Filterstruktur geschrieben wird
 *
 * \return	\b 0 falls ein Fehler auftrat, \
 *			\b 1 falls der Filter korrekt gelesen werden konnte.
 *
 */
extern uint8_t
can_get_filter(uint8_t number, can_filter_t *filter);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Ueberpruefen ob neue CAN Nachrichten vorhanden sind
 *
 * \return	true falls neue Nachrichten verfuegbar sind, false ansonsten.
 */
extern bool
can_check_message(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Ueberprueft ob ein Puffer zum Versenden einer Nachricht frei ist.
 *
 * \return	true falls ein Sende-Puffer frei ist, false ansonsten.
 */
extern bool
can_check_free_buffer(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Verschickt eine Nachricht über den CAN Bus
 *
 * \param	msg	Nachricht die verschickt werden soll
 * \return	FALSE falls die Nachricht nicht verschickt werden konnte, \n
 *			ansonsten der Code des Puffes in den die Nachricht gespeichert wurde
 */
extern uint8_t
can_send_message(const can_t *msg);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Liest eine Nachricht aus den Empfangspuffern des CAN Controllers
 *
 * \param	msg	Pointer auf die Nachricht die gelesen werden soll.
 * \return	FALSE falls die Nachricht nicht ausgelesen konnte,
 *			ansonsten Filtercode welcher die Nachricht akzeptiert hat.
 */
extern uint8_t
can_get_message(can_t *msg);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 *
 * \~german
 * \brief   Liest den Inhalt der Fehler-Register
 *
 * \~english
 * \brief	Reads the Contents of the CAN Error Counter
 */
extern can_error_register_t
can_read_error_register(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzt den Operations-Modus
 *
 * \param	mode	Gewünschter Modus des CAN Controllers
 */
extern void
can_set_mode(can_mode_t mode);

typedef enum {
	CanErrorFlags_Rx1Overflow		= 0x80,
	CanErrorFlags_Rx0Overflow		= 0x40,
	CanErrorFlags_TxBusOff			= 0x20,
	CanErrorFlags_TxErrorPassive	= 0x10,
	CanErrorFlags_RxErrorPassive	= 0x08,
	CanErrorFlags_TxWarning			= 0x04,
	CanErrorFlags_RxWarning			= 0x02,
	CanErrorFlags_ErrorWarning		= 0x01
} can_error_flags_t;

extern can_error_flags_t
can_read_error_flags(void);

extern void
can_reset_error_flags(void);

#if defined (__cplusplus)
}
#endif

#endif // CAN_H
