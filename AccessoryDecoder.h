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
 * AccessoryDecoder.h
 *
 *  Created on: Feb 2, 2018
 *      Author: paul
 */

#ifndef ACCESSORYDECODER_H_
#define ACCESSORYDECODER_H_

#include "Decoder.h"

namespace TBT
{
	class Accessory;	//	Forward Declaration

	/*
	 *
	 */
	class AccessoryDecoder: public Decoder
	{
		public:
			AccessoryDecoder(Manager* pManager, uint16_t dccAddress);
			virtual ~AccessoryDecoder();

			Accessory* getAccessory(uint8_t port) { return m_pAccessories[port]; }

			virtual bool getDccMessage(uint8_t* pMsg);

			uint8_t	getState(uint8_t port);
			void setDesiredState(uint8_t port, uint8_t OutputNbr, uint8_t state);

		protected:
			virtual uint8_t* insertDCCAddress(uint8_t* pMsg) { return NULL; };

			Accessory*	m_pAccessories[4];

		private:

	};	/*	class AccessoryDecoder	*/

} /* namespace TBT */

#endif /* ACCESSORYDECODER_H_ */
