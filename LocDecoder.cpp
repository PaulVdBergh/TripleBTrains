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
	{
		// TODO Auto-generated constructor stub
		m_LocInfo.Addr_MSB = (m_DCCAddress >> 8);
		m_LocInfo.Addr_LSB = (m_DCCAddress & 0xFF);
	}

	LocDecoder::~LocDecoder()
	{
		// TODO Auto-generated destructor stub
	}

	void LocDecoder::getLANLocInfo(uint8_t* pMsg)
	{ lock_guard<recursive_mutex> guard(m_MLocInfo);
		m_LocInfo.XOR = 0;
		for(uint8_t i = 0; i < *((uint8_t*)&m_LocInfo.DataLen); i++)
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