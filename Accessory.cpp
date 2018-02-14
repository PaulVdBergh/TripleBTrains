/*
 * Copyright (C) 2018 Paul Van den Bergh
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
 * Accessory.cpp
 *
 *  Created on: Feb 6, 2018
 *      Author: paul
 */

#include "Accessory.h"

namespace TBT
{

	Accessory::Accessory(AccessoryDecoder* pAccessoryDecoder, uint8_t port)
	:	m_pAccDecoder(pAccessoryDecoder)
	,	m_Port(port)
	,	m_currentState{0, 0}
	,	m_desiredState{0, 0}
	,	m_UDPState(0)
	{
		// TODO Auto-generated constructor stub

	}

	Accessory::~Accessory()
	{
		// TODO Auto-generated destructor stub
	}

	uint8_t Accessory::getUDPState()
	{
		return m_UDPState;
	}

	void Accessory::setUDPState(uint8_t newState)
	{
		if(m_UDPState != newState)
		{
			m_UDPState = newState;
			m_pAccDecoder->m_pManager->broadcastAccessoryInfoChanged(this);
		}
	}

	bool Accessory::getDCCMessage(uint8_t* pMsg)
	{
		if(csTrackVoltageOff & m_pAccDecoder->m_pManager->getCentralState())
		{
			return false;
		}

		bool retval = false;
		pMsg[0] = 4;
		pMsg[1] = 0x80 | (m_pAccDecoder->getDCCAddress() & 0x003F);
		pMsg[2] = 0x80 | ((~(m_pAccDecoder->getDCCAddress() & 0x01C0) >> 2) & 0x70) | ((m_Port & 0x03) << 1);
		if((m_desiredState[0] == 0) && (m_currentState[0] != 0))
		{
			retval = true;
			m_currentState[0]--;
		}
		else if((m_desiredState[1] == 0) && (m_currentState[1] != 0))
		{
			pMsg[2] |= 0x01;
			retval = true;
			m_currentState[1]--;

		}
		else if((m_desiredState[0] == ACCESSORYREPEATCOUNT) && (m_currentState[0] != ACCESSORYREPEATCOUNT))
		{
			pMsg[2] |= 0x08;
			retval = true;
			if(++m_currentState[0] == ACCESSORYREPEATCOUNT)
			{
				setUDPState(1);
			}
		}
		else if((m_desiredState[1] == ACCESSORYREPEATCOUNT) && (m_currentState[1] != ACCESSORYREPEATCOUNT))
		{
			pMsg[2] |= 0x09 ;
			retval = true;
			if(++m_currentState[1] == ACCESSORYREPEATCOUNT)
			{
				setUDPState(2);
			}
		}

		pMsg[3] = pMsg[1] ^ pMsg[2];

		return retval;
	}

	void Accessory::setState(uint8_t outputNbr, uint8_t state)
	{
		if(state)
		{
			uint16_t FAddr = (m_pAccDecoder->getDCCAddress() << 2) + (m_Port & 0x03);
			printf("Accessory %i : set output %i to %i.\n", FAddr, outputNbr, state);
			m_desiredState[outputNbr] = state * ACCESSORYREPEATCOUNT;
			m_desiredState[outputNbr ? 0 : 1] = 0;
		}
	}

} /* namespace TBT */
