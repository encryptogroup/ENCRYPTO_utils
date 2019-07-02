/**
 \file 		timer.cpp
 \author 	michael.zohner@ec-spride.de
 \copyright	ABY - A Framework for Efficient Mixed-protocol Secure Two-party Computation
			Copyright (C) 2019 ENCRYPTO Group, TU Darmstadt
			This program is free software: you can redistribute it and/or modify
            it under the terms of the GNU Lesser General Public License as published
            by the Free Software Foundation, either version 3 of the License, or
            (at your option) any later version.
            ABY is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
            GNU Lesser General Public License for more details.
            You should have received a copy of the GNU Lesser General Public License
            along with this program. If not, see <http://www.gnu.org/licenses/>.
 \brief		timer Implementation
 */

#include <sys/time.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

#include "timer.h"
#include "constants.h"
#include "socket.h"
#include "typedefs.h"


aby_timings m_tTimes[P_LAST - P_FIRST + 1];
aby_comm m_tSend[P_LAST - P_FIRST + 1];
aby_comm m_tRecv[P_LAST - P_FIRST + 1];

double getMillies(timespec timestart, timespec timeend) {
	long time1 = (timestart.tv_sec * 1000000) + (timestart.tv_nsec / 1000);
	long time2 = (timeend.tv_sec * 1000000) + (timeend.tv_nsec / 1000);

	return (double) (time2 - time1) / 1000;
}

void StartWatch(const std::string& msg, ABYPHASE phase) {
	if (phase < P_FIRST || phase > P_LAST) {
		std::cerr << "Phase not recognized: " << phase << std::endl;
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &(m_tTimes[phase].tbegin));
#ifndef BATCH
	std::cout << msg << std::endl;
#else
	(void)msg;  // silence -Wunused-parameter warning
#endif
}


void StopWatch(const std::string& msg, ABYPHASE phase) {
	if (phase < P_FIRST || phase > P_LAST) {
		std::cerr << "Phase not recognized: " << phase << std::endl;
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &(m_tTimes[phase].tend));
	m_tTimes[phase].timing = getMillies(m_tTimes[phase].tbegin, m_tTimes[phase].tend);

#ifndef BATCH
	std::cout << msg << m_tTimes[phase].timing << " ms " << std::endl;
#else
	(void)msg;  // silence -Wunused-parameter warning
#endif
}

void StartRecording(const std::string& msg, ABYPHASE phase,
		const std::vector<std::unique_ptr<CSocket>>& sock) {
	StartWatch(msg, phase);

	m_tSend[phase].cbegin = 0;
	m_tRecv[phase].cbegin = 0;
	for(uint32_t i = 0; i < sock.size(); i++) {
		m_tSend[phase].cbegin += sock[i]->getSndCnt();
		m_tRecv[phase].cbegin += sock[i]->getRcvCnt();
	}
}

void StopRecording(const std::string& msg, ABYPHASE phase,
		const std::vector<std::unique_ptr<CSocket>>& sock) {
	StopWatch(msg, phase);

	m_tSend[phase].cend = 0;
	m_tRecv[phase].cend = 0;
	for(uint32_t i = 0; i < sock.size(); i++) {
		m_tSend[phase].cend += sock[i]->getSndCnt();
		m_tRecv[phase].cend += sock[i]->getRcvCnt();
	}

	m_tSend[phase].totalcomm = m_tSend[phase].cend - m_tSend[phase].cbegin;
	m_tRecv[phase].totalcomm = m_tRecv[phase].cend - m_tRecv[phase].cbegin;
}


void PrintTimings() {
	std::string unit = " ms";
	std::cout << "Timings: " << std::endl;
	std::cout << "Total =\t\t" << m_tTimes[P_TOTAL].timing << unit << std::endl;
	std::cout << "Init =\t\t" << m_tTimes[P_INIT].timing << unit << std::endl;
	std::cout << "CircuitGen =\t" << m_tTimes[P_CIRCUIT].timing << unit << std::endl;
	std::cout << "Network =\t" << m_tTimes[P_NETWORK].timing << unit << std::endl;
	std::cout << "BaseOTs =\t" << m_tTimes[P_BASE_OT].timing << unit << std::endl;
	std::cout << "Setup =\t\t" << m_tTimes[P_SETUP].timing << unit << std::endl;
	std::cout << "OTExtension =\t" << m_tTimes[P_OT_EXT].timing << unit << std::endl;
	std::cout << "Garbling =\t" << m_tTimes[P_GARBLE].timing << unit << std::endl;
	std::cout << "Online =\t" << m_tTimes[P_ONLINE].timing << unit << std::endl;
}

void PrintCommunication() {
	std::string unit = " bytes";
	std::cout << "Communication: " << std::endl;
	std::cout << "Total Sent / Rcv\t" << m_tSend[P_TOTAL].totalcomm << " " << unit << " / " << m_tRecv[P_TOTAL].totalcomm << unit << std::endl;
	std::cout << "BaseOTs Sent / Rcv\t" << m_tSend[P_BASE_OT].totalcomm << " " << unit << " / " << m_tRecv[P_BASE_OT].totalcomm << unit << std::endl;
	std::cout << "Setup Sent / Rcv\t" << m_tSend[P_SETUP].totalcomm << " " << unit << " / " << m_tRecv[P_SETUP].totalcomm << unit << std::endl;
	std::cout << "OTExtension Sent / Rcv\t" << m_tSend[P_OT_EXT].totalcomm << " " << unit << " / " << m_tRecv[P_OT_EXT].totalcomm << unit << std::endl;
	std::cout << "Garbling Sent / Rcv\t" << m_tSend[P_GARBLE].totalcomm << " " << unit << " / " << m_tRecv[P_GARBLE].totalcomm << unit << std::endl;
	std::cout << "Online Sent / Rcv\t" << m_tSend[P_ONLINE].totalcomm << " " << unit << " / " << m_tRecv[P_ONLINE].totalcomm << unit << std::endl;
}
