/*
 * Copyright (C) 2018 Paul Van den Bergh
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
 * Accessory.h
 *
 *  Created on: Feb 6, 2018
 *      Author: paul
 */

#ifndef ACCESSORY_H_
#define ACCESSORY_H_

#include "AccessoryDecoder.h"

namespace TBT
{

#define ACCESSORYREPEATCOUNT	5
	/*
	 *
	 */
	class Accessory
	{
		public:
			Accessory(AccessoryDecoder* pAccessoryDecoder, uint8_t port);
			virtual ~Accessory();

			uint8_t getUDPState(void);
			void setUDPState(uint8_t newState);
			bool getDCCMessage(uint8_t* pMsg);
			void setState(uint8_t outputNbr, uint8_t state);

		protected:

		private:
			AccessoryDecoder*	m_pAccDecoder;
			uint8_t				m_Port;

			uint8_t				m_currentState[2];
			uint8_t				m_desiredState[2];

			uint8_t				m_UDPState;

	};

} /* namespace TBT */

#endif /* ACCESSORY_H_ */
