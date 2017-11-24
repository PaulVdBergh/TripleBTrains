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
 * UDPClient.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#include "UDPClient.h"

#include "LocDecoder.h"

namespace TBT
{

	UDPClient::UDPClient(UDPClientInterface* pinterface, const sockaddr_in& address)
	:	Client(pinterface)
	,	m_Address(address)
	,	m_BroadcastFlags(0)
	{
		m_MySocket = ((UDPClientInterface*)m_pInterface)->getMySocket();
	}

	UDPClient::~UDPClient()
	{
		// TODO Auto-generated destructor stub
	}

	void UDPClient::broadcastPowerStateChange(PowerState newState)
	{
		if(PowerOn == newState)
		{
			const uint8_t msg[] = { 0x07, 0x00, 0x40, 0x00, 0x61, 0x01, 0x60 };
			sendto(m_MySocket, msg, msg[0], 0, (sockaddr*)&m_Address, sizeof(m_Address));
		}
		else
		{
			const uint8_t msg[] = { 0x07, 0x00, 0x40, 0x00, 0x61, 0x00, 0x61 };
			sendto(m_MySocket, msg, msg[0], 0, (sockaddr*)&m_Address, sizeof(m_Address));
		}
	}

	void UDPClient::broadcastLocInfoChanged(LocDecoder* pLoc)
	{
		uint8_t pMsg[0x0E];
		pLoc->getLANLocInfo(pMsg);

		sendto(m_MySocket, pMsg, pMsg[0], 0, (sockaddr*)&m_Address, sizeof(m_Address));
	}

	void UDPClient::broadcastEmergencyStop(bool state)
	{
		if(state)
		{
			uint8_t msg[] = { 0x07, 0x00, 0x40, 0x00, 0x81, 0x00, 0x81};
			sendto(m_MySocket, msg, msg[0], 0, (sockaddr*)&m_Address, sizeof(m_Address));
		}
	}

	uint32_t UDPClient::getBroadcastFlags()
	{
		return m_BroadcastFlags;
	}

} /* namespace TBT */
