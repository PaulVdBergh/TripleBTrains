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

#ifndef UDPCLIENTINTERFACE_H_
#define UDPCLIENTINTERFACE_H_

#include <arpa/inet.h>
#include <vector>
#include <mutex>
#include <thread>
#include "ClientInterface.h"

using namespace std;

namespace TBT
{
	class UDPClient;
	class LocDecoder;

	class UDPClientInterface: public ClientInterface
	{
		public:
			UDPClientInterface(Manager* pManager, const in_port_t& port = 21105);
			virtual ~UDPClientInterface();

			int				getMySocket(void) { return m_fdsock_me; }

			virtual void 	broadcastPowerStateChange(PowerState newState);
			virtual void 	broadcastLocInfoChanged(LocDecoder* pLoc);
			virtual void	broadcastEmergencyStop(bool state);

		protected:
			UDPClient* 		findClient(const sockaddr_in& address);	//	if client doesn't exists, it's created.
			bool			removeClient(UDPClient* pClient);

		private:
			void threadFunc(void);

			in_port_t	m_port;
			thread		m_thread;
			sockaddr_in m_sockaddr_me;
			int			m_fdsock_me;
			int			m_fdStop;

			vector<UDPClient*>	sm_Clients;
			recursive_mutex		sm_MClients;

	};	/* class UDPInterface */

} /* namespace TBT */

#endif /* UDPCLIENTINTERFACE_H_ */
