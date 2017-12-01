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

	DccGenerator::DccGenerator(map<uint16_t, Decoder*>* pDecoders, recursive_mutex* pMutex)
	:	m_pDecoders(pDecoders)
	,	m_pMutex(pMutex)
	,	m_bContinue(true)
	{

		m_thread = thread([this]{ threadFunc(); });
	}

	DccGenerator::~DccGenerator()
	{
		m_bContinue = false;
		m_thread.join();
	}

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
