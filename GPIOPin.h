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
 * GPIOPin.h
 *
 *  Created on: Jan 29, 2018
 *      Author: paul
 */

#ifndef GPIOPIN_H_
#define GPIOPIN_H_

#include <stdint.h>
#include <string>
#include <fstream>

using std::string;
using std::ofstream;

#define GPIO_PATH "/sys/class/gpio/"

namespace TBT
{

	/*
	 *
	 */
	class GPIOPin
	{
		public:
			enum GPIO_DIRECTION
			{
				INPUT,
				OUTPUT
			};

			enum GPIO_VALUE
			{
				LOW = 0,
				HIGH = 1
			};

			enum GPIO_EDGE
			{
				NONE,
				RISING,
				FALLING,
				BOTH
			};

			GPIOPin(int number);
			virtual ~GPIOPin();

			int getNumber(void) { return m_pinNumber; }
			const string& getPath(void) { return m_Path; }

			int setDirection(GPIO_DIRECTION direction);
			GPIO_DIRECTION getDirection(void);

			int setEdge(GPIO_EDGE edge);
			GPIO_EDGE getEdge(void);

			int setValue(GPIO_VALUE value);
			GPIO_VALUE getValue(void);

			int setActiveLow(bool isLow = true);
			int setActiveHigh(void);

		protected:
			int exportPin(void);
			int unexportPin(void);

			int write(string path, string filename, string value);
			int write(string path, string filename, int value);

			string read(string path, string filename);

		private:
			int			m_pinNumber;
			string		m_Name;
			string		m_Path;

	};

} /* namespace TBT */

#endif /* GPIOPIN_H_ */
