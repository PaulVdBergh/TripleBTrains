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

	/**
	 *	Default constructor for class Manager.
	 *
	 *	Create a new Manager instance with the following interfaces:
	 *
	 *	-	UDPClientInterface
	 *	-	XpressNetClientInterface
	 *	-	WSClientInterface
	 *
	 *	then, create and start a DccGenerator
	 *
	 *	@param	none
	 *
	 *	@return	none, a new Manager instance is created
	 *
	 *	\warning	It is quite obvious that there should be just one and only one instance
	 *	of this class in the running application.
	 *	Maybe I implement it as a singleton, don't know for now...
	 */
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

	/**
	 *	Destructor for class Manager
	 *
	 *	Destroy the instance by first deleting the DccGenerator and then
	 *	deleting all clientInterfaces.
	 *
	 *	@param	none
	 *
	 *	@return	none, the Manager instance is deleted.
	 */
	Manager::~Manager()
	{
		delete m_pDccGenerator;

		for(auto client : m_ClientInterfaces)
		{
			delete client;
		}
	}

	/**
	 * Manager::findDecoder returns a pointer to the registered decoder with the
	 * specified DCC address.  If the decoder isn't found, return NULL.
	 *
	 * At this point we cannot create a new decoder instance because there is no
	 * way to determine what kind of decoder to create.  (class Decoder is virtual)
	 *
	 * @param	dccAddress:	the DCC address of the decoder to find
	 *
	 * @return	pointer to the decoder or NULL if the decoder doesn't appear in the
	 * list of registered decoders.
	 */
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

	/**
	 * Manager::registerDecoder registers (add) the decoder pointed to by pDecoder
	 * into the pool of decoders.  If there already exists a decoder with the same
	 * address in the pool, the existing pointer will be overwritten.
	 * \warning This situation can lead to a memoryleak.
	 *
	 * @param	pDecoder: pointer to the decoder to add into the pool
	 *
	 * @return	void
	 */
	void Manager::registerDecoder(Decoder* pDecoder)
	{
		lock_guard<recursive_mutex> guard(m_MDecoders);
		m_Decoders[pDecoder->getDCCAddress()] = pDecoder;
	}

	/**
	 * Manager::unregisterDecoder unregisters (remove) the decoder pointed to by pDecoder
	 * from the pool of decoders.  If the decoder wasn't prevoiusly registerd, i.e. it
	 * isn't in the pool, nothing changes.
	 *
	 * @param	pDecoder:	pointer to the decoder to remove from the pool
	 *
	 * @return	void
	 */
	void Manager::unregisterDecoder(Decoder* pDecoder)
	{
		lock_guard<recursive_mutex> guard(m_MDecoders);
		m_Decoders.erase(pDecoder->getDCCAddress());
	}


	/**
	 * Manager::broadcastLocInfoChanged itterates over the pool of interfaces
	 * and informs each of them about the change in the state of LocDecoder pointed to
	 * by pLoc.
	 *
	 * @param	pLoc:	pointer to LocDecoder who's state changed.
	 *
	 * @return	void
	 */
	void Manager::broadcastLocInfoChanged(LocDecoder* pLoc)
	{
		lock_guard<recursive_mutex> guard(m_MClientInterfaces);
		for(auto interface : m_ClientInterfaces)
		{
			interface->broadcastLocInfoChange(pLoc);
		}
	}

	/**
	 * Manager::setPowerState set the powerstate of the system to the new state indicated by
	 * newState and informs each interface about the new state.  In the case that newState is
	 * PowerOn, this function will cancel any pending emergencystop.
	 *
	 * @param	newState:	member of enum PowerState
	 *
	 * @return	void
	 */
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

	/**
	 * Manager::setEmergencyStop raises an emergencystop condition.  All locdecoders
	 * shall be stopped immediatly but the power will remain on the tracks.
	 *
	 * @param	none
	 *
	 * @return	void
	 */
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

	/**
	 * Manager::getSystemState generate the UDP message (Z21 LAN protocol)
	 * about the unit's systemstate.  The user is responsible for allocating a buffer
	 * big enough to contain the returned message.
	 *
	 * @param	pMsg	: pointer to a struct SystemState variable.
	 *
	 * @return	void
	 */
	void Manager::getSystemState(SystemState* pMsg)
	{
		lock_guard<recursive_mutex> guard(m_MSystemState);
		memcpy(pMsg, &m_SystemState, m_SystemState.DataLen);
		//	guard unlocked
	}

} /* namespace TBT */
