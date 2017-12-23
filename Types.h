/*
 * Copyright (C) 2017 Paul Van den Bergh
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Types.h
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

namespace TBT
{
	enum PowerState
	{
		PowerOn,
		PowerOff
	};

	#define csEmergencyStop			0x01
	#define csTrackVoltageOff		0x02
	#define csShortCircuit			0x04
	#define csProgrammingModeActive	0x20

	#define cseHighTemperature		0x01
	#define csePowerLost			0x02
	#define cseShortCircuitExternal	0x04
	#define cseShortCircuitInternal	0x08

	/**
	 *
     <table>
	 <caption>response:</caption>
	 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
	 <tr><td>0x14<td>0x00<td>0x84<td>0x00<td>SystemState (16 Bytes)
	 </table>

	 <table>
	 <caption><b>SystemState</b> layout (INT16 is little-endian)</caption>
	 <tr><th>Byte offset<th>Type<th>Name<th>Unit<th>note
	 <tr><td>0<td>int16_t<td>MainCurrent<td>mA<td>Main Track Current
	 <tr><td>2<td>int16_t<td>ProgCurrent<td>mA<td>Programm Track Current
	 <tr><td>4<td>int16_t<td>FilteredMainCurrent<td>mA<td>Smoothed Main Track Current
	 <tr><td>6<td>int16_t<td>Temperature<td>&#176;C<td>Internal temperature
	 <tr><td>8<td>uint16_t<td>SupplyVoltage<td>mV<td>Supply voltage
	 <tr><td>10<td>uint16_t<td>VCCVoltage<td>mV<td>Voltage at the track
	 <tr><td>12<td>uint8_t<td>CentralState<td>bitmask<td>see Manager::getCentralState
	 <tr><td>13<td>uint8_t<td>CentralStateEx<td>bitmask<td>see Manager::getCentralStateEx
	 <tr><td>14<td>uint8_t<td>reserved<td><td>
	 <tr><td>15<td>uint8_t<td>reserved<td><td>
	 </table>
	 */
	struct SystemState
	{
		uint8_t DataLen = 0x14;
		uint8_t filler1 = 0x00;
		uint8_t Header = 0x84;
		uint8_t filler2 = 0x00;
		int16_t MainCurrent = 0x0000;
		int16_t ProgCurrent = 0x0000;
		int16_t FilteredMainCurrent = 0x0000;
		int16_t Temperature = 0x0000;
		uint16_t SupplyVoltage = 0x0000;
		uint16_t VCCVoltage = 0x0000;
		uint8_t CentralState = csTrackVoltageOff;
		uint8_t CentralStateEx = 0x00;
		uint8_t reserved1 = 0x00;
		uint8_t reserved2 = 0x00;
	}__attribute__((packed));

	struct LocInfo
	{
		uint32_t DataLen = 0x0040000E;
		uint8_t X_Header = 0xEF;
		uint8_t Addr_MSB;
		uint8_t Addr_LSB;
		uint8_t DB2;
		uint8_t DB3;
		uint8_t DB4;
		uint8_t DB5;
		uint8_t DB6;
		uint8_t DB7;
		uint8_t XOR;
	}__attribute__((packed));

	#define LOCMODE_DCC	0
	#define LOCMODE_MM	1

}	/* namespace TBT */

#endif /* TYPES_H_ */
