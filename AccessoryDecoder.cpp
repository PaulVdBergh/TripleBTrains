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
 * AccessoryDecoder.cpp
 *
 *  Created on: Feb 2, 2018
 *      Author: paul
 */

#include "AccessoryDecoder.h"

#include "Accessory.h"

namespace TBT
{

	AccessoryDecoder::AccessoryDecoder(Manager* pManager, uint16_t dccAddress)
	:	Decoder(pManager, dccAddress)
	{
		for(uint8_t port = 0; port < 4; port++)
		{
			m_pAccessories[port] = new Accessory(this, port);
		}

	}

	AccessoryDecoder::~AccessoryDecoder()
	{
		for(auto item : m_pAccessories)
		{
			delete item;
		}
	}

	void AccessoryDecoder::setTurnout(uint8_t port, bool outputNbr, bool state)
	{
		m_pAccessories[port]->setTurnout(outputNbr, state);
	}

} /* namespace TBT */
