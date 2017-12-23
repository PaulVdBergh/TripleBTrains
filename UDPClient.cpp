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
 * UDPClient.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: paul
 */

#include "UDPClient.h"

#include "LocDecoder.h"

namespace TBT
{

	UDPClient::UDPClient(UDPClientInterface* pinterface, const sockaddr_in& address)
	:	Client(pinterface)
	,	m_Address(address)
	,	m_BroadcastFlags(0)
	{
		m_MySocket = ((UDPClientInterface*)m_pInterface)->getMySocket();
	}

	UDPClient::~UDPClient()
	{
		// TODO Auto-generated destructor stub
	}

	/**
	 *
	 <table>
	 <caption>response: LAN_X_BC_TRACK_POWER_ON</caption>
	 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="3">Data
	 <tr><td rowspan="2">0x07<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>XOR-Byte
	 <tr><td>0x61<td>0x01<td>0x60
	 </table>
	 <table>
	 <caption>response: LAN_X_BC_TRACK_POWER_OFF</caption>
	 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="3">Data
	 <tr><td rowspan="2">0x07<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>XOR-Byte
	 <tr><td>0x61<td>0x00<td>0x61
	 </table>
	 */
	void UDPClient::broadcastPowerStateChange(bool newState)
	{
		if(newState)	//	Track Power On
		{
			const uint8_t msg[] = { 0x07, 0x00, 0x40, 0x00, 0x61, 0x01, 0x60 };
			sendto(m_MySocket, msg, msg[0], 0, (sockaddr*)&m_Address, sizeof(m_Address));
		}
		else	//	Track Power Off
		{
			const uint8_t msg[] = { 0x07, 0x00, 0x40, 0x00, 0x61, 0x00, 0x61 };
			sendto(m_MySocket, msg, msg[0], 0, (sockaddr*)&m_Address, sizeof(m_Address));
		}
	}

	void UDPClient::broadcastLocInfoChanged(LocDecoder* pLoc)
	{
		uint8_t pMsg[0x0E];
		pLoc->getLANLocInfo(pMsg);

		sendto(m_MySocket, pMsg, pMsg[0], 0, (sockaddr*)&m_Address, sizeof(m_Address));
	}

	/**
	 *
	 <table>
	 <caption>response: LAN_X_BC_STOPPED</caption>
	 <tr><th colspan="2">DataLen<th colspan="2">Headers<th colspan="3">Data
	 <tr><td rowspan="2">0x07<td rowspan="2">0x00<td rowspan="2">0x40<td rowspan="2">0x00<th>X-Header<th>DB0<th>XOR-Byte
	 <tr><td>0x81<td>0x00<td>0x81
	 </table>
	 */
	void UDPClient::broadcastEmergencyStop(void)
	{
		uint8_t msg[] = { 0x07, 0x00, 0x40, 0x00, 0x81, 0x00, 0x81};
		sendto(m_MySocket, msg, msg[0], 0, (sockaddr*)&m_Address, sizeof(m_Address));
	}

	/**
	 *
	 <table>
	 <caption>Broadcast flags is an OR combination of the following values:</caption>
	 <tr><th>Value<th>Meaning
	 <tr><td>0x00000001<td>Automatically generated broadcasts and messages about driving
	 and switching are delivered to the registered client. The following messages are
	 subscribed here:
	 - LAN_X_BC_TRACK_POWER_OFF
	 - LAN_X_BC_TRACK_POWER_ON
	 - LAN_X_BC_PROGRAMMING_MODE
	 - LAN_X_BC_TRACK_SHORT_CIRCUIT
	 - LAN_X_BC_STOPPED
	 - LAN_X_LOCO_INFO (the relevant locomotive address must also be subscribed)
	 - LAN_X_TURNOUT_INFO
	 <tr><td>0x00000002<td>Changes to the feedback on the R-Bus are sent automatically.
	 Broadcast message of Z21 see LAN_RMBUS_DATACHANGED
	 <tr><td>0x00000004<td>Changes to RailCom data of the subscribed locomotives will be sent automatically.
	 Broadcast message of Z21 see LAN_RAILCOM_DATACHANGED
	 <tr><td>0x00000100<td>Changes to the Z21 system state are sent automatically.
	 Broadcast message of Z21 see LAN_SYSTEMSTATE_DATACHANGED
	 <tr><td colspan="2"><b>From Z21 FW Version 1.20:</b>
	 <tr><td>0x00010000<td>Replaces flag 0x00000001;
	 Client now gets LAN_X_LOCO_INFO without first having to subscribe to the corresponding loco
	 addresses, i.e. for all controlled locomotives! Due to the high network traffic, this flag
	 may only be used in full PC controlled applications and is by no means intended for mobile
	 handheld controllers.
	 - From FW V1.20 to V1.23: LAN_X_LOCO_INFO is sent for all locomotives.
	 - From FW V1.24: LAN_X_LOCO_INFO will be sent for all modified locos.
	 <tr><td>0x01000000<td>Forward messages from the LocoNet bus to the LAN client without locos and points.
	 <tr><td>0x02000000<td>Forward Loco-Specific LocoNet Messages to LAN Client: OPC_LOCO_SPD,
	 OPC_LOCO_DIRF, OPC_LOCO_SND, OPC_LOCO_F912, OPC_EXP_CMD
	 <tr><td>0x04000000<td>Route turnout-specific LocoNet messages to LAN client: OPC_SW_REQ,
	 OPC_SW_REP, OPC_SW_ACK, OPC_SW_STATE
	 <tr><td colspan="2"><b>From Z21 FW Version 1.22:</b>
	 <tr><td>0x08000000<td>Send status messages from track occupancy detectors on the LocoNet bus to the LAN client.
	 See LAN_LOCONET_DETECTOR
	 <tr><td colspan="2"><b>From Z21 FW Version 1.29:</b>
	 <tr><td>0x00040000<td>Changes to RailCom data are sent automatically.
	 Client gets LAN_RAILCOM_DATACHANGED, even without having to subscribe to the locomotive addresses
	 beforehand, i.e. for all controlled locomotives!
	 Due to the high network traffic, this flag may only be used in full PC controlled applications and
	  is by no means intended for mobile handheld controllers.
	  Broadcast message of Z21 see LAN_RAILCOM_DATACHANGED
	 <tr><td colspan="2"><b>From Z21 FW Version 1.30:</b>
	 <tr><td>0x00080000<td>Send status messages from track occupancy detectors on the CAN bus to the LAN client.
	 See LAN_CAN_DETECTOR
	 </table>
	 */
	uint32_t UDPClient::getBroadcastFlags()
	{
		return m_BroadcastFlags;
	}

} /* namespace TBT */
