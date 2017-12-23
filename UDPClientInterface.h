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

	/**
	 * Class UDPClientInterface represents the ClientInterface for UDP clients
	 * using the Z21 Lan Protocol.
	 *
	 * # Basics
	 * ## Communication:
	 * Communication with the Z21 protocol over UDP port 21105 or 21106.
	 * Control applications on the client (PC, App, ...) should primarily use port 21105.
	 * The communication is always asynchronous, i.e. between a request and the
	 * corresponding response other broadcast messages can occur.
	 *
	 * It is expected that each client communicates with the Z21 once per minute, otherwise
	 * it will be removed from the list of active participants. If possible, a client
	 * should log off when exiting with the LAN_LOGOFF command.
	 *
	 * ## Z21 Datagram layout
	 * A Z21 datagram, i.e. a request or response is structured as follows:
	 *
	 <table>
	 <caption>Z21 Datagram Layout</caption>
	 <tr><th>DataLen<th>Header<th>Data
	 <tr><td>2 Bytes<td>2 Bytes<td>n Bytes
	 </table>
	 *	- <b>DataLen</b> (little endian):
	 *
	 *		Total length over the entire data set including DataLen, Header and Data,
	 *		i.e. DataLen = 2 + 2 + n.
	 *	- <b>Header</b> (little endian):
	 *
	 *		Describes the command or protocol group.
	 *	- <b>Data</b>:
	 *
	 *		Structure and length depend on command. Exact description see respective command.
	 *
	 * Unless otherwise stated, the byte order is little-endian, i.e. first the low byte, then
	 * the high byte.
	 *
	 */
	class UDPClientInterface: public ClientInterface
	{
		public:
			/// UDPClientInterface constructor
			UDPClientInterface(Manager* pManager, const in_port_t& port = 21105);

			/// UDPClientInterface destructor
			virtual ~UDPClientInterface();

			/**
			 * returns handle to the socket of the client.
			 *
			 * UDPClientInterface::getMySocket returns the handle to the socket on which to
			 * send responses to the client.
			 *
			 * @param	none
			 *
			 * @return	int : handle to socket on wich to send responses to.
			 */
			int				getMySocket(void) { return m_fdsock_me; }

			///	sends a notification to all clients about a powerstate change.
			virtual void 	broadcastPowerStateChange(bool newState);

			///	sends a notification to all clients about the locdecoder.
			virtual void 	broadcastLocInfoChange(LocDecoder* pLoc);

			///	sends a notification to all clients about an emergencystop.
			virtual void	broadcastEmergencyStop(void);

		protected:
			/// returns pointer to client with specified address.
			UDPClient* 		findClient(const sockaddr_in& address);

			/// removes client with specified address and destroys it.
			bool			removeClient(UDPClient* pClient);

		private:
			/// does the actual work in a separate thread.
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
