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
 * UDPSocketInterface.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#include "UDPInterface.h"
#include "UDPClient.h"

#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace TBT
{

	UDPInterface::UDPInterface(Manager* pManager, const in_port_t& port /* = 21105 */)
	:	Interface(pManager)
	,	m_port(port)
	{
		int result;

		m_fdStop = eventfd(0, 0);
		m_fdsock_me = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		int optVal = 1;
		result = setsockopt(m_fdsock_me, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
		if (-1 == result)
		{
			exit(-1);
		}

		result = setsockopt(m_fdsock_me, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(optVal));
		if (-1 == result)
		{
			exit(-1);
		}

		memset(&m_sockaddr_me, 0, sizeof(sockaddr_in));
		m_sockaddr_me.sin_family = AF_INET;
		m_sockaddr_me.sin_port = htons(m_port);
		m_sockaddr_me.sin_addr.s_addr = htonl(INADDR_ANY);
		result = bind(m_fdsock_me, (sockaddr*)&m_sockaddr_me, sizeof(m_sockaddr_me));
		if (-1 == result)
		{
			exit(-1);
		}

		m_thread = thread([this]{ threadFunc(); });
	}

	UDPInterface::~UDPInterface()
	{
		uint64_t stop = 1;
		write(m_fdStop, &stop, sizeof(stop));
		m_thread.join();
		shutdown(m_fdsock_me, SHUT_RDWR);
		close(m_fdStop);
	}

	void UDPInterface::broadcastPowerStateChange(PowerState newState)
	{
		for(auto client : sm_Clients)
		{
			client->broadcastPowerStateChange(newState);
		}
	}

	UDPClient* UDPInterface::findClient(const sockaddr_in& address)
	{
		lock_guard<recursive_mutex> guard(sm_MClients);

		auto client = find_if(sm_Clients.begin(), sm_Clients.end(),
				[&address](UDPClient* p)
				{
					const sockaddr_in& otherAddress = p->getAddress();
					return ((otherAddress.sin_addr.s_addr == address.sin_addr.s_addr) && (otherAddress.sin_port == address.sin_port));
				}
				);

		if(sm_Clients.end() == client)
		{
			UDPClient* pClient = new UDPClient(this, address);
			sm_Clients.push_back(pClient);
			return pClient;
		}

		return *client;
	}

	bool UDPInterface::removeClient(UDPClient* pClient)
	{
		lock_guard<recursive_mutex> guard(sm_MClients);

		vector<UDPClient*>::iterator it = find(sm_Clients.begin(), sm_Clients.end(), pClient);
		if(sm_Clients.end() != it)
		{
			sm_Clients.erase(it);
			delete pClient;
			return true;
		}
		return false;
	}

	void UDPInterface::threadFunc()
	{
		epoll_event ev;
		epoll_event evlist[2];
		int epfd;
		bool bContinue = true;

		if(-1 == (epfd = epoll_create(2)))
		{
			//	TODO error handling
			return;
		}

		ev.data.fd = m_fdStop;
		ev.events = EPOLLIN;

		if(-1 == (epoll_ctl(epfd, EPOLL_CTL_ADD, m_fdStop, &ev)))
		{
			//	TODO error handling
			return;
		}

		memset(&ev, 0, sizeof(ev));
		ev.data.fd = m_fdsock_me;
		ev.events = EPOLLIN;

		if(-1 == (epoll_ctl(epfd, EPOLL_CTL_ADD, m_fdsock_me, &ev)))
		{
			//	TODO	error handling
			return;
		}

		while(bContinue)
		{
			int notifications = epoll_wait(epfd, evlist, 2, -1);
			if (-1 == notifications)
			{
				if (EINTR == notifications)
				{
					continue;
				}
				else
				{
					//	TODO : better error recovery !!
					bContinue = false;
					continue;
				}
			}

			for (int notification = 0; notification < notifications; notification++)
			{
				if (m_fdStop == evlist[notification].data.fd)
				{
					bContinue = false;
					continue;
				}
				else if (m_fdsock_me == evlist[notification].data.fd)
				{
					uint8_t recvbuffer[128];
					struct sockaddr_in si_other;
					socklen_t slen = sizeof(si_other);

					int recv_len = recvfrom(m_fdsock_me, recvbuffer, 128, 0, (sockaddr*)&si_other, &slen);
					UDPClient* pClient = findClient(si_other);
					uint8_t* payload = recvbuffer;
					while(payload < (recvbuffer + recv_len))
					{
						switch (*(uint32_t*)payload)
						{

							case 0x00100004: //  LAN_GET_SERIAL_NUMBER
							{
								static const uint8_t LAN_SERIAL_NUMBER[] = {0x08, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
								sendto(m_fdsock_me, LAN_SERIAL_NUMBER, LAN_SERIAL_NUMBER[0], 0, (sockaddr*)&si_other, sizeof(si_other));
								break;
							}

							case 0x00180004: //  LAN_GET_CODE
							{
								static const uint8_t LAN_CODE[] = {0x05, 0x00, 0x18, 0x00, 0x00};
								sendto(m_fdsock_me, LAN_CODE, LAN_CODE[0], 0, (sockaddr*)&si_other, sizeof(si_other));
								break;
							}

							case 0x001A0004: //  LAN_GET_HWINFO
							{
								const uint8_t LAN_HWINFO[] = {0x0C, 0x00, 0x1A, 0x00, 0x01, 0x02, 0x00, 0x00, 0x30, 0x01, 0x00, 0x00};
								sendto(m_fdsock_me, LAN_HWINFO, LAN_HWINFO[0], 0, (sockaddr*)&si_other, sizeof(si_other));
								break;
							}

							case 0x00300004: //  LAN_LOGOFF
							{
								removeClient(pClient);
								break;
							}

							default:
							{
								static const uint8_t LAN_X_UNKNOWN_COMMAND[] = {0x07, 0x00, 0x40, 0x00, 0x61, 0x82, 0xE3};
								sendto(m_fdsock_me, LAN_X_UNKNOWN_COMMAND, LAN_X_UNKNOWN_COMMAND[0], 0, (sockaddr*)&si_other, sizeof(si_other));
								break;
							}

						}	/*	switch(*(uint32_t*)payload)	*/
						payload += payload[0];	//	advance payload pointer to next message
					}	/*	while(payload <(recv...	*/
				}	/* if(m_fdsock_me == evlist..... */
			}	/* for(int notifications.... */

		}	/* while(bContinue) */
	}	/*	threadFunc(void)	*/

	vector<UDPClient*> UDPInterface::sm_Clients;
	recursive_mutex UDPInterface::sm_MClients;

} /* namespace TBT */
