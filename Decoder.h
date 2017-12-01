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
 * Decoder.h
 *
 *  Created on: Nov 24, 2017
 *      Author: paul
 */

#ifndef DECODER_H_
#define DECODER_H_

#include <stdint.h>
#include "Manager.h"

namespace TBT
{

	class Decoder
	{
		public:
			Decoder(Manager* pManager, uint16_t dccAddress);
			virtual ~Decoder();

			const uint16_t		getDCCAddress(void) { return m_DCCAddress; }
			virtual bool		getDccMessage(uint8_t* pMsg) { return false; }

		protected:
			uint16_t			m_DCCAddress;
			Manager*			m_pManager;

		private:

	};	/*	class Decoder	*/

} /* namespace TBT */

#endif /* DECODER_H_ */
