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
 * DccGenerator.cpp
 *
 *  Created on: Nov 29, 2017
 *      Author: paul
 */

#include "DccGenerator.h"

#include "Decoder.h"

#include <fcntl.h>
#include <unistd.h>

namespace TBT
{

	/**
	 * 	class DccGenerator constructor.
	 *
	 * 	Create a new DccGenerator instance and start the working thread.
	 *
	 * 	@param pDecoders : pointer to the pool of registered decoders in the system.
	 *
	 * 	@param pMutex : pointer to the mutex that protect the pool of decoders against concurrent access.
	 *
	 * 	@return none
	 */
	DccGenerator::DccGenerator(map<uint16_t, Decoder*>* pDecoders, recursive_mutex* pMutex)
	:	m_pDecoders(pDecoders)
	,	m_pMutex(pMutex)
	,	m_bContinue(true)
	{

		m_thread = thread([this]{ threadFunc(); });
	}

	/**
	 * 	class DccGenerator destructor.
	 *
	 * 	notify the worker thread to abort its activities and wait on it to finish.  Thereafter
	 * 	the DccGenerator instance is destroyed.
	 */
	DccGenerator::~DccGenerator()
	{
		m_bContinue = false;
		m_thread.join();
	}

	/**
	 * 	The function that 'does the work' in a separate thread.
	 *
	 * 	The function itterates over the decoders in the pool, querying each of them for
	 * 	a message to generate.  If a decoder responds with a message, this message is
	 * 	forwarded to the PRUSS subsystem, which in turn generates the signal on the rail
	 * 	terminals.
	 *
	 * 	The function terminates when bContinue is set to false.  This also terminates the
	 * 	thread.
	 *
	 * 	\warning	This function will 'eat the cpu' if and only if there are one or
	 * 	more decoder in the system and all of them responds false to getDccMessage.
	 */
	void DccGenerator::threadFunc()
	{
		uint8_t DCCMessage[32];
		int fd;
		fd = open("/dev/rpmsg_pru30", O_RDWR);
		if (fd < 0)
		{
			perror("/dev/rpmsg_pru30");
		}

		while(m_bContinue)
		{
			if (!m_pDecoders->empty())
			{
				for (auto decoderpair : *m_pDecoders)
				{
					if (decoderpair.second->getDccMessage(DCCMessage))
					{
						write(fd, DCCMessage, DCCMessage[0]);
					}
				}
			}
			else
			{
				sleep(1);
			}
		}
		close(fd);
	}

} /* namespace TBT */
