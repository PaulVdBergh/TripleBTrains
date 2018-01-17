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
 * DccGenerator.h
 *
 *  Created on: Nov 29, 2017
 *      Author: paul
 */

#ifndef DCCGENERATOR_H_
#define DCCGENERATOR_H_

#include "Manager.h"

#include <thread>

using namespace std;

namespace TBT
{

	///	Class TBT::DccGenerator is responsible for the generation of the DCC signal on the rails.
	/**
	 * Class DccGenerator is responsible for the communication with the PRUSS subsystem, which in turn
	 * generates the DCC signal, including the RailCom gap.  The class interrogate every decoder and,
	 * if needed, assembles an appropriate DCC message.  This message is handled over to the PRUSS sub-
	 * system.  The PRUSS system generates the correct timings and waveform according to the DCC message.
	 */
	class DccGenerator
	{
		public:
			///	class DccGenerator constructor
			DccGenerator(map<uint16_t, Decoder*>* pDecoders, recursive_mutex* pMutex);

			///	class DccGenerator destructor
			virtual ~DccGenerator();

		protected:

		private:
			///	The function that 'does the work' in a separate thread
			void						threadFunc(void);

			///	pointer to the pool of decoders in the system
			map<uint16_t, Decoder*>*	m_pDecoders;

			///	pointer to the mutex that protect the pool of decoders against concurrent access
			recursive_mutex*			m_pMutex;

			///	the effective worker thread
			thread						m_thread;

			///	flag indicating the thread to continue(true)/shutdown(false)
			volatile bool				m_bContinue;
	};

} /* namespace TBT */

#endif /* DCCGENERATOR_H_ */
