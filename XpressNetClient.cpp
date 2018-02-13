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
 * XpressNetClient.cpp
 *
 *  Created on: Nov 24, 2017
 *      Author: paul
 */

#include "XpressNetClient.h"
#include "XpressNetClientInterface.h"

#include <unistd.h>

namespace TBT
{

	XpressNetClient::XpressNetClient(ClientInterface* pInterface, uint8_t XpressNetAddress)
	:	Client(pInterface)
	,	m_XpressNetAddress(XpressNetAddress)
	{
		// TODO Auto-generated constructor stub

	}

	XpressNetClient::~XpressNetClient()
	{
		// TODO Auto-generated destructor stub
	}

	void XpressNetClient::broadcastPowerStateChange(bool newState)
	{
		//	TODO implementation
	}


	void XpressNetClient::broadcastLocInfoChanged(LocDecoder* pLoc)
	{
		//	TODO implementation
	}

	void XpressNetClient::broadcastAccessoryInfoChanged(Accessory* pAccessory)
	{
		//	TODO implementation
	}

	void XpressNetClient::broadcastOvercurrent()
	{
		uint8_t msg[] = { 0x05, 0x60, 0x61, 0x12, 0x73 };
		msg[1] += m_XpressNetAddress;
		for(uint8_t x = 0x40; x != 0; x = x >> 1)
		{
			if(msg[1] & x)
			{
				msg[1] ^= 0x80;
			}
		}

		XpressNetClientInterface* pInterface = dynamic_cast<XpressNetClientInterface*>(getInterface());
		if(pInterface)
		{
			write(pInterface->getSerial(), msg, msg[0]);
		}
	}

} /* namespace TBT */
