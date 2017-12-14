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
 * LocDecoder.h
 *
 *  Created on: Nov 24, 2017
 *      Author: paul
 */

#ifndef LOCDECODER_H_
#define LOCDECODER_H_

#include "Decoder.h"

namespace TBT
{

	class LocDecoder: public Decoder
	{
		public:
			LocDecoder(Manager* pManager, uint16_t dccAddress);
			virtual ~LocDecoder();

			virtual bool getDccMessage(uint8_t* pMsg);

			uint8_t getLocMode(void) { return m_LocMode; }
			void	getLANLocInfo(uint8_t* pMsg);
			uint8_t	setLocMode(uint8_t newMode);

			bool getBusy() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB2 & 0x08); }
			uint8_t getSpeedsteps() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB2 & 0x07); }
			bool getDirection() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB3 & 0x80); }
			uint8_t getSpeed() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB3 & 0x7F); }
			bool getDualTraction() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x40); }
			bool getSmartSearch() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x20); }
			bool getLight() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x10); }
			uint8_t getFunctionGroup1() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x1F); }
			bool getF0() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x10); }
			bool getF1() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x01); }
			bool getF2() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x02); }
			bool getF3() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x04); }
			bool getF4() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB4 & 0x08); }
			uint8_t getFunctionGroup2() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x0F); }
			bool getF5() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x01); }
			bool getF6() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x02); }
			bool getF7() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x04); }
			bool getF8() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x08); }
			uint8_t getFunctionGroup3() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 >> 4); }
			bool getF9() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x10); }
			bool getF10() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x20); }
			bool getF11() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x40); }
			bool getF12() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB5 & 0x80); }
			uint8_t getFunctionGroup4() { lock_guard<recursive_mutex>lock(m_MLocInfo); return (m_LocInfo.DB6); }
			bool getF13() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB6 & 0x01); }
			bool getF14() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB6 & 0x02); }
			bool getF15() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB6 & 0x04); }
			bool getF16() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB6 & 0x08); }
			bool getF17() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB6 & 0x10); }
			bool getF18() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB6 & 0x20); }
			bool getF19() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB6 & 0x40); }
			bool getF20() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB6 & 0x80); }
			uint8_t getFunctionGroup5() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7); }
			bool getF21() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7 & 0x01); }
			bool getF22() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7 & 0x02); }
			bool getF23() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7 & 0x04); }
			bool getF24() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7 & 0x08); }
			bool getF25() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7 & 0x10); }
			bool getF26() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7 & 0x20); }
			bool getF27() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7 & 0x40); }
			bool getF28() { lock_guard<recursive_mutex> lock(m_MLocInfo); return (m_LocInfo.DB7 & 0x80); }

			void setBusy(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB2 |= 0x08) : (m_LocInfo.DB2 &= ~(0x08)); m_pManager->broadcastLocInfoChanged(this); }
			void setSpeedsteps(uint8_t value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB2 &= ~(0x07); m_LocInfo.DB2 |= (value & 0x07); m_pManager->broadcastLocInfoChanged(this); }
			void setLocoDrive14(uint8_t value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB3 = value; m_pManager->broadcastLocInfoChanged(this); }
			void setLocoDrive27(uint8_t value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB3 = value; m_pManager->broadcastLocInfoChanged(this); }
			void setLocoDrive28(uint8_t value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB3 = value; m_pManager->broadcastLocInfoChanged(this); }
			void setLocoDrive128(uint8_t value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB3 = value; m_pManager->broadcastLocInfoChanged(this); }
			void setDirection(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB3 |= 0x80) : (m_LocInfo.DB3 &= ~(0x80)); m_pManager->broadcastLocInfoChanged(this); }
			void setSpeed(uint8_t value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB3 &= ~(0x7F); m_LocInfo.DB3 |= (value & 0x7F); m_pManager->broadcastLocInfoChanged(this); }
			void setDualTraction(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB4 |= 0x40) : (m_LocInfo.DB4 &= ~(0x40)); m_pManager->broadcastLocInfoChanged(this); }
			void setSmartSearch(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB4 |= 0x20) : (m_LocInfo.DB4 &= ~(0x20)); m_pManager->broadcastLocInfoChanged(this); }
			void setFunctionGroup1(const uint8_t& value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB4 &= ~0x1F; m_LocInfo.DB4 |= (value & 0x1F); m_pManager->broadcastLocInfoChanged(this); }
			void setLight(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB4 |= 0x10) : (m_LocInfo.DB4 &= ~(0x10)); m_pManager->broadcastLocInfoChanged(this); }
			void setF0(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB4 |= 0x10) : (m_LocInfo.DB4 &= ~(0x10)); m_pManager->broadcastLocInfoChanged(this); }
			void setF1(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB4 |= 0x01) : (m_LocInfo.DB4 &= ~(0x01)); m_pManager->broadcastLocInfoChanged(this); }
			void setF2(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB4 |= 0x02) : (m_LocInfo.DB4 &= ~(0x02)); m_pManager->broadcastLocInfoChanged(this); }
			void setF3(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB4 |= 0x04) : (m_LocInfo.DB4 &= ~(0x04)); m_pManager->broadcastLocInfoChanged(this); }
			void setF4(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB4 |= 0x08) : (m_LocInfo.DB4 &= ~(0x08)); m_pManager->broadcastLocInfoChanged(this); }
			void setFunctionGroup2(const uint8_t& value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB5 &= 0xF0; m_LocInfo.DB5 |= (value & 0x0F); m_pManager->broadcastLocInfoChanged(this); }
			void setF5(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB5 |= 0x01) : (m_LocInfo.DB5 &= ~(0x01)); m_pManager->broadcastLocInfoChanged(this); }
			void setF6(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB5 |= 0x02) : (m_LocInfo.DB5 &= ~(0x02)); m_pManager->broadcastLocInfoChanged(this); }
			void setF7(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB5 |= 0x04) : (m_LocInfo.DB5 &= ~(0x04)); m_pManager->broadcastLocInfoChanged(this); }
			void setF8(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB5 |= 0x08) : (m_LocInfo.DB5 &= ~(0x08)); m_pManager->broadcastLocInfoChanged(this); }
			void setFunctionGroup3(const uint8_t& value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB5 &= 0x0F; m_LocInfo.DB5 |= (value << 4); m_pManager->broadcastLocInfoChanged(this); }
			void setF9(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB5 |= 0x10) : (m_LocInfo.DB5 &= ~(0x10)); m_pManager->broadcastLocInfoChanged(this); }
			void setF10(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB5 |= 0x20) : (m_LocInfo.DB5 &= ~(0x20)); m_pManager->broadcastLocInfoChanged(this); }
			void setF11(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB5 |= 0x40) : (m_LocInfo.DB5 &= ~(0x40)); m_pManager->broadcastLocInfoChanged(this); }
			void setF12(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB5 |= 0x80) : (m_LocInfo.DB5 &= ~(0x80)); m_pManager->broadcastLocInfoChanged(this); }
			void setFunctionGroup4(const uint8_t& value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB6 = value; m_pManager->broadcastLocInfoChanged(this); }
			void setF13(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB6 |= 0x01) : (m_LocInfo.DB6 &= ~(0x01)); m_pManager->broadcastLocInfoChanged(this); }
			void setF14(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB6 |= 0x02) : (m_LocInfo.DB6 &= ~(0x02)); m_pManager->broadcastLocInfoChanged(this); }
			void setF15(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB6 |= 0x04) : (m_LocInfo.DB6 &= ~(0x04)); m_pManager->broadcastLocInfoChanged(this); }
			void setF16(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB6 |= 0x08) : (m_LocInfo.DB6 &= ~(0x08)); m_pManager->broadcastLocInfoChanged(this); }
			void setF17(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB6 |= 0x10) : (m_LocInfo.DB6 &= ~(0x10)); m_pManager->broadcastLocInfoChanged(this); }
			void setF18(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB6 |= 0x20) : (m_LocInfo.DB6 &= ~(0x20)); m_pManager->broadcastLocInfoChanged(this); }
			void setF19(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB6 |= 0x40) : (m_LocInfo.DB6 &= ~(0x40)); m_pManager->broadcastLocInfoChanged(this); }
			void setF20(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB6 |= 0x80) : (m_LocInfo.DB6 &= ~(0x80)); m_pManager->broadcastLocInfoChanged(this); }
			void setFunctionGroup5(const uint8_t& value) { lock_guard<recursive_mutex> lock(m_MLocInfo); m_LocInfo.DB7 = value; m_pManager->broadcastLocInfoChanged(this); }
			void setF21(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB7 |= 0x01) : (m_LocInfo.DB7 &= ~(0x01)); m_pManager->broadcastLocInfoChanged(this); }
			void setF22(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB7 |= 0x02) : (m_LocInfo.DB7 &= ~(0x02)); m_pManager->broadcastLocInfoChanged(this); }
			void setF23(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB7 |= 0x04) : (m_LocInfo.DB7 &= ~(0x04)); m_pManager->broadcastLocInfoChanged(this); }
			void setF24(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB7 |= 0x08) : (m_LocInfo.DB7 &= ~(0x08)); m_pManager->broadcastLocInfoChanged(this); }
			void setF25(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB7 |= 0x10) : (m_LocInfo.DB7 &= ~(0x10)); m_pManager->broadcastLocInfoChanged(this); }
			void setF26(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB7 |= 0x20) : (m_LocInfo.DB7 &= ~(0x20)); m_pManager->broadcastLocInfoChanged(this); }
			void setF27(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB7 |= 0x40) : (m_LocInfo.DB7 &= ~(0x40)); m_pManager->broadcastLocInfoChanged(this); }
			void setF28(bool value) { lock_guard<recursive_mutex> lock(m_MLocInfo); value ? (m_LocInfo.DB7 |= 0x80) : (m_LocInfo.DB7 &= ~(0x80)); m_pManager->broadcastLocInfoChanged(this); }

		protected:
			bool getDCCSpeedMessage(uint8_t* pMsg);
			bool getDCCFG1Message(uint8_t* pMsg);
			bool getDCCFG2Message(uint8_t* pMsg);
			bool getDCCFG3Message(uint8_t* pMsg);
			bool getDCCFG4Message(uint8_t* pMsg);
			bool getDCCFG5Message(uint8_t* pMsg);

			uint8_t m_LocMode;

		private:
			uint8_t	m_DCCState;
			uint8_t	m_CurrentCVRead;
			LocInfo m_LocInfo;
			recursive_mutex m_MLocInfo;

	};

} /* namespace TBT */

#endif /* LOCDECODER_H_ */
