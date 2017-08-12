/**
 \file 		timer.h
 \author 	michael.zohner@ec-spride.de
 \copyright	ABY - A Framework for Efficient Mixed-protocol Secure Two-party Computation
			Copyright (C) 2015 Engineering Cryptographic Protocols Group, TU Darmstadt
			This program is free software: you can redistribute it and/or modify
			it under the terms of the GNU Affero General Public License as published
			by the Free Software Foundation, either version 3 of the License, or
			(at your option) any later version.
			This program is distributed in the hope that it will be useful,
			but WITHOUT ANY WARRANTY; without even the implied warranty of
			MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
			GNU Affero General Public License for more details.
			You should have received a copy of the GNU Affero General Public License
			along with this program. If not, see <http://www.gnu.org/licenses/>.
 \brief		timer Implementation
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/time.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include "socket.h"

#include "typedefs.h"

//Note do not change P_FIRST and P_LAST and keep them pointing to the first and last element in the enum
enum ABYPHASE {
	P_TOTAL, P_INIT, P_CIRCUIT, P_NETWORK, P_BASE_OT, P_SETUP, P_OT_EXT, P_GARBLE, P_ONLINE, P_FIRST = P_TOTAL, P_LAST = P_ONLINE
};

// Structure for measuring runtime
struct aby_timings {
	double timing;
	timespec tbegin;
	timespec tend;
};

// Structure for counting communication
struct aby_comm {
	uint64_t totalcomm;
	uint64_t cbegin;
	uint64_t cend;
};

static aby_timings m_tTimes[P_LAST - P_FIRST + 1];
static aby_comm m_tSend[P_LAST - P_FIRST + 1];
static aby_comm m_tRecv[P_LAST - P_FIRST + 1];

/**
 * Return time difference in milliseconds
 */
static double getMillies(timespec timestart, timespec timeend) {
	long time1 = (timestart.tv_sec * 1000000) + (timestart.tv_nsec / 1000);
	long time2 = (timeend.tv_sec * 1000000) + (timeend.tv_nsec / 1000);

	return (double) (time2 - time1) / 1000;
}

/**
 * Start measuring runtime for a given phase
 * @param msg - a message for debugging
 * @param phase - the ABY phase to measure
 */
static void StartWatch(const std::string& msg, ABYPHASE phase) {
	if (phase < P_FIRST || phase > P_LAST) {
		std::cerr << "Phase not recognized: " << phase << std::endl;
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &(m_tTimes[phase].tbegin));
#ifndef BATCH
	std::cout << msg << std::endl;
#endif
}

/**
 * Stop measuring runtime
 * Called after StartWatch() with identical phase parameter
 * @param msg - a message for debugging
 * @param phase - the ABY phase to measure
 */
static void StopWatch(const std::string& msg, ABYPHASE phase) {
	if (phase < P_FIRST || phase > P_LAST) {
		std::cerr << "Phase not recognized: " << phase << std::endl;
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &(m_tTimes[phase].tend));
	m_tTimes[phase].timing = getMillies(m_tTimes[phase].tbegin, m_tTimes[phase].tend);

#ifndef BATCH
	std::cout << msg << m_tTimes[phase].timing << " ms " << std::endl;
#endif
}

/**
 * Start measuring both runtime and communication
 * @param msg - a message for debugging
 * @param phase - the ABY phase to measure
 * @param sock - a vector of sockets
 */
static void StartRecording(const std::string& msg, ABYPHASE phase, std::vector<CSocket*> sock) {
	StartWatch(msg, phase);

	m_tSend[phase].cbegin = 0;
	m_tRecv[phase].cbegin = 0;
	for(uint32_t i = 0; i < sock.size(); i++) {
		m_tSend[phase].cbegin += sock[i]->getSndCnt();
		m_tRecv[phase].cbegin += sock[i]->getRcvCnt();
	}
}

/**
 * Stop measuring both runtime and communication
 * Called after StartRecording() with identical phase parameter
 * @param msg - a message for debugging
 * @param phase - the ABY phase to measure
 * @param sock - a vector of sockets
 */
static void StopRecording(const std::string& msg, ABYPHASE phase, std::vector<CSocket*> sock) {
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


static void PrintTimings() {
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

static void PrintCommunication() {
	std::string unit = " bytes";
	std::cout << "Communication: " << std::endl;
	std::cout << "Total Sent / Rcv\t" << m_tSend[P_TOTAL].totalcomm << " " << unit << " / " << m_tRecv[P_TOTAL].totalcomm << unit << std::endl;
	std::cout << "BaseOTs Sent / Rcv\t" << m_tSend[P_BASE_OT].totalcomm << " " << unit << " / " << m_tRecv[P_BASE_OT].totalcomm << unit << std::endl;
	std::cout << "Setup Sent / Rcv\t" << m_tSend[P_SETUP].totalcomm << " " << unit << " / " << m_tRecv[P_SETUP].totalcomm << unit << std::endl;
	std::cout << "OTExtension Sent / Rcv\t" << m_tSend[P_OT_EXT].totalcomm << " " << unit << " / " << m_tRecv[P_OT_EXT].totalcomm << unit << std::endl;
	std::cout << "Garbling Sent / Rcv\t" << m_tSend[P_GARBLE].totalcomm << " " << unit << " / " << m_tRecv[P_GARBLE].totalcomm << unit << std::endl;
	std::cout << "Online Sent / Rcv\t" << m_tSend[P_ONLINE].totalcomm << " " << unit << " / " << m_tRecv[P_ONLINE].totalcomm << unit << std::endl;
}

static double GetTimeForPhase(ABYPHASE phase) {
	return m_tTimes[phase].timing;
}

static uint64_t GetSentDataForPhase(ABYPHASE phase) {
	return m_tSend[phase].totalcomm;
}

static uint64_t GetReceivedDataForPhase(ABYPHASE phase) {
	return m_tRecv[phase].totalcomm;
}

#endif /* TIMER_H_ */
