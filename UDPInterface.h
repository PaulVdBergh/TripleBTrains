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
 * UDPInterface.h
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#ifndef UDPINTERFACE_H_
#define UDPINTERFACE_H_

#include "Interface.h"

#include <arpa/inet.h>
#include <vector>
#include <mutex>
#include <thread>

using namespace std;

namespace TBT
{
	class UDPClient;

	class UDPInterface: public Interface
	{
		public:
			UDPInterface(Manager* pManager, const in_port_t& port = 21105);
			virtual ~UDPInterface();

			virtual void broadcastPowerStateChange(PowerState newState);

		protected:
			UDPClient* 	findClient(const sockaddr_in& address);
			bool		removeClient(UDPClient* pClient);

		private:
			void threadFunc(void);

			in_port_t	m_port;
			thread		m_thread;
			sockaddr_in m_sockaddr_me;
			int			m_fdsock_me;
			int			m_fdStop;

			static vector<UDPClient*>	sm_Clients;
			static recursive_mutex		sm_MClients;

	};	/* class UDPInterface */

} /* namespace TBT */

#endif /* UDPINTERFACE_H_ */
