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
 * XpressNetClient.h
 *
 *  Created on: Nov 24, 2017
 *      Author: paul
 */

#ifndef XPRESSNETCLIENT_H_
#define XPRESSNETCLIENT_H_

#include "Client.h"

namespace TBT
{

	class XpressNetClient: public Client
	{
		public:
			XpressNetClient(ClientInterface* pInterface, uint8_t XpressNetAddress);
			virtual ~XpressNetClient();

			const uint8_t& getAddress(void) { return m_XpressNetAddress; }

			//	Virtuals
			virtual void broadcastPowerStateChange(PowerState newState);
			virtual void broadcastLocInfoChanged(LocDecoder* pLoc);
			virtual void broadcastEmergencyStop(bool state) {};

		protected:
			uint8_t	m_XpressNetAddress;

		private:

	};	/*	class XpressNetClient	*/

} /* namespace TBT */

#endif /* XPRESSNETCLIENT_H_ */
