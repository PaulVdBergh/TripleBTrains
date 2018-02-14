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
 * XpressNetClientInterface.cpp
 *
 *  Created on: Nov 24, 2017
 *      Author: paul
 */

#include "XpressNetClient.h"
#include "XpressNetClientInterface.h"

#include "AccessoryDecoder.h"
#include "LocDecoder.h"

#include <string.h>

#include <algorithm>
#include <asm/termbits.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace TBT
{

	XpressNetClientInterface::XpressNetClientInterface(Manager* pManager, const char* ttyPath /* = "/dev/ttyO1" */)
	:	ClientInterface(pManager)
	{
		m_pTtyPath = new char[strlen(ttyPath) + 1];
		strcpy(m_pTtyPath, ttyPath);

		termios2 settings;

		m_fdSerial = open(m_pTtyPath, O_RDWR | O_NOCTTY);
		if(0 > m_fdSerial)
		{
			//TODO error recovery
		}

		int r = ioctl(m_fdSerial, TCGETS2, &settings);
		if(r)
		{
			//TODO error recovery
		}

		settings.c_ispeed = settings.c_ospeed = 62500;
		settings.c_cflag &= ~CBAUD;
		settings.c_cflag |= BOTHER;
		settings.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
		settings.c_oflag &= ~OPOST;
		settings.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		settings.c_cflag &= ~(CSIZE | PARENB);
		settings.c_cflag |= CS8;

		r = ioctl(m_fdSerial, TCSETS2, &settings);
		if(r)
		{
			//	TODO Error recovery
		}

		m_fdStop = eventfd(0, 0);

		m_thread = thread([this]{threadFunc();});
	}

	XpressNetClientInterface::~XpressNetClientInterface()
	{
		delete [] m_pTtyPath;

		uint64_t stop = 1;
		if(sizeof(stop) != write(m_fdStop, &stop, sizeof(stop)))
		{
			//	TODO error recovery

		}
		m_thread.join();
		close(m_fdSerial);
		close(m_fdStop);
	}

	void XpressNetClientInterface::broadcastPowerStateChange(bool newState)
	{
		//	if newState == true --> Track Power On --> PowerState --> Off
		if(newState)
		{
			uint8_t msg[] = {0x05, 0x60, 0x61, 0x01, 0x60};
			write(m_fdSerial, msg, msg[0]);
		}
		else
		{
			uint8_t msg[] = {0x05, 0x60, 0x61, 0x00, 0x61};
			write(m_fdSerial, msg, msg[0]);
		}
	}

	void XpressNetClientInterface::broadcastLocInfoChange(LocDecoder* pLoc)
	{
		//	TODO implementation
	}

	void XpressNetClientInterface::broadcastAccessoryInfoChanged(Accessory* pAccessory)
	{
		lock_guard<recursive_mutex> guard(m_MClients);
		for(auto client : m_Clients)
		{
			client.second->broadcastAccessoryInfoChanged(pAccessory);
		}
	}

	void XpressNetClientInterface::broadcastEmergencyStop()
	{
		uint8_t msg[] = {0x05, 0x60, 0x81, 0x00, 0x81};
		write(m_fdSerial, msg, msg[0]);
	}

	void XpressNetClientInterface::broadcastOvercurrent()
	{
		lock_guard<recursive_mutex> guard(m_MClients);
		for(auto client : m_Clients)
		{
			client.second->broadcastOvercurrent();
		}
	}

	XpressNetClient* XpressNetClientInterface::findClient(const uint8_t& address)
	{
		lock_guard<recursive_mutex> guard(m_MClients);

		auto client = m_Clients.find(address);
		if(m_Clients.end() == client)
		{
			XpressNetClient* pClient = new XpressNetClient(this, address);
			m_Clients[address] = pClient;
			return pClient;
		}
		return client->second;
	}

	uint8_t XpressNetClientInterface::makeStraight(uint8_t* pValue)
	{
		*pValue &= 0x7F;
		for(uint8_t i = 0x40; i != 0; i >>= 1)
		{
			if(*pValue & i)
			{
				*pValue ^= 0x80;
			}
		}
		return *pValue;
	}

	void XpressNetClientInterface::threadFunc()
	{
		#define NBRXPRESSNETPOLLEVENTS 2
		epoll_event ev;
		epoll_event evlist[NBRXPRESSNETPOLLEVENTS];
		int epfd;
		bool bContinue = true;

		if(-1 == (epfd = epoll_create(NBRXPRESSNETPOLLEVENTS)))
		{
			//	TODO Error recovery
		}

		memset(&ev, 0, sizeof(ev));
		ev.data.fd = m_fdStop;
		ev.events = EPOLLIN;
		if(-1 == (epoll_ctl(epfd, EPOLL_CTL_ADD, m_fdStop, &ev)))
		{
			//	TODO	Error recovery
		}

		memset(&ev, 0, sizeof(ev));
		ev.data.fd = m_fdSerial;
		ev.events = EPOLLIN;
		if(-1 == (epoll_ctl(epfd, EPOLL_CTL_ADD, m_fdSerial, &ev)))
		{
			//	TODO Error recovery
		}

		while(bContinue)
		{
			int notifications = epoll_wait(epfd, evlist, NBRXPRESSNETPOLLEVENTS, -1);

			timespec time;
			clock_gettime(CLOCK_MONOTONIC, &time);
			printf("%lld.%.09ld : ", (long long)time.tv_sec, time.tv_nsec);

			if (-1 == notifications)
			{
				if (EINTR == errno)
				{
					continue;
				}
				else
				{
					//	TODO : better error recovery !!
					perror("epoll_wait() failed in XpressNetClientInterface ");
					bContinue = false;
					continue;
				}
			}

			for(int notification = 0; notification < notifications; notification++)
			{
				if (m_fdStop == evlist[notification].data.fd)
				{
					bContinue = false;
				}
				else if (m_fdSerial == evlist[notification].data.fd)
				{
					uint8_t msg[32];
					ssize_t nbrRead = read(m_fdSerial, msg, 1);
					if (nbrRead == -1)
					{
						perror("read m_fdSerial");
					}
					else if (nbrRead != 1)
					{
						perror("read m_fdSerial nbrRead");
					}
					else
					{
						nbrRead = read(m_fdSerial, &msg[1], msg[0] - 1);
						while(nbrRead != msg[0] - 1)
						{
							ssize_t nbr = read(m_fdSerial, &msg[nbrRead], 1);
							if (nbr == 1)
							{
								++nbrRead;
								continue;
							}
						}
						//	Process msg here
						uint8_t check = 0;
						printf("XpressNet : Received %i bytes from address %i : ", msg[0], msg[1]);

						for(uint8_t i = 2; i < msg[0]; i++)
						{
							check ^= msg[i];
						}

						if (check)
						{
							printf("--> Checksum Error <--\n");
							// TODO The message has an invalid checksum.
							// Notify the client and discard message.
							//	But, since there is a checksum error, can we be sure the
							//	client address is consistent ???

							continue;
						}

						// The message has good checksum, so process it.
						XpressNetClient* pClient = findClient(msg[1]);
						switch(msg[2])
						{
							case 0x21:
							{
								switch (msg[3])
								{
									case 0x10://	Request for Service Mode results
									{
										printf("Request for Service Mode results");
										//	TODO Implementation
										break;
									}

									case 0x21://	Command station software-version request
									{
										printf("Command station software-version request");
										uint8_t response[7] = {0x07, 0x60, 0x63, 0x21, 0x30, 0x00, 0x72};
										response[1] += msg[1];
										makeStraight(&(response[1]));

										ssize_t nbrWritten = write(m_fdSerial, response, response[0]);
										if (nbrWritten == -1)
										{
											perror("Write response");
										}
										break;
									}

									case 0x24://	Command station status request
									{
										printf("Command station status request");
										uint8_t response[6] = {0x06, 0x60, 0x62, 0x22, 0x00, 0x00};
										response[1] += msg[1];	//	address
										makeStraight(&(response[1]));
										uint8_t centralState = pClient->getInterface()->getManager()->getCentralState();

										response[4] = centralState & 0x03;				// Emergency Stop and Track Voltage Off
										response[4] |= ((centralState & 0x04) << 1);	// Short Curcuit

										response[5] = response[2] ^ response[3] ^ response[4];
										ssize_t nbrWritten = write(m_fdSerial, response, response[0]);
										if (nbrWritten == -1)
										{
											perror("Write response");
										}
										break;
									}

									case 0x80://	Track Power Off
									{
										printf("Track Power Off");
										pClient->getInterface()->getManager()->setPowerState(PowerOff);
										break;
									}

									case 0x81://	Resume operation
									{
										printf("Resume operation");
										pClient->getInterface()->getManager()->setPowerState(PowerOn);
										break;
									}

									default:
									{
										break;
									}
								}
								break;
							}	//	case 0x21:

							case 0x22:
							{
								switch(msg[3])
								{
									case 0x11://	Register mode Read request
									{
										printf("Register mode Read request");
										break;
									}

									case 0x14://	Paged mode Read request
									{
										printf("Paged mode Read request");
										break;
									}

									case 0x15://	Direct mode CV Read request
									{
										printf("Direct mode CV Read request");
										break;
									}

									case 0x18://	Direct mode CV Read request (CV 1 - 255)
									{
										printf("Direct mode CV Read request (CV 1 - 255)");
										break;
									}

									case 0x19://	Direct mode CV Read request (CV 256 - 511)
									{
										printf("Direct mode CV Read request (CV 256 - 511)");
										break;
									}

									case 0x1A://	Direct mode CV Read request (CV 512 - 767)
									{
										printf("Direct mode CV Read request (CV 512 - 767)");
										break;
									}

									case 0x1B://	Direct mode CV Read request (CV 768 - 1023)
									{
										printf("Direct mode CV Read request (CV 768 - 1023)");
										break;
									}

									case 0x22://	Set Command station power-up mode
									{
										printf("Set Command station power-up mode");
										break;
									}

									default:
									{
										break;
									}
								}
								break;
							}	//	case 0x22

							case 0x23:
							{
								switch(msg[3])
								{
									case 0x12://	Register mode Write request
									{
										printf("Register mode Write request");
										break;
									}

									case 0x16://	Direct mode CV Write request
									{
										printf("Direct mode CV Write request");
										break;
									}

									case 0x17://	Paged mode Write request
									{
										printf("Paged mode Write request");
										break;
									}

									case 0x1C://	Direct mode CV Write request (CV 1 - 255)
									{
										printf("Direct mode CV Write request (CV 1 - 255)");
										break;
									}

									case 0x1D://	Direct mode CV Write request (CV 256 - 511)
									{
										printf("Direct mode CV Write request (CV 256 - 511)");
										break;
									}

									case 0x1E://	Direct mode CV Write request (CV 512 - 767)
									{
										printf("Direct mode CV Write request (CV 512 - 767)");
										break;
									}

									case 0x1F://	Direct mode CV Write request (CV 768 - 1023)
									{
										printf("Direct mode CV Write request (CV 768 - 1023)");
										break;
									}

									default:
									{
										break;
									}
								}
								break;
							}	//	case 0x23

							case 0x42://	Accessory Decoder Information Request
							{
								printf("Accessory Decoder Information Request");
								uint16_t dccAddress = 0x8000 | msg[3];
								uint8_t nibble = (msg[4] & 0x01);

								Manager* pManager = pClient->getInterface()->getManager();
								Decoder* pDecoder = pManager->findDecoder(dccAddress);
								if(!pDecoder)
								{
									pDecoder = new AccessoryDecoder(pManager, dccAddress);
								}
								AccessoryDecoder* pAccessoryDecoder = dynamic_cast<AccessoryDecoder*>(pDecoder);
								if(pAccessoryDecoder)
								{
									uint8_t response[] = {0x06, 0x60, 0x42, 0x00, 0x00, 0x00};
									response[1] += msg[1];
									makeStraight(&(response[1]));
									response[3] = msg[3];
									response[4] |= nibble << 4;
									response[4] |= (pAccessoryDecoder->getState(0 + (nibble * 2)));
									response[4] |= (pAccessoryDecoder->getState(1 + (nibble * 2))) << 2;
									if(((response[4] & 0x03) == 0) || ((response[4] & 0x0C) == 0))
									{
										response[4] |= 0x80;
									}
									for (uint8_t i = 2; i < 5; i++)
									{
										response[5] ^= response[i];
									}
									ssize_t nbrWritten = write(m_fdSerial, response, response[0]);
									if (nbrWritten == -1)
									{
										perror("Write response");
									}
								}
								break;
							}	//	case 0x42

							case 0x52://	Accessory Decoder operation request
							{
								printf("Accessory Decoder operation request");
								uint16_t dccAddress = 0x8000 | msg[3];
								uint8_t Port =  ((msg[4] & 0x06) >> 1);

								Manager* pManager = pClient->getInterface()->getManager();
								Decoder* pDecoder = pManager->findDecoder(dccAddress);
								if(!pDecoder)
								{
									pDecoder = new AccessoryDecoder(pManager, dccAddress);
								}
								AccessoryDecoder* pAccessoryDecoder = dynamic_cast<AccessoryDecoder*>(pDecoder);
								if(pAccessoryDecoder)
								{
									pAccessoryDecoder->setDesiredState(Port, msg[4] & 0x01, (msg[4] & 0x08) >> 3);
								}
								break;
							}	//	case 0x52

							case 0x80://	Emergency stop
							{
								printf("Emergency stop");
								pClient->getInterface()->getManager()->setEmergencyStop();
								break;
							}	//	case 0x80

							case 0x92://	Emergency stop a Locomotive
							{
								printf("Emergency stop a Locomotive");
								break;
							}	//	case 0x92

							case 0xE2://	Address Inquiry Multi-unit request
							{
								printf("Address Inquiry Multi-unit request");
								break;
							}	//	case 0xE2

							case 0xE3:
							{
								switch (msg[3])
								{
									case 0x00://	Locomotive information request
									{
										printf("Locomotive information request");
										uint16_t dccAddress = (msg[4] << 8) + msg[5];
										if(dccAddress > 127)
										{
											dccAddress |= 0xC000;
										}
										Decoder* pDecoder = pClient->getInterface()->getManager()->findDecoder(dccAddress);
										if (pDecoder == NULL)
										{
											pDecoder = new LocDecoder(pClient->getInterface()->getManager(), dccAddress);
										}
										LocDecoder* pLoc = dynamic_cast<LocDecoder*>(pDecoder);
										if (pLoc)
										{
											uint8_t response[8] = {0x08, 0x60, 0xE4, 0x00, 0x00, 0x00, 0x00, 0x00};
											response[1] += msg[1];
											makeStraight(&(response[1]));

											response[3] = pLoc->getSpeedsteps();
											response[4] = pLoc->getDirection() | pLoc->getSpeed();
											response[5] = pLoc->getFunctionGroup1();
											response[6] = pLoc->getFunctionGroup2() | (pLoc->getFunctionGroup3() << 4);
											for (uint8_t i = 2; i < 7; i++)
											{
												response[7] ^= response[i];
											}
											ssize_t nbrWritten = write(m_fdSerial, response, response[0]);
											if (nbrWritten == -1)
											{
												perror("Write response");
											}
										}
										break;
									}

									case 0x07://	Function status request
									{
										printf("Function status request");
										break;
									}

									case 0x08://	Function status request (F13 - F28)
									{
										printf("Function status request (F13 - F28)");
										break;
									}

									case 0x09://	Function state request (F13 - F28)
									{
										printf("Function state request (F13 - F28)");
										break;
									}

									case 0x44://	Delete Locomotive from Command Station stack request
									{
										printf("Delete Locomotive from Command Station stack request");
										break;
									}

									default:
									{
										break;
									}
								}
								break;
							}	//	case 0xE3

							case 0xE4:
							{
								uint16_t dccAddress = (msg[4] << 8) + msg[5];
								if(dccAddress > 127)
								{
									dccAddress |= 0xC000;
								}
								Decoder* pDecoder = pClient->getInterface()->getManager()->findDecoder(dccAddress);
								if(!pDecoder)
								{
									pDecoder = new LocDecoder(pClient->getInterface()->getManager(), dccAddress);
								}
								LocDecoder* pLoc = dynamic_cast<LocDecoder*>(pDecoder);

								if(pLoc)
								{
									switch(msg[3])
									{
										case 0x10://	Locomotive Speed and Direction operation (14 steps)
										{
											printf("Locomotive Speed and Direction operation (14 steps)");
											pLoc->setLocoDrive14(msg[6]);
											break;
										}

										case 0x11://	Locomotive Speed and Direction operation (27 steps)
										{
											printf("Locomotive Speed and Direction operation (27 steps)");
											pLoc->setLocoDrive27(msg[6]);
											break;
										}

										case 0x12://	Locomotive Speed and Direction operation (28 steps)
										{
											printf("Locomotive Speed and Direction operation (28 steps)");
											pLoc->setLocoDrive28(msg[6]);
											break;
										}

										case 0x13://	Locomotive Speed and Direction operation (128 steps)
										{
											printf("Locomotive Speed and Direction operation (128 steps)");
											pLoc->setLocoDrive128(msg[6]);
											break;
										}

										case 0x20://	Function operation instruction (F0 - F4)
										{
											printf("Function operation instruction (F0 - F4)");
											pLoc->setFunctionGroup1(msg[6]);
											break;
										}

										case 0x21://	Function operation instruction (F5 - F8)
										{
											printf("Function operation instruction (F5 - F8)");
											pLoc->setFunctionGroup2(msg[6]);
											break;
										}

										case 0x22://	Function operation instruction (F9 - F12)
										{
											printf("Function operation instruction (F9 - F12)");
											pLoc->setFunctionGroup3(msg[6]);
											break;
										}

										case 0x23://	Function operation instruction (F13 - F20)
										{
											printf("Function operation instruction (F13 - F20)");
											pLoc->setFunctionGroup4(msg[6]);
											break;
										}

										case 0x28://	Function operation instruction (F21 - F28)
										{
											printf("Function operation instruction (F21 - F28)");
											pLoc->setFunctionGroup5(msg[6]);
											break;
										}

										case 0x42://	Remove Locomotive from multi-unit request
										{
											printf("Remove Locomotive from multi-unit request");
											break;
										}

										default:
										{
											break;
										}
									}
								}
								break;
							}	//	case 0xE4

							case 0xE5://	Establish Double Header
							{
								printf("Establish Double Header");
								break;
							}	//	case 0xE5

							case 0xE6://	POM mode Write request
							{
								printf("POM mode Write request");
								break;
							}	//	case 0xE6

							default:
							{
								break;
							}

						}	//	switch(msg[2])

						printf(" ( ");
						for (uint8_t i = 0; i < msg[0]; i++)
						{
							printf("0x%02X ", msg[i]);
						}
						printf(" )\n");
					}
				}	//	else if (m_fdSerial == evlist[notification].data.fd)
			}	//	for(int notification = 0; notification < notifications; notification++)
		}	//	while(bContinue)
	}

} /* namespace TBT */
