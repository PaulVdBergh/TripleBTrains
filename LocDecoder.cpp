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
 * LocDecoder.cpp
 *
 *  Created on: Nov 24, 2017
 *      Author: paul
 */

#include "LocDecoder.h"

#include <cstring>

namespace TBT
{

	LocDecoder::LocDecoder(Manager* pManager, uint16_t dccAddress)
	:	Decoder(pManager, dccAddress)
	,	m_LocMode(LOCMODE_DCC)
	,	m_DCCState(0)
	,	m_CurrentCVRead(8)
	{
		// TODO Auto-generated constructor stub
		m_LocInfo.Addr_MSB = (m_DCCAddress >> 8);
		m_LocInfo.Addr_LSB = (m_DCCAddress & 0xFF);
		setSpeedsteps(4);
	}

	LocDecoder::~LocDecoder()
	{
		// TODO Auto-generated destructor stub
	}

	bool LocDecoder::getDccMessage(uint8_t* pMsg)
	{
		SystemState systemState;

		m_pManager->getSystemState(&systemState);

		if(systemState.CentralState)
		{
			lock_guard<recursive_mutex> lock(m_MLocInfo);
			pMsg[0] = 4;
			pMsg[1] = 0x00;
			pMsg[2] = 0x71;
			pMsg[3] = pMsg[1] ^ pMsg[2];
			return true;
		}
		else
		{
			switch(m_DCCState)
			{
				case 0:
				{
					//	Speed message
					bool rslt = getDCCSpeedMessage(pMsg);
					m_DCCState++;
					return rslt;
				}

				case 1:
				{
					//	Function group 1 (F0 - F4) Message
					bool rslt = getDCCFG1Message(pMsg);
					m_DCCState++;
					return rslt;
				}

				case 2:
				{
					//	Function group 2 (F5 - F8) Message
					bool rslt = getDCCFG2Message(pMsg);
					m_DCCState++;
					return rslt;
				}

				case 3:
				{
					//	Function group 3 (F9 - F12) Message
					bool rslt = getDCCFG3Message(pMsg);
					m_DCCState++;
					return rslt;
				}

				case 4:
				{
					//	Function group 4 (F13 - F20) Message
					bool rslt = getDCCFG4Message(pMsg);
					m_DCCState++;
					return rslt;
				}

				case 5:
				{
					//	Function group 5 (F21 - F28) Message
					bool rslt = getDCCFG5Message(pMsg);
					m_DCCState++;
					return rslt;
				}

				case 100:
				{
					//	POM - Read CV
					pMsg[0] = 6;
					pMsg[1] = m_LocInfo.Addr_LSB;
					pMsg[2] = 0xE4;
					pMsg[3] = m_CurrentCVRead;
					pMsg[4] = 0x00;
					pMsg[5] = pMsg[1] ^ pMsg[2] ^ pMsg[3] ^ pMsg[4];
//					m_DCCState++;
					return true;
				}

				case 101:
				{
					return false;
				}

				default:
				{
					m_DCCState = 0;
				}
			}
		}
		return false;
	}

	bool LocDecoder::getDCCSpeedMessage(uint8_t* pMsg)
	{
		uint8_t* pCurrent = insertDCCAddress(pMsg);

		if(getSpeedsteps() == 0x04)
		{
			//	128 speed steps
			*pCurrent++ = 0x3F;
			*pCurrent++ = getSpeed() | (getDirection() ? 0x80 : 0x00);
		}
		else
		{
			//	14 speed steps
			*pCurrent++ = 0x40 | (getDirection() ? 0x20 : 0x00) | getSpeed();
		}

		pMsg[0] = 1 + (pCurrent - pMsg);

		insertXOR(pMsg);

		return true;
	}

	bool LocDecoder::getDCCFG1Message(uint8_t* pMsg)
	{
		uint8_t* pCurrent = insertDCCAddress(pMsg);

		*pCurrent++ = 0x80 | getFunctionGroup1();

		pMsg[0] = 1 + (pCurrent - pMsg);

		insertXOR(pMsg);

		return true;
	}

	bool LocDecoder::getDCCFG2Message(uint8_t* pMsg)
	{
		uint8_t* pCurrent = insertDCCAddress(pMsg);

		*pCurrent++ = 0xB0 | getFunctionGroup2();

		pMsg[0] = 1 + (pCurrent - pMsg);

		insertXOR(pMsg);

		return true;
	}

	bool LocDecoder::getDCCFG3Message(uint8_t* pMsg)
	{
		uint8_t* pCurrent = insertDCCAddress(pMsg);

		*pCurrent++ = 0xA0 | getFunctionGroup3();

		pMsg[0] = 1 + (pCurrent - pMsg);

		insertXOR(pMsg);

		return true;
	}

	bool LocDecoder::getDCCFG4Message(uint8_t* pMsg)
	{
		uint8_t* pCurrent = insertDCCAddress(pMsg);

		*pCurrent++ = 0xDE;
		*pCurrent++ = getFunctionGroup4();

		pMsg[0] = 1 + (pCurrent - pMsg);

		insertXOR(pMsg);

		return true;
	}

	bool LocDecoder::getDCCFG5Message(uint8_t* pMsg)
	{
		uint8_t* pCurrent = insertDCCAddress(pMsg);

		*pCurrent++ = 0xDF;
		*pCurrent++ = getFunctionGroup5();

		pMsg[0] = 1 + (pCurrent - pMsg);

		insertXOR(pMsg);

		return true;
	}

	void LocDecoder::getLANLocInfo(uint8_t* pMsg)
	{ lock_guard<recursive_mutex> guard(m_MLocInfo);
		m_LocInfo.XOR = 0;
		for(uint8_t i = 4; i < *((uint8_t*)&m_LocInfo.DataLen); i++)
		{
			m_LocInfo.XOR ^= ((uint8_t*)&m_LocInfo)[i];
		}
		memcpy(pMsg, &m_LocInfo, *((uint8_t*)&m_LocInfo.DataLen));
	}

	uint8_t LocDecoder::setLocMode(uint8_t newMode)
	{
		if(256 > m_DCCAddress)
		{
			m_LocMode = newMode;
		}
		else
		{
			m_LocMode = LOCMODE_DCC;
		}
		return m_LocMode;
	}

} /* namespace TBT */
