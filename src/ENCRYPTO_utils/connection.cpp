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
#include "utils.h"
#include <iostream>

bool Connect(const std::string& address, uint16_t port,
		std::vector<std::unique_ptr<CSocket>> &sockets, int id) {
#ifndef BATCH
	std::cout << "Connecting party "<< id <<": " << address << ", " << port << std::endl;
#endif

	for (size_t j = 0; j < sockets.size(); j++) {
		for (int i = 0; i < RETRY_CONNECT; i++) {
			if (sockets[j]->Connect(address, port)) {
				// send pid when connected
				sockets[j]->Send(&id, sizeof(int));
				sockets[j]->Send(&j, sizeof(int));
#ifndef BATCH
				std::cout << " (" << id << ") (" << j << ") connected" << std::endl;
#endif
				if (j == sockets.size() - 1) {
					return true;
				} else {
					break;
				}
			}
			SleepMiliSec(10);
			sockets[j]->Close();
		}
	}

	std::cerr << " (" << id << ") connection failed due to timeout!" << std::endl;

	return false;
}

bool Listen(const std::string& address, uint16_t port,
		std::vector<std::vector<std::unique_ptr<CSocket>>> &sockets, int
		numConnections, int myID) {
	// everybody except the last thread listenes

	auto listen_socket = std::make_unique<CSocket>();

#ifndef BATCH
	std::cout << "Listening: " << address << ":" << port << std::endl;
#endif
	if (!listen_socket->Bind(address, port)) {
		std::cerr << "Error: a socket could not be bound\n";
		return false;
	}
	if (!listen_socket->Listen()) {
		std::cerr << "Error: could not listen on the socket \n";
		return false;
	}

	for (int i = 0; i < numConnections; i++)
	{
		auto sock = listen_socket->Accept();
		if (!sock) {
			std::cerr << "Error: could not accept connection\n";
			return false;
		}
		// receive initial pid when connected
		UINT nID;
		UINT conID; //a mix of threadID and role - depends on the application
		sock->Receive(&nID, sizeof(int));
		sock->Receive(&conID, sizeof(int));

		if (nID >= sockets.size()) //Not more than two parties currently allowed
				{
			sock->Close();
			i--;
			continue;
		}
		if (conID >= sockets[myID].size()) {
			sock->Close();
			i--;
			continue;
		}

#ifndef BATCH
		std::cout << " (" << conID <<") (" << conID << ") connection accepted" std::endl;
#endif
		// locate the socket appropriately
		sockets[nID][conID] = std::move(sock);
	}

#ifndef BATCH
	std::cout << "Listening finished" << std::endl;
#endif
	return true;
}

std::unique_ptr<CSocket> Connect(const std::string& address, uint16_t port) {
	auto socket = std::make_unique<CSocket>();
	for (int i = 0; i < RETRY_CONNECT; i++) {
		if (socket->Connect(address, port))
			return socket;
		SleepMiliSec(10);
	}
	std::cerr << "Connect failed due to timeout!\n";
	return nullptr;
}

std::unique_ptr<CSocket> Listen(const std::string& address, uint16_t port) {
	auto listen_socket = std::make_unique<CSocket>();
	if (!listen_socket->Bind(address, port)) {
		return nullptr;
	}
	if (!listen_socket->Listen()) {
		return nullptr;
	}
	return listen_socket->Accept();
}
