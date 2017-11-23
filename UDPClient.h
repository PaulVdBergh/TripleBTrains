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
 * UDPClient.h
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#ifndef UDPCLIENT_H_
#define UDPCLIENT_H_

#include "Client.h"
#include "UDPInterface.h"

namespace TBT
{

	class UDPClient: public Client
	{
		public:
			UDPClient(UDPInterface* pinterface, const sockaddr_in& address);
			virtual ~UDPClient();

			virtual void broadcastPowerStateChange(PowerState newState);

			const sockaddr_in& getAddress(void) { return m_Address; }

		protected:
			const sockaddr_in	m_Address;

		private:

	};	/*	class UDPClient */

} /* namespace TBT */

#endif /* UDPCLIENT_H_ */
