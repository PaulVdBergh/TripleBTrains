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
	{
		// TODO Auto-generated constructor stub

	}

	Accessory::~Accessory()
	{
		// TODO Auto-generated destructor stub
	}

	void Accessory::setTurnout(bool outputNbr, bool state)
	{

	}

} /* namespace TBT */
