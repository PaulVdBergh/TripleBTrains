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

	class DccGenerator
	{
		public:
			DccGenerator(map<uint16_t, Decoder*>* pDecoders, recursive_mutex* pMutex);
			virtual ~DccGenerator();

		protected:

		private:
			void						threadFunc(void);

			map<uint16_t, Decoder*>*	m_pDecoders;
			recursive_mutex*			m_pMutex;

			thread						m_thread;
			volatile bool				m_bContinue;
	};

} /* namespace TBT */

#endif /* DCCGENERATOR_H_ */
