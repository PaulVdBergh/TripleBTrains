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
 * WSClientInterface.h
 *
 *  Created on: Nov 25, 2017
 *      Author: paul
 */

#ifndef WSCLIENTINTERFACE_H_
#define WSCLIENTINTERFACE_H_

#include "ClientInterface.h"

#include "mongoose.h"

#include <thread>
using namespace std;

namespace TBT
{

#ifndef WS_PORT
#define WS_PORT "9030"
#endif

	class WSClientInterface: public ClientInterface
	{
		public:
			WSClientInterface(Manager* pManager, const char* port = WS_PORT);
			virtual ~WSClientInterface();

			virtual void	broadcastPowerStateChange(bool newState);
			virtual void 	broadcastLocInfoChange(LocDecoder* pLoc);
			virtual void	broadcastEmergencyStop(void);
			virtual void	broadcastOvercurrent(void);

		protected:
			char*			m_port;
			volatile bool	m_bContinue;
			thread			m_thread;
			mg_mgr			m_mgr;
			mg_connection*	m_conn;

		private:
			void			threadFunc(void);

	};	/* class WSClientInterface */

	//	Mongoose callback functions
	void ev_handler(mg_connection* nc, int ev, void* ev_data);
	void ev_handler_StateChange(mg_connection* nc, int ev, void* ev_data);


} /* namespace TBT */

#endif /* WSCLIENTINTERFACE_H_ */
