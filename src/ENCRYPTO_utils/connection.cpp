/**
 \file 		connection.cpp
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
 \brief		Connection Implementation
 */

#include "connection.h"
#include "constants.h"
#include "socket.h"
#include "utils.h"
#include <cassert>
#include <iostream>
#include <limits>

bool Connect(const std::string& address, uint16_t port,
		std::vector<std::unique_ptr<CSocket>> &sockets, uint32_t id) {
#ifndef BATCH
	std::cout << "Connecting party "<< id <<": " << address << ", " << port << std::endl;
#endif
	assert(sockets.size() <= std::numeric_limits<uint32_t>::max());
	for (size_t j = 0; j < sockets.size(); j++) {
		sockets[j] = Connect(address, port);
		if (sockets[j]) {
			// handshake
			sockets[j]->Send(&id, sizeof(id));
			uint32_t index = static_cast<uint32_t>(j);
			sockets[j]->Send(&index, sizeof(index));
		}
		else {
			return false;
		}
	}
	return true;
}

bool Listen(const std::string& address, uint16_t port,
		std::vector<std::vector<std::unique_ptr<CSocket>>> &sockets,
		size_t numConnections, uint32_t myID) {

	auto listen_socket = std::make_unique<CSocket>();

	if (!listen_socket->Bind(address, port)) {
		std::cerr << "Error: a socket could not be bound\n";
		return false;
	}
	if (!listen_socket->Listen()) {
		std::cerr << "Error: could not listen on the socket \n";
		return false;
	}

	for (size_t i = 0; i < numConnections; i++)
	{
		auto sock = listen_socket->Accept();
		if (!sock) {
			std::cerr << "Error: could not accept connection\n";
			return false;
		}
		// receive initial pid when connected
		uint32_t nID;
		uint32_t conID; //a mix of threadID and role - depends on the application
		sock->Receive(&nID, sizeof(nID));
		sock->Receive(&conID, sizeof(conID));

		if (nID >= sockets.size()) //Not more than two parties currently allowed
				{
			sock->Close();
			i--;  // try same index again
			continue;
		}
		if (conID >= sockets[myID].size()) {
			sock->Close();
			i--;  // try same index again
			continue;
		}
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
