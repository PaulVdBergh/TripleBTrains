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
 * WSClientInterface.cpp
 *
 *  Created on: Nov 25, 2017
 *      Author: paul
 */

#include "WSClientInterface.h"

#include <string.h>

namespace TBT
{

	WSClientInterface::WSClientInterface(Manager* pManager, const char* port)
	:	ClientInterface(pManager)
	,	m_port(new char[strlen(port)] + 1)
	,	m_bContinue(true)
	{
		strcpy(m_port, port);

		m_thread = thread([this]{ threadFunc(); });
	}

	WSClientInterface::~WSClientInterface()
	{
		m_bContinue = false;
		m_thread.join();
		delete [] m_port;
	}

	void WSClientInterface::broadcastPowerStateChange(bool newState)
	{
		//	if newState == true --> Track Power On --> PowerState --> Off
		char msg[256];
		sprintf(msg, "{ \"PowerState\":\"%s\" }", newState ? "OFF" : "ON");

		mg_broadcast(&m_mgr, ev_handler_StateChange, msg, strlen(msg) + 1);

		if(newState)
		{
			sprintf(msg, "{\"EmergencyStop\":\"OFF\" }");
			mg_broadcast(&m_mgr, ev_handler_StateChange, msg, strlen(msg) + 1);
		}
	}

	void WSClientInterface::broadcastLocInfoChange(LocDecoder* pLoc)
	{

	}

	void WSClientInterface::broadcastAccessoryInfoChanged(Accessory* pAccessory)
	{

	}

	void WSClientInterface::broadcastEmergencyStop()
	{
		char msg[] = "{ \"EmergencyStop\":\"ON\" }";

		mg_broadcast(&m_mgr, ev_handler_StateChange, msg, strlen(msg) + 1);
	}

	void WSClientInterface::broadcastOvercurrent()
	{
		char msg[] = "{ \"Overcurrent\":\"ON\" }";

		mg_broadcast(&m_mgr, ev_handler_StateChange, msg, strlen(msg) + 1);
	}

	void WSClientInterface::threadFunc(void)
	{
		mg_mgr_init(&m_mgr, NULL);

		if(NULL == (m_conn = mg_bind(&m_mgr, m_port, ev_handler)))
		{
			//	TODO error recovery
		}
		else
		{
			mg_set_protocol_http_websocket(m_conn);

			while(m_bContinue)
			{
				mg_mgr_poll(&m_mgr, 200);
			}
		}
		mg_mgr_free(&m_mgr);
	}

	void ev_handler(mg_connection* nc, int ev, void* ev_data)
	{
		switch(ev)
		{
			case MG_EV_WEBSOCKET_FRAME:
			{
				char address[32];
				mg_sock_addr_to_str(&nc->sa, address, sizeof(address), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
				websocket_message* pMsg = (websocket_message*)ev_data;
				printf("WS : received %i bytes from %s : %.*s\n", pMsg->size, address, pMsg->size, pMsg->data);
				//	TODO process messages here
			}
		}	/* switch(ev) */
	}

	void ev_handler_StateChange(mg_connection* nc, int ev, void* ev_data)
	{
		switch(ev)
		{
			case MG_EV_POLL:
			{
				mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, ev_data, strlen((char*)ev_data));
			}
		}
	}

} /* namespace TBT */
