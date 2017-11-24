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
 * Client.h
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "ClientInterface.h"

namespace TBT
{

	class Client
	{
		public:
			Client(ClientInterface* pInterface);
			virtual ~Client();

			ClientInterface* getInterface(void) { return m_pInterface; }

			//	pure virtuals
			virtual void broadcastPowerStateChange(PowerState newState) = 0;
			virtual void broadcastLocInfoChanged(LocDecoder* pLoc) = 0;
			virtual void broadcastEmergencyStop(bool state) = 0;

		protected:
			ClientInterface*	m_pInterface;

		private:

	};	/* class Client */

} /* namespace TBT */

#endif /* CLIENT_H_ */
