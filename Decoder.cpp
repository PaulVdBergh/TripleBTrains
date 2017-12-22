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
 * Decoder.cpp
 *
 *  Created on: Nov 24, 2017
 *      Author: paul
 */

#include "Decoder.h"

namespace TBT
{

	Decoder::Decoder(Manager* pManager, uint16_t dccAddress)
	:	m_DCCAddress(dccAddress)
	,	m_pManager(pManager)
	{
		m_pManager->registerDecoder(this);
	}

	Decoder::~Decoder()
	{
		m_pManager->unregisterDecoder(this);
	}

	uint8_t* Decoder::insertDCCAddress(uint8_t* pMsg)
	{
		uint8_t* pCurrent = &pMsg[1];
		if(m_DCCAddress < 128)
		{
			*pCurrent++ = m_DCCAddress & 0xFF;
		}
		else
		{
			*pCurrent++ = 0xC0 | ((m_DCCAddress >> 8) & 0x3F);
			*pCurrent++ = m_DCCAddress & 0xFF;
		}

		return pCurrent;
	}

	void Decoder::insertXOR(uint8_t* pMsg)
	{
		uint8_t* pXOR = pMsg + (pMsg[0] - 1);

		*pXOR = 0;
		for(uint8_t* pIndex = &pMsg[1]; pIndex < pXOR; pIndex++)
		{
			*pXOR ^= *pIndex;
		}
	}

} /* namespace TBT */
