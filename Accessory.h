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

	/*
	 *
	 */
	class Accessory
	{
		public:
			Accessory(AccessoryDecoder* pAccessoryDecoder, uint8_t port);
			virtual ~Accessory();

			void setTurnout(bool outputNbr, bool state);

		protected:

		private:
			AccessoryDecoder*	m_pAccDecoder;
			uint8_t				m_Port;

	};

} /* namespace TBT */

#endif /* ACCESSORY_H_ */
