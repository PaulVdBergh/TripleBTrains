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

#include "UDPClient.h"

#include "LocDecoder.h"

#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "UDPClientInterface.h"

namespace TBT
{

	/**
	 * The constructor of UDPClientInterface
	 * 	- initializes its base class
	 * 	- sets up an event file descriptor (m_fdStop) which is responsible for a clean
	 * 	shutdown of the worker thread.
	 * 	- sets up a socket file descriptor (m_fdsock_me) and initializes it for listening
	 * 	for incomming Z21 portocol datagrams.
	 * 	- sets up a worker thread and start the threadFunc to listen to incomming datagrams.
	 *
	 * @param	pManager : pointer to the Manager instance.
	 * @param	port : The port on which to listen for incomming messages.
	 *
	 * @return	none : a new instance of type class UDPClientInterface is constructed.
	 */
	UDPClientInterface::UDPClientInterface(Manager* pManager, const in_port_t& port /* = 21105 */)
	:	ClientInterface(pManager)
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

	/**
	 * The destructor of UDPClientInterface
	 * - informs the worker thread to finish processing of incomming datagrams.
	 * - waits for the worker thread to finish.
	 * - cleans up open file descriptors
	 */
	UDPClientInterface::~UDPClientInterface()
	{
		uint64_t stop = 1;
		write(m_fdStop, &stop, sizeof(stop));
		m_thread.join();
		shutdown(m_fdsock_me, SHUT_RDWR);
		close(m_fdStop);
	}

	/**
	 * UDPClientInterface::broadcastPowerStateChange informs each client about
	 * a power state change.
	 *
	 * @param newState : bool true if the power is applied to the track, false otherwise.
	 *
	 * @return void
	 */
	void UDPClientInterface::broadcastPowerStateChange(bool newState)
	{
		lock_guard<recursive_mutex> guard(sm_MClients);

		for(auto client : sm_Clients)
		{
			client->broadcastPowerStateChange(newState);
		}
	}

	/**
	 * UDPClientInterface::broadcastLocInfoChange informs each client about
	 * changes in the state of a LocDecoder.
	 *
	 * @param pLoc : pointer to the LocDecoder instance whoes state changed.
	 *
	 * @return void
	 */
	void UDPClientInterface::broadcastLocInfoChange(LocDecoder* pLoc)
	{
		lock_guard<recursive_mutex> guard(sm_MClients);

		for(auto client : sm_Clients)
		{
			client->broadcastLocInfoChanged(pLoc);
		}
	}

	/**
	 * UDPClinetInterface::broadcastEmergencyStop informs each client about
	 * the raise of an emergency stop.
	 *
	 * @param none
	 *
	 * @return void
	 */
	void UDPClientInterface::broadcastEmergencyStop()
	{
		lock_guard<recursive_mutex> guard(sm_MClients);

		for(auto client : sm_Clients)
		{
			client->broadcastEmergencyStop();
		}
	}

	/**
	 * UDPClientInterface::findClient returns a pointer to an UDPClient instance
	 * at the specified address.  If the client doesn't exists, it is created.
	 *
	 * @param address : the socket address on which the client is addressed.
	 *
	 * @return a pointer to the instance of the UDPClient.
	 */
	UDPClient* UDPClientInterface::findClient(const sockaddr_in& address)
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

	/**
	 * UDPClientInterface::removeClient removes a client from the pool of UDPClients and
	 * destroys the client if it exists.
	 *
	 * @param	pClient : pointer to instance of class UDPClient to remove.
	 *
	 * @return
	 * 			- true if client is removed,
	 * 			- false if client didn't exists.
	 */
	bool UDPClientInterface::removeClient(UDPClient* pClient)
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

	/**
	 * UDPClientInterface::threadFunc runs in a separate thread.  The function listen on the
	 * socket (m_fdsock_me) for incomming datagrams.
	 */
	void UDPClientInterface::threadFunc()
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
						printf("UDP : received %i bytes from %s:%i : ", *payload, inet_ntoa(si_other.sin_addr), si_other.sin_port);

						switch (*(uint32_t*)payload)
						{

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_GET_SERIAL_NUMBER
							 * read the serial number from the Z21
							 *
							 <table>
							 <caption id="multi_row">request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x04<td>0x00<td>0x10<td>0x00<td>--
							 </table>
							 <table>
							 <caption>response:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x08<td>0x00<td>0x10<td>0x00<td>Serialnumber (32 bit, little endian)
							 </table>
							 */
							case 0x00100004:
							{
								printf("LAN_GET_SERIAL_NUMBER.");
								static const uint8_t LAN_SERIAL_NUMBER[] = {0x08, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
								sendto(m_fdsock_me, LAN_SERIAL_NUMBER, LAN_SERIAL_NUMBER[0], 0, (sockaddr*)&si_other, sizeof(si_other));
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_GET_CODE
							 * With this command, the SW feature scope of the Z21 can be checked and read out.
							 * This command is of particular interest for the hardware version "z21 start" in order
							 * to check whether driving and switching via LAN is blocked or allowed.

							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Header<th>Data
							 <tr><td>0x04<td>0x00<td>0x18<td>0x00<td>-
							 </table>

							 <table>
							 <caption>Response:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Header<th>Data
							 <tr><td>0x05<td>0x00<td>0x18<td>0x00<td>Code (8 Bit)
							 </table>


							 <table>
							 <caption>Code layout</caption>
							 <tr><th>define<th>Value<th>Meaning
							 <tr><td>Z21_NO_LOCK<td>0x00<td>No features locked
							 <tr><td>Z21_START_LOCKED<td>0x01<td>"z21 start" : Driving and switching disabled via LAN.
							 <tr><td>Z21_START_UNLOCKED<td>0x02<td>"z21 start" : all feature locks are removed
			 	 	 	 	 </table>
							 */
							case 0x00180004:
							{
								printf("LAN_GET_CODE");
								static const uint8_t LAN_CODE[] = {0x05, 0x00, 0x18, 0x00, 0x00};
								sendto(m_fdsock_me, LAN_CODE, LAN_CODE[0], 0, (sockaddr*)&si_other, sizeof(si_other));
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_GET_HWINFO
							 * With this command the hardware type and the firmware version of the Z21 can be read out.
							 *
							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Header<th>Data
							 <tr><td>0x04<td>0x00<td>0x1A<td>0x00<td>-
							 </table>

							 <table>
							 <caption>Response:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Header<th colspan="2">Data
							 <tr><td>0x0C<td>0x00<td>0x1A<td>0x00<td>HwType 32 Bit(little endian)<td>FW Version 32 Bit (little endian)
							 </table>


							 <table>
							 <caption>HwType 32 Bit</caption>
							 <tr><th>define<th>Value<th>Meaning
							 <tr><td>D_HWT_Z21_OLD<td>0x00000200<td>"Black Z21" (Hardware-version from 2012)
							 <tr><td>D_HWT_Z21_NEW<td>0x00000201<td>"Black Z21" (Hardware-version from 2013)
							 <tr><td>D_HWT_SMARTRAIL<td>0x00000202<td>"SmartRail" (from 2012)
							 <tr><td>D_HWT_z21_SMALL<td>0x00000203<td>"White z21" Starterset-version (from 2013)
							 <tr><td>D_HWT_z21_START<td>0x00000204<td>"z21 start" Starterset-version (from 2016)
			 	 	 	 	 </table>

			 	 	 	 	 * The FW version is specified in BCD format.
							 */
							case 0x001A0004: //  LAN_GET_HWINFO
							{
								printf("LAN_GET_HWINFO");
								static const uint8_t LAN_HWINFO[] = {0x0C, 0x00, 0x1A, 0x00, 0x01, 0x02, 0x00, 0x00, 0x30, 0x01, 0x00, 0x00};
								sendto(m_fdsock_me, LAN_HWINFO, LAN_HWINFO[0], 0, (sockaddr*)&si_other, sizeof(si_other));
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_LOGOFF
							 * Log off this client from the system
							 *
							 <table>
							 <caption>request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x04<td>0x00<td>0x30<td>0x00<td>--
							 </table>
							 *
							 * response:
							 * none
							 *
							 * When logging out, use the same port number as when logging in.
							 *
							 * <b>Note</b>: logging in occurs implicitly with the client's first command (e.g.
							 * LAN_SYSTEM_STATE_GETDATA, ...).
							 */
							case 0x00300004:
							{
								printf("LAN_LOGOFF");
								removeClient(pClient);
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_GET_BROADCASTFLAGS
							 * Reading out the broadcast flags in the Z21.
							 *
							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x04<td>0x00<td>0x51<td>0x00<td>--
							 </table>

							 <table>
							 <caption>Response:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x08<td>0x00<td>0x51<td>0x00<td>Broadcast-Flags (32 Bit little endian)
							 </table>

							 * Broadcast-Flags : see UDPClient::getBroadcastFlags
							 */
							case 0x00510004:
							{
								printf("LAN_GET_BROADCASTFLAGS");
								static uint8_t LAN_BROADCASTFLAGS[] = {0x08, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00};
								*((uint32_t*)(&LAN_BROADCASTFLAGS[4])) = pClient->getBroadcastFlags();
								sendto(m_fdsock_me, LAN_BROADCASTFLAGS, LAN_BROADCASTFLAGS[0], 0, (struct sockaddr*)&si_other, sizeof(si_other));
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_SYSTEMSTATE_GETDATA
							 * Request the current system state.
							 *
							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x04<td>0x00<td>0x85<td>0x00<td>-
							 </table>

							 * response : see TBT::SystemState
							 */
							case 0x00850004: //  LAN_SYSTEMSTATE_GETDATA
							{
								printf("LAN_SYSTEMSTATE_GETDATA");
								SystemState state;
								pClient->getInterface()->getManager()->getSystemState(&state);
								sendto(m_fdsock_me, &state, sizeof(struct SystemState), 0, (struct sockaddr*)&si_other, sizeof(si_other));
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_LOCONET_FROM_LAN
							 * <b>From Z21 FW version 1.20.</b>
							 *
							 * With this message, a LAN client can write a message to the LocoNet bus.
							 *
							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td rowspan="2">0x04 + n<td rowspan="2">0x00<td rowspan="2">0xA2<td rowspan="2">0x00<th>LocoNet message incl. CKSUM
							 <tr><td>n Bytes
							 </table>
							 */
							case 0x00A20004: //  LAN_LOCONET_FROM_LAN
							{
								printf("LAN_LOCONET_FROM_LAN");
								//	TODO implementation
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_RMBUS_GETDATA
							 *
							 * Request the current status of the feedback.
							 *
							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x05<td>0x00<td>0x81<td>0x00<td>Group index (1 Byte)
							 </table>

							 <table>
							 <caption>Response:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="2">Data
							 <tr><td>0x0F<td>0x00<td>0x80<td>0x00<td>Group index (1 Byte)<td>Feedback status (10 bytes)
							 </table>

							 <b>Group index :</b>
							  - 0 --> Feedback status for address from  1 to 10
							  - 1 --> Feedback status for address from 11 to 20
							  -
							  -


							  <b>Feedback status :</b>
							   - 1 byte per feedback,
							   - 1 bit per input.

							   The assignment of feedback address and byte position is statically ascending.


							   <b>Example:</b>
							   GroupIndex = 1 and feedback status = 0x01 0x00 0xC5 0x00 0x00 0x00 0x00 0x00 0x00 0x00
							   means "feedback 11, contact input 1; Feedback 13, contact input 8,7,3 and 1 "
							 */
							case 0x00810005: //  LAN_RMBUS_GETDATA
							{
								printf("LAN_RMBUS_GETDATA");
								//	TODO implementation
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_RMBUS_PROGRAMMODULE
							 *
							 * Change the feedback address.
							 *
							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x05<td>0x00<td>0x82<td>0x00<td>Address (1 Byte)
							 </table>

							 <b>Address:</b>
							 new address for the feedback module to be programmed.
							 Supported value range: 0 and 1 ... 20.

							 Response: none

							 \warning
							 The programming command is output on the R-BUS until this command is sent again to Z21 with address = 0.
							 During the programming process, no other feedback module may be connected on the R-BUS.
							 *
							 */
							case 0x00820005: //  LAN_RMBUS_PROGRAMMODULE
							{
								printf("LAN_RMBUS_PROGRAMMODULE");
								//	TODO implementation
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_GET_LOCOMODE
							 *
							 * Read the output format for a given locomotive address.
							 *
							 * The output format (DCC, MM) can be stored persistently per locomotive address in the Z21.
							 * A maximum of 256 different locomotive addresses can be stored.
							 * Each address> = 256 is automatically DCC.
							 *
							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x06<td>0x00<td>0x60<td>0x00<td>Loc-Address (16 Bit <b>big endian</b>)
							 </table>

							 <table>
							 <caption>Response:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="2">Data
							 <tr><td>0x07<td>0x00<td>0x60<td>0x00<td>Loc-Address (16 Bit <b>big endian</b>)<td>Modus (8 bit)
							 </table>

							 Modus : see LocDecoder::getLocMode
							 */
							case 0x00600006: //  LAN_GET_LOCOMODE
							{
								printf("LAN_GET_LOC0MODE");
								Manager* pManager = pClient->getInterface()->getManager();
								uint16_t locAddress = (payload[4] << 8) + payload[5];
								Decoder* pDecoder = pManager->findDecoder(locAddress);
								if (pDecoder == NULL)
								{
									pDecoder = new LocDecoder(pManager, locAddress);
								}
								LocDecoder* pLoc = dynamic_cast<LocDecoder*>(pDecoder);
								if (pLoc)
								{
									uint8_t locMode[] = { 0x07, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00};
									locMode[4] = pLoc->getDCCAddress() >> 8;
									locMode[5] = pLoc->getDCCAddress() & 0xFF;
									locMode[6] = pLoc->getLocMode();
									sendto(m_fdsock_me, locMode, locMode[0], 0, (struct sockaddr*)&si_other, sizeof(si_other));
								}
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_X_SET_STOP
							 * With the following command the emergency stop is raised.  All Locs goes into
							 * speedstep 01, but the power is still feed to the rails.
							 *
							 <table>
							 <caption>request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="2">Data
							 <tr><td rowspan="2">0x06<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>XOR-Byte
							 <tr><td>0x80<td>0x80
							 </table>

							 response : see UDPClient::broadcastEmergencyStop
							 */
							case 0x00400006:
							{
								printf("LAN_X_SET_STOP");
								if(payload[4] == 0x80)
								{
									pClient->getInterface()->getManager()->setEmergencyStop();
								}
								break;
							}

							/** @ingroup Z21LANProtocol
							 * ---
							 * ### LAN_GET_TURNOUTMODE
							 * Reading the settings for a given function decoder address ("Function Decoder"
							 * in the sense of "Accessory Decoder" RP-9.2.1).
							 * The output format (DCC, MM) can be stored persistently per function decoder
							 * address in the Z21. A maximum of 256 different
							 * function decoder addresses can be stored. Each Address> = 256 is automatically DCC.
							 *
							 <table>
							 <caption>Request:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th>Data
							 <tr><td>0x06<td>0x00<td>0x70<td>0x00<td>Function Decoder Address (16 Bit <b>big endian</b>)
							 </table>

							 <table>
							 <caption>Response:</caption>
							 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="2">Data
							 <tr><td>0x07<td>0x00<td>0x70<td>0x00<td>Function Decoder Address (16 Bit <b>big endian</b>)<td>Modus (8 Bit)
							 </table>

							 Modus : see FunctionDecoder::getMode

							 At the LAN interface and in the Z21, the function decoder addresses are addressed from 0, in the visualization
							 on the apps or on the multiMaus, however, from 1. This is merely a decision of the visualization. Example:
							 multiMaus switch address # 3, corresponds on the LAN and Z21 to address # 2.
							 */
							case 0x00700006: //  LAN_GET_TURNOUTMODE
							{
								printf("LAN_GET_TURNOUTMODE");
								//	TODO implementation
								break;
							}

							case 0x00A30006: //  LAN_LOCONET_DISPATCH_ADDR
							{
								printf("LAN_LOCONET_DISPATCH_ADDR");
								//	TODO implementation
								break;
							}

							case 0x00400007:
							{
								switch (*(uint16_t*)&payload[4])
								{
									/** @ingroup Z21LANProtocol
									 * ---
									 * ### LAN_X_GET_VERSION
									 * With the following command the X-Bus version of the Z21 can be read out.
									 *
									 <table>
									 <caption>request:</caption>
									 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="3">Data
									 <tr><td rowspan="2">0x07<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>XOR-Byte
									 <tr><td>0x21<td>0x21<td>0x00
									 </table>

									 <table>
									 <caption>response:</caption>
									 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="5">Data
									 <tr><td rowspan="2">0x09<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>DB1<th>DB2<th>XOR-Byte
									 <tr><td>0x63<td>0x21<td>0x30<td>0x12<td>0x60
									 </table>

									 - DB1 --> X-Bus Version (3.0)
									 - DB2 --> ID (Z21 = 0x12)
									 */
									case 0x2121: //  LAN_X_GET_VERSION
									{
										printf("LAN_X_GET_VERSION");
										if (payload[6] == 0x00) //  xor check
										{
											const uint8_t LAN_X_VERSION[] = {0x09, 0x00, 0x40, 0x00, 0x63, 0x21, 0x30, 0x12, 0x60};
											sendto(m_fdsock_me, LAN_X_VERSION, LAN_X_VERSION[0], 0, (struct sockaddr*)&si_other, sizeof(si_other));
										}
										break;
									}

									/** @ingroup Z21LANProtocol
									 * ---
									 * ### LAN_X_GET_STATUS
									 * with the following command the system's status can be read out.
									 *
									 <table>
									 <caption>request:</caption>
									 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="3">Data
									 <tr><td rowspan="2">0x07<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>XOR-Byte
									 <tr><td>0x21<td>0x24<td>0x05
									 </table>

									 <table>
									 <caption>response:</caption>
									 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="4">Data
									 <tr><td rowspan="2">0x08<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>DB1<th>XOR-Byte
									 <tr><td>0x62<td>0x22<td>CentralState<td>XOR-Byte
									 </table>

									 The system status is identical to the centralState, which is supplied in SystemStatus.

									 @see Manager::getCentralState
									 */
									case 0x2421: //  LAN_X_GET_STATUS
									{
										printf("LAN_X_GET_STATUS");
										if (payload[6] == 0x05)
										{
											uint8_t LAN_X_STATUS[] = {0x08, 0x00, 0x40, 0x00, 0x62, 0x22, 0x00, 0x00};
											LAN_X_STATUS[6] = pClient->getInterface()->getManager()->getCentralState();
											for (uint8_t i = 4; i < 7; i++)
											{
												LAN_X_STATUS[7] ^= LAN_X_STATUS[i];
											}
											sendto(m_fdsock_me, LAN_X_STATUS, LAN_X_STATUS[0], 0, (struct sockaddr*)&si_other, sizeof(si_other));
										}
										break;
									}

									/** @ingroup Z21LANProtocol
									 * ---
									 * ### LAN_X_SET_TRACK_POWER_OFF
									 * With the following command the trackpower is switched off.
									 *
									 <table>
									 <caption>request:</caption>
									 <tr><th colspan="2">DataLen<th colspan="2">Header<th colspan="3">Data
									 <tr><td rowspan="2">0x07<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>XOR-Byte
									 <tr><td>0x21<td>0x80<td>0xA1
									 </table>

									 response: see UDPClient::broadcastPowerStateChange
									 */
									case 0x8021: //  LAN_X_SET_TRACK_POWER_OFF
									{
										printf("LAN_X_SET_TRACK_POWER_OFF");
										if (payload[6] == 0xA1)
										{
											pClient->getInterface()->getManager()->setPowerState(PowerOff);
										}
										break;
									}

									/** @ingroup Z21LANProtocol
									 * ---
									 * ### LAN_X_SET_TRACK_POWER_ON
									 * With the following command the trackpower is switched on.
									 * This cancels a pending emergencystop or programmingmode.
									 *
									 <table>
									 <caption>request:</caption>
									 <tr><th colspan="2">DataLen<th colspan="2">Header<th colspan="3">Data
									 <tr><td rowspan="2">0x07<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>XOR-Byte
									 <tr><td>0x21<td>0x81<td>0xA0
									 </table>

									 response: see UDPClient::broadcastPowerStateChange
									 */
									case 0x8121: //  LAN_X_SET_TRACK_POWER_ON
									{
										printf("LAN_X_SET_TRACK_POWER_ON");
										if (payload[6] == 0xA0)
										{
											pClient->getInterface()->getManager()->setPowerState(PowerOn);
										}
										break;
									}

									case 0x0AF1: //  LAN_X_GET_FIRMWARE_VERSION
									{
										printf("LAN_X_GET_FIRMWARE_VERSION");
										if (payload[6] == 0xFB)
										{
											const uint8_t LAN_X_FIRMWARE_VERSION[] = {0x09, 0x00, 0x40, 0x00, 0xF3, 0x0A, 0x01, 0x30, 0xC8};
											sendto(m_fdsock_me, LAN_X_FIRMWARE_VERSION, LAN_X_FIRMWARE_VERSION[0], 0, (struct sockaddr*)&si_other, sizeof(si_other));
										}
										break;
									}

									default:
									{
										break;
									}
								}
								break;
							}

							case 0x00610007: //  LAN_SET_LOCOMODE
							{
								printf("LAN_SET_LOCOMODE");
								Manager* pManager = pClient->getInterface()->getManager();
								uint16_t locAddress = (payload[4] << 8) + payload[5];
								Decoder* pDecoder = pManager->findDecoder(locAddress);
								if (pDecoder == NULL)
								{
									pDecoder = new LocDecoder(pManager, locAddress);
								}
								LocDecoder* pLoc = dynamic_cast<LocDecoder*>(pDecoder);
								if (pLoc)
								{
									pLoc->setLocMode(payload[6]);
								}
								break;
							}

							case 0x00710007: //  LAN_SET_TURNOUTMODE
							{
								printf("LAN_SET_TURNOUTMODE");
								//	TODO implementation
								break;
							}

							case 0x00890007: //  LAN_RAILCOM_GETDATA
							{
								printf("LAN_RAILCOM_GETDATA");
								//	TODO implementation
								break;
							}

							case 0x00A40007: //  LAN_LOCONET_DETECTOR
							{
								printf("LAN_LOCONET_DETECTOR");
								//	TODO implementation
								break;
							}

							case 0x00C40007: //  LAN_CAN_DETECTOR
							{
								printf("LAN_CAN_DETECTOR");
								//	TODO implementation
								break;
							}

							case 0x00400008:
							{
								switch (payload[4])
								{
									case 0x22:
									{
										if (payload[5] == 0x11) //  LAN_X_DCC_READ_REGISTER
										{
											printf("LAN_X_DCC_READ_REGISTER");
											//	TODO implementation
										}
										break;
									}

									case 0x43: //  LAN_X_GET_TURNOUT_INFO
									{
										printf("LAN_X_GET_TURNOUT_INFO");
										//	TODO implementation
										break;
									}

									default:
									{
										break;
									}
								}
								break;
							}

							case 0x00500008: //  LAN_SET_BROADCASTFLAGS
							{
								printf("LAN_SET_BROADCASTFLAGS");
								pClient->setBroadcastFlags(*(uint32_t*)&payload[4]);
								break;
							}

							case 0x00400009:
							{
								switch (payload[4])
								{
									case 0x23:
									{
										switch (payload[5])
										{
											case 0x11: //  LAN_X_CV_READ
											{
												printf("LAN_X_CV_READ");
												//	TODO implementation
												break;
											}

											case 0x12: //  LAN_X_DCC_WRITE_REGISTER
											{
												printf("LAN_X_DCC_WRITE_REGISTER");
												//	TODO implementation
												break;
											}

											default:
											{
												break;
											}
										}
										break;
									}

									case 0x53: //  LAN_X_SET_TURNOUT
									{
										printf("LAN_X_SET_TURNOUT");
										//	TODO implementation
										break;
									}

									case 0xE3: //  LAN_X_GET_LOCO_INFO
									{
										printf("LAN_X_GET_LOCO_INFO");
										Manager* pManager = pClient->getInterface()->getManager();
										uint16_t locAddress = ((payload[6] & 0x3F) << 8) + payload[7];
										Decoder* pDecoder = pManager->findDecoder(locAddress);
										if (pDecoder == NULL)
										{
											pDecoder = new LocDecoder(pManager, locAddress);
										}
										LocDecoder* pLoc = dynamic_cast<LocDecoder*>(pDecoder);
										if (pLoc)
										{
											uint8_t infoMessage[14];
											pLoc->getLANLocInfo(infoMessage);
											sendto(m_fdsock_me, infoMessage, infoMessage[0], 0, (struct sockaddr*)&si_other, sizeof(si_other));
										}
										break;
									}

									default:
									{
										break;
									}
								}
								break;
							}

							case 0x0040000A:
							{
								if (payload[4] == 0xE4)
								{
									Manager* pManager = pClient->getInterface()->getManager();
									uint16_t locAddress = ((payload[6] & 0x3F) << 8) + payload[7];
									Decoder* pDecoder = pManager->findDecoder(locAddress);
									if (pDecoder == NULL)
									{
										pDecoder = new LocDecoder(pManager, locAddress);
									}
									LocDecoder* pLoc = dynamic_cast<LocDecoder*>(pDecoder);
									if (pLoc)
									{
										if (payload[5] == 0xF8) //  LAN_X_SET_LOCO_FUNCTION
										{
											printf("LAN_X_SET_LOCO_FUNCTION");
											uint8_t FunctionIndex = payload[8] & 0x3F;
											uint8_t Type = payload[8] >> 5;
											switch (FunctionIndex)
											{
												case 0:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF0(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF0(!pLoc->getF0());
													}
													break;
												}
												case 1:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF1(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF1(!pLoc->getF1());
													}
													break;
												}
												case 2:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF2(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF2(!pLoc->getF2());
													}
													break;
												}
												case 3:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF3(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF3(!pLoc->getF3());
													}
													break;
												}
												case 4:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF4(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF4(!pLoc->getF4());
													}
													break;
												}
												case 5:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF5(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF5(!pLoc->getF5());
													}
													break;
												}
												case 6:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF6(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF6(!pLoc->getF6());
													}
													break;
												}
												case 7:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF7(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF7(!pLoc->getF7());
													}
													break;
												}
												case 8:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF8(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF8(!pLoc->getF8());
													}
													break;
												}
												case 9:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF9(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF9(!pLoc->getF9());
													}
													break;
												}
												case 10:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF10(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF10(!pLoc->getF10());
													}
													break;
												}
												case 11:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF11(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF11(!pLoc->getF11());
													}
													break;
												}
												case 12:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF12(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF12(!pLoc->getF12());
													}
													break;
												}
												case 13:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF13(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF13(!pLoc->getF13());
													}
													break;
												}
												case 14:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF14(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF14(!pLoc->getF14());
													}
													break;
												}
												case 15:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF15(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF15(!pLoc->getF15());
													}
													break;
												}
												case 16:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF16(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF16(!pLoc->getF16());
													}
													break;
												}
												case 17:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF17(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF17(!pLoc->getF17());
													}
													break;
												}
												case 18:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF18(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF18(!pLoc->getF18());
													}
													break;
												}
												case 19:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF19(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF19(!pLoc->getF19());
													}
													break;
												}
												case 20:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF20(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF20(!pLoc->getF20());
													}
													break;
												}
												case 21:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF21(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF21(!pLoc->getF21());
													}
													break;
												}
												case 22:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF22(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF22(!pLoc->getF22());
													}
													break;
												}
												case 23:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF23(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF23(!pLoc->getF23());
													}
													break;
												}
												case 24:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF24(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF24(!pLoc->getF24());
													}
													break;
												}
												case 25:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF25(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF25(!pLoc->getF25());
													}
													break;
												}
												case 26:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF26(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF26(!pLoc->getF26());
													}
													break;
												}
												case 27:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF27(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF27(!pLoc->getF27());
													}
													break;
												}
												case 28:
												{
													if (!(Type & 0x02))
													{
														pLoc->setF28(Type & 0x01);
													}
													else if (Type == 0x02)
													{
														pLoc->setF28(!pLoc->getF28());
													}
													break;
												}
												default:
												{
													break;
												}
											}
										}
										else if ((payload[5] & 0xF0) == 0x10) //  LAN_X_SET_LOCO_DRIVE
										{
											printf("LAN_X_SET_LOCO_DRIVE");
											switch(payload[5] & 0x0F)
											{
												case 0:
												case 1:
												{
													printf(" (14 Steps) ");
													pLoc->setLocoDrive14(payload[8]);
													break;
												}

												case 2:
												{
													printf(" (28 Steps) ");
													pLoc->setLocoDrive28(payload[8]);
													break;
												}

												case 3:
												{
													printf(" (128 Steps) ");
													pLoc->setLocoDrive128(payload[8]);
													break;
												}

												default:
												{
													printf(" !!! UNKNOWN STEPS !!!  ");
													break;
												}
											}
										}
										else
										{
											break;
										}
									}
								}
								else if (payload[4] == 0x24)
								{
									if (payload[5] == 0x12) //  LAN_X_CV_WRITE
									{
										printf("LAN_X_CV_WRITE");
										//	TODO implementation
									}
									else if (payload[5] == 0xFF) //  LAN_X_MM_WRITE_BYTE
									{
										printf("LAN_X_MM_WRITE_BYTE");
										//	TODO	implementation
									}
								}
								break;
							}

							case 0x0040000C:
							{
								switch (*(uint16_t*)&payload[4])
								{
									case 0x30E6:
									{
										if ((payload[8] & 0xFC) == 0xEC) //  LAN_X_CV_POM_WRITE_BYTE
										{
											printf("LAN_X_CV_POM_WRITE_BYTE");
											//	TODO	implementation
										}
										else if ((payload[8] & 0xFC) == 0xE8)//  LAN_X_CV_POM_WRITE_BIT
										{
											printf("LAN_X_CV_POM_WRITE_BIT");
											//	TODO	implementation
										}
										else if((payload[8] & 0xFC) == 0xE4) // LAN_X_CV_POM_READ_BYTE
										{
											printf("LAN_X_CV_POM_READ_BYTE");
											//	TODO	implementation
										}
										break;
									}

									case 0x31E6:
									{
										if ((payload[8] & 0xFC) == 0xEC) //  LAN_X_CV_POM_ACCESSORY_WRITE_BYTE
										{
											printf("LAN_X_CV_POM_ACCESSORY_WRITE_BYTE");
											//	TODO	implementation
										}
										else if ((payload[8] & 0xFC) == 0xE8) //  LAN_X_CV_POM_ACCESSORY_WRITE_BIT
										{
											printf("LAN_X_CV_POM_ACCESSORY_WRITE_BIT");
											//	TODO	implementation
										}
										else if((payload[8] & 0xFC) == 0xE4) //  LAN_X_CV_POM_ACCESSORY_READ_BYTE
										{
											printf("LAN_X_CV_POM_ACCESSORY_READ_BYTE");
											//	TODO	implementation
										}
										break;
									}

									default:
									{
										break;
									}
								}
								break;
							}

							default:
							{
								static const uint8_t LAN_X_UNKNOWN_COMMAND[] = {0x07, 0x00, 0x40, 0x00, 0x61, 0x82, 0xE3};
								sendto(m_fdsock_me, LAN_X_UNKNOWN_COMMAND, LAN_X_UNKNOWN_COMMAND[0], 0, (sockaddr*)&si_other, sizeof(si_other));
								break;
							}

						}	/*	switch(*(uint32_t*)payload)	*/
						printf(" ( ");
						for(uint8_t i = 0; i < payload[0]; i++)
						{
							printf("0x%02X ", payload[i]);
						}
						printf(").\n");

						payload += payload[0];	//	advance payload pointer to next message
					}	/*	while(payload <(recv...	*/
				}	/* if(m_fdsock_me == evlist..... */
			}	/* for(int notifications.... */

		}	/* while(bContinue) */
	}	/*	threadFunc(void)	*/

} /* namespace TBT */
