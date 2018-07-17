/**
 \file 		connection.cpp
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
 \brief		Connection Implementation
 */

#include "connection.h"
#include "constants.h"
#include "socket.h"
#include <sstream>
#include <iostream>

BOOL Connect(std::string address, short port, std::vector<CSocket*> &sockets, int id) {
	int nNumConnections;

	BOOL bFail = FALSE;
	LONG lTO = CONNECT_TIMEO_MILISEC;
	//std::cout << "Connecting" << endl;
	std::ostringstream os;
#ifndef BATCH
	std::cout << "Connecting party "<< id <<": " << address << ", " << port << std::endl;
#endif

	for (int j = 0; j < sockets.size(); j++) {
		for (int i = 0; i < RETRY_CONNECT; i++) {
			if (!sockets[j]->Socket())
				goto connect_failure;
			if (sockets[j]->Connect(address, port, lTO)) {
				// send pid when connected
				sockets[j]->Send(&id, sizeof(int));
				sockets[j]->Send(&j, sizeof(int));
#ifndef BATCH
				os.str("");
				os << " (" << id << ") (" << j << ") connected" << std::endl;
				std::cout << os.str() << std::flush;
#endif
				if (j == sockets.size() - 1) {
					return TRUE;
				} else {
					break;
				}
			}
			SleepMiliSec(10);
			sockets[j]->Close();
		}
	}

	connect_failure:

	os.str("");
	os << " (" << id << ") connection failed due to timeout!\n";
	std::cout << os.str() << std::flush;
	return FALSE;

}

BOOL Listen(std::string address, short port, std::vector<std::vector<CSocket*> > &sockets, int numConnections, int myID) {
	// everybody except the last thread listenes
	std::ostringstream os;

#ifndef BATCH
	std::cout << "Listening: " << address << ":" << port << std::endl;
#endif
	if (!sockets[myID][0]->Socket()) {
		std::cerr << "Error: a socket could not be created \n";
		goto listen_failure;
	}
	if (!sockets[myID][0]->Bind(port, address)) {
		std::cerr << "Error: a socket could not be bound\n";
		goto listen_failure;
	}
	if (!sockets[myID][0]->Listen()) {
		std::cerr << "Error: could not listen on the socket \n";
		goto listen_failure;
	}

	for (int i = 0; i < numConnections; i++) //twice the actual number, due to double sockets for OT
			{
		CSocket sock;
		if (!sockets[myID][0]->Accept(sock)) {
			std::cerr << "Error: could not accept connection\n";
			goto listen_failure;
		}
		// receive initial pid when connected
		UINT nID;
		UINT conID; //a mix of threadID and role - depends on the application
		sock.Receive(&nID, sizeof(int));
		sock.Receive(&conID, sizeof(int));

		if (nID >= sockets.size()) //Not more than two parties currently allowed
				{
			sock.Close();
			i--;
			continue;
		}
		if (conID >= sockets[myID].size()) {
			sock.Close();
			i--;
			continue;
		}

#ifndef BATCH
		os.str("");
		os << " (" << conID <<") (" << conID << ") connection accepted\n";

		std::cout << os.str() << std::flush;
#endif
		// locate the socket appropriately
		sockets[nID][conID]->AttachFrom(sock);
		sock.Detach();
	}

#ifndef BATCH
	std::cout << "Listening finished" << std::endl;
#endif
	return TRUE;

	listen_failure: std::cout << "Listen failed" << std::endl;
	return FALSE;
}
