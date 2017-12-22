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

#include <cstring>

#include "DccGenerator.h"
#include "LocDecoder.h"
#include "UDPClientInterface.h"
#include "WSClientInterface.h"
#include "XpressNetClientInterface.h"

namespace TBT
{

	Manager::Manager()
	{
		UDPClientInterface* pUDPClientIF = new UDPClientInterface(this);
		m_ClientInterfaces.push_back(pUDPClientIF);

		XpressNetClientInterface* pXpressNetClientIF = new XpressNetClientInterface(this);
		m_ClientInterfaces.push_back(pXpressNetClientIF);

		WSClientInterface* pWSClientInterface = new WSClientInterface(this);
		m_ClientInterfaces.push_back(pWSClientInterface);

		m_pDccGenerator = new DccGenerator(&m_Decoders, &m_MDecoders);
	}

	Manager::~Manager()
	{
		delete m_pDccGenerator;

		for(auto client : m_ClientInterfaces)
		{
			delete client;
		}
	}

	Decoder* Manager::findDecoder(uint16_t dccAddress)
	{
		lock_guard<recursive_mutex> guard(m_MDecoders);
		map<uint16_t, Decoder*>::iterator it = m_Decoders.find(dccAddress);
		if(it == m_Decoders.end())
		{
			return NULL;
		}
		return it->second;
	}

	void Manager::registerDecoder(Decoder* pDecoder)
	{
		lock_guard<recursive_mutex> guard(m_MDecoders);
		m_Decoders[pDecoder->getDCCAddress()] = pDecoder;
	}

	void Manager::unregisterDecoder(Decoder* pDecoder)
	{
		lock_guard<recursive_mutex> guard(m_MDecoders);
		m_Decoders.erase(pDecoder->getDCCAddress());
	}

	void Manager::broadcastLocInfoChanged(LocDecoder* pLoc)
	{
		lock_guard<recursive_mutex> guard(m_MClientInterfaces);
		for(auto interface : m_ClientInterfaces)
		{
			interface->broadcastLocInfoChange(pLoc);
		}
	}

	void Manager::setPowerState(PowerState newState)
	{
		{
			lock_guard<recursive_mutex> guard(m_MSystemState);
			if(newState == PowerOn)
			{
				m_SystemState.CentralState &= ~(csTrackVoltageOff | csEmergencyStop);
			}
			else
			{
				m_SystemState.CentralState |= csTrackVoltageOff;
			}
		}	//	guard unlocked

		{
			lock_guard<recursive_mutex> guard(m_MClientInterfaces);
			for(auto interface : m_ClientInterfaces)
			{
				interface->broadcastPowerStateChange(newState == PowerOn);
			}
		}	//	guard unlocked
	}

	void Manager::setEmergencyStop()
	{
		bool currentState = m_SystemState.CentralState & csEmergencyStop;
		if(currentState == false)
		{
			{
				lock_guard<recursive_mutex> guard(m_MSystemState);
				m_SystemState.CentralState |= csEmergencyStop;

			}

			{
				lock_guard<recursive_mutex> guard(m_MClientInterfaces);
				for(auto interface : m_ClientInterfaces)
				{
					interface->broadcastEmergencyStop();
				}
			}
		}
	}

	void Manager::getSystemState(SystemState* pMsg)
	{
		lock_guard<recursive_mutex> guard(m_MSystemState);
		memcpy(pMsg, &m_SystemState, m_SystemState.DataLen);
		//	guard unlocked
	}

} /* namespace TBT */
