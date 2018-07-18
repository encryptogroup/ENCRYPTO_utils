/**
 \file 		socket.h
 \author 	Seung Geol Choi
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
 \brief		Socket Implementation
 */

#ifndef __SOCKET_H__BY_SGCHOI
#define __SOCKET_H__BY_SGCHOI

#include <cstdint>
#include <mutex>
#include <string>

// moved here from typedefs.h
#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>

typedef unsigned short USHORT;
typedef int socklen_t;
#pragma comment(lib, "wsock32.lib")

#else //WIN32
//
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <netinet/tcp.h>
//
typedef int SOCKET;
#define INVALID_SOCKET -1

#endif //WIN32


class CSocket {
public:
	CSocket();
	~CSocket();

	uint64_t getSndCnt() const;
	uint64_t getRcvCnt() const;
	void ResetSndCnt();
	void ResetRcvCnt();

	bool Socket();

	void Close();

	void AttachFrom(CSocket& s);

	void Detach();

	std::string GetIP() const;

	uint16_t GetPort() const;

	bool Bind(uint16_t nPort = 0, std::string ip = "");

	bool Listen(int nQLen = 5);

	bool Accept(CSocket& sock);

	bool Connect(std::string ip, uint16_t port, long lTOSMilisec = -1);

	uint64_t Receive(void* pBuf, uint64_t nLen, int nFlags = 0);

	int Send(const void* pBuf, uint64_t nLen, int nFlags = 0);

private:
	SOCKET m_hSock;
	uint64_t m_nSndCount, m_nRcvCount;
	mutable std::mutex m_nSndCount_mutex_;
	mutable std::mutex m_nRcvCount_mutex_;
};

#endif //SOCKET_H__BY_SGCHOI

