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
 * Manager.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#include "Manager.h"
#include "Interface.h"

namespace TBT
{

	Manager::Manager()
	:	m_PowerState(PowerOff)
	{
		// TODO Auto-generated constructor stub
	}

	Manager::~Manager()
	{
		// TODO Auto-generated destructor stub
	}

	void Manager::setPowerState(PowerState newState)
	{
		if(m_PowerState != newState)
		{
			m_PowerState = newState;
			for(auto interface : m_Interfaces)
			{
				interface->broadcastPowerStateChange(m_PowerState);
			}
		}
	}

} /* namespace TBT */
