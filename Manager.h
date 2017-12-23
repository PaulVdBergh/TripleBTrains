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
 * Manager.h
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include <map>
#include <mutex>
#include <vector>
using namespace std;

#include "Types.h"

namespace TBT
{

	class ClientInterface;	//	forward declaration
	class Decoder;			//	forward declaration
	class LocDecoder;		//	forward declaration
	class DccGenerator;		//	forward declaration

	/// Class Manager represents 'the Boss' in the system.
	/**
	 * Class Manager is responsible for general supervision over the correct distribution
	 * of the different events through the whole system.  It collects and keeps state information
	 * regarding the overall state of the system.
	 */
	class Manager
	{
		public:
			///	class Manager constructor
			Manager();

			///	class Manager destructor
			virtual ~Manager();

			///	returns pointer to decoder with address dccAddress or NULL if decoder doesn't exists.
			Decoder*		findDecoder(uint16_t dccAddress);	//	return NULL if decoder doesn't exists
																//	we cannot known here which instance to create.

			///	registers (add) the decoder into the pool of decoders
			void			registerDecoder(Decoder* pDecoder);

			///	unregisters (remove) the decoder from the pool of decoders
			void			unregisterDecoder(Decoder* pDecoder);

			///	sends message to all interfaces about the locdecoder
			void			broadcastLocInfoChanged(LocDecoder* pLoc);

			///	sets the new powerstate and notifies all interfaces about the new state
			void 			setPowerState(PowerState newState);

			///	sets the emergency stop flag and notifies all interfaces about the new state
			void			setEmergencyStop();

			///	copies the systemstate into pMsg
			void 			getSystemState(SystemState* pMsg);

			///	returns reference to the centralState of the unit
			/**

			 <table>
			 <caption>CentralState layout</caption>
			 <tr><th>bit 7<th>bit 6<th>bit 5<th>bit 4<th>bit 3<th>bit 2<th>bit 1<th>bit 0
			 <tr><td>-<td>-<td>csProgrammingModeActive<td>-<td>-<td>csShortCircuit<td>cdTrackVoltageOff<td>csEmergencyStop
			 </table>

			 */
			const uint8_t& 	getCentralState(void) { return m_SystemState.CentralState; }

			/// returns reference to the centralStateEx of the unit
			/**

			 <table>
			 <caption>CentralStateEx layout</caption>
			 <tr><th>bit 7<th>bit 6<th>bit 5<th>bit 4<th>bit 3<th>bit 2<th>bit 1<th>bit 0
			 <tr><td>-<td>-<td>-<td>-<td>cseShortCircuitInternal<td>cseShortCircuitExternal<td>csePowerLost<td>cseHighTemperature
			 </table>

			 */
			const uint8_t& getCentralStateEx(void) { return m_SystemState.CentralStateEx; }

		protected:

		private:
			///	variable to hold the systemstate of the unit
			SystemState					m_SystemState;

			///	mutex to protect m_SystemState against concurrent access
			recursive_mutex				m_MSystemState;

			///	all the active clientinterfaces
			vector<ClientInterface*>	m_ClientInterfaces;

			///	mutex to protect m_ClientInterfaces against concurrent access
			recursive_mutex				m_MClientInterfaces;

			///	all the decoders in the system, indexed by DCC address
			map<uint16_t, Decoder*>		m_Decoders;

			///	mutex to protect m_Decoders against concurrent access
			recursive_mutex				m_MDecoders;

			///	pointer to the DCC generator
			DccGenerator*				m_pDccGenerator;

	};	/* class Manager */

} /* namespace TBT */

#endif /* MANAGER_H_ */
