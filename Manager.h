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
 * Manager.h
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include <vector>
using namespace std;

#include "Types.h"

namespace TBT
{

	class Interface;

	class Manager
	{
		public:
			Manager();
			virtual ~Manager();

			void setPowerState(PowerState newState);

		protected:

		private:
			PowerState	m_PowerState;

			vector<Interface*>	m_Interfaces;

	};	/* class Manager */

} /* namespace TBT */

#endif /* MANAGER_H_ */
