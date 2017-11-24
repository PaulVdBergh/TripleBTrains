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

	class Manager
	{
		public:
			Manager();
			virtual ~Manager();

			Decoder*		findDecoder(uint16_t dccAddress);	//	return NULL if decoder doesn't exists
			void			registerDecoder(Decoder* pDecoder);

			void			broadcastLocInfoChanged(LocDecoder* pLoc);

			void 			setPowerState(PowerState newState);

			void 			getSystemState(SystemState* pMsg);
			const uint8_t& 	getCentralState(void) { return m_SystemState.CentralState; }

		protected:

		private:
			SystemState		m_SystemState;
			recursive_mutex	m_MSystemState;

			vector<ClientInterface*>	m_ClientInterfaces;
			recursive_mutex				m_MClientInterfaces;

			map<uint16_t, Decoder*>		m_Decoders;
			recursive_mutex				m_MDecoders;

	};	/* class Manager */

} /* namespace TBT */

#endif /* MANAGER_H_ */
