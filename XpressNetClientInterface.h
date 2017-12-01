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
 * XpressNetClientInterface.h
 *
 *  Created on: Nov 24, 2017
 *      Author: paul
 */

#ifndef XPRESSNETCLIENTINTERFACE_H_
#define XPRESSNETCLIENTINTERFACE_H_

#include "ClientInterface.h"

#include <thread>

namespace TBT
{

	class XpressNetClient;	//	forward declaration

	class XpressNetClientInterface: public ClientInterface
	{
		public:
			XpressNetClientInterface(Manager* pManager, const char* ttyPath = "/dev/ttyO1");
			virtual ~XpressNetClientInterface();

			//	Virtuals
			virtual void broadcastPowerStateChange(bool newState);
			virtual void broadcastLocInfoChange(LocDecoder* pLoc);
			virtual void broadcastEmergencyStop(void);

		protected:
			XpressNetClient*	findClient(const uint8_t& address);	//	if client doesn't exists, it's created.

		private:
			void threadFunc(void);

			char*	m_pTtyPath;
			int		m_fdSerial;
			int		m_fdStop;
			thread	m_thread;

			map<uint8_t, XpressNetClient*>	m_Clients;
			recursive_mutex m_MClients;

	};	/*	class XpressNetClientInterface	*/

} /* namespace TBT */

#endif /* XPRESSNETCLIENTINTERFACE_H_ */
