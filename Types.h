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

	#define LOCMODE_DCC	0
	#define LOCMODE_MM	1

}	/* namespace TBT */

#endif /* TYPES_H_ */
