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
 * GPIOPin.cpp
 *
 *  Created on: Jan 29, 2018
 *      Author: paul
 */

#include "GPIOPin.h"

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace TBT
{

	GPIOPin::GPIOPin(int number)
	:	m_pinNumber(number)
	{
		ostringstream s;
		s << "gpio" << m_pinNumber;
		m_Name = string(s.str());
		m_Path = GPIO_PATH + m_Name + "/";
		exportPin();
		usleep(250000);
	}

	GPIOPin::~GPIOPin()
	{
		unexportPin();
	}

	int GPIOPin::setDirection(GPIO_DIRECTION direction)
	{
		switch(direction)
		{
			case INPUT:
			{
				return write(m_Path, "direction", "in");
			}

			case OUTPUT:
			{
				return write(m_Path, "direction", "out");
			}
		}
		return -1;
	}

	GPIOPin::GPIO_DIRECTION GPIOPin::getDirection()
	{
		string input = read(m_Path, "edge");
		if(input == "in")
			return INPUT;
		else
			return OUTPUT;
	}

	int GPIOPin::setValue(GPIO_VALUE value)
	{
		switch(value)
		{
			case HIGH:
			{
				return write(m_Path, "value", "1");
			}

			case LOW:
			{
				return write(m_Path, "value", "0");
			}
		}
		return -1;
	}

	GPIOPin::GPIO_VALUE GPIOPin::getValue()
	{
		string input = read(m_Path, "value");
		if(input == "0")
			return LOW;
		return HIGH;
	}

	int GPIOPin::setActiveLow(bool isLow /* = true */)
	{
		if(isLow)
			return this->write(m_Path, "active_low", "1");
		return this->write(m_Path, "active_low", "0");
	}

	int GPIOPin::setActiveHigh()
	{
		return setActiveLow(false);
	}

	int GPIOPin::exportPin()
	{
		return write(GPIO_PATH, "export", m_pinNumber);
	}

	int GPIOPin::unexportPin()
	{
		return write(GPIO_PATH, "unexport", m_pinNumber);
	}

	int GPIOPin::write(string path, string filename, string value)
	{
		ofstream fs;
		fs.open((path + filename).c_str());
		if(!fs.is_open())
		{
			perror("GPIO: write failed to open file ");
			return -1;
		}
		fs << value;
		fs.close();
		return 0;
	}

	int GPIOPin::write(string path, string filename, int value)
	{
		stringstream s;
		s << value;
		return write(path, filename, s.str());
	}

	string GPIOPin::read(string path, string filename)
	{
		ifstream fs;
		fs.open((path + filename).c_str());
		if(!fs.is_open())
		{
			perror("GPIO: read failed to open file ");
		}
		string input;
		getline(fs, input);
		fs.close();
		return input;
	}

} /* namespace TBT */
