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
 * Interface.h
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#ifndef CLIENTINTERFACE_H_
#define CLIENTINTERFACE_H_

#include "Manager.h"

namespace TBT
{

	class LocDecoder;	//	forward declaration

	class ClientInterface
	{
		public:
			ClientInterface(Manager* pManager);
			virtual ~ClientInterface();

			Manager* getManager(void) { return m_pManager; }

			//	pure virtuals
			virtual void broadcastPowerStateChange(PowerState newState) = 0;
			virtual void broadcastLocInfoChanged(LocDecoder* pLoc) = 0;

		protected:
			Manager*	m_pManager;

		private:

	};	/* class Interface */

} /* namespace TBT */

#endif /* CLIENTINTERFACE_H_ */
