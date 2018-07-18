/**
 \file 		socket.cpp
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

#include "socket.h"
#include "utils.h"


#include <cstdint>
#include <cstring>
#include <iostream>

// moved here from typedefs.h
#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>

typedef int socklen_t;
#pragma comment(lib, "wsock32.lib")

#else //WIN32

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>

#endif //WIN32


CSocket::CSocket() {
	m_hSock = INVALID_SOCKET;
	m_nSndCount = 0;
	m_nRcvCount = 0;
}
CSocket::~CSocket() {
	Close();
}

uint64_t CSocket::getSndCnt() const {
	std::lock_guard<std::mutex> lock(m_nSndCount_mutex_);
	return m_nSndCount;
}
uint64_t CSocket::getRcvCnt() const {
	std::lock_guard<std::mutex> lock(m_nRcvCount_mutex_);
	return m_nRcvCount;
}
void CSocket::ResetSndCnt() {
	std::lock_guard<std::mutex> lock(m_nSndCount_mutex_);
	m_nSndCount = 0;
}
;
void CSocket::ResetRcvCnt() {
	std::lock_guard<std::mutex> lock(m_nRcvCount_mutex_);
	m_nRcvCount = 0;
}
;

bool CSocket::Socket() {
	bool success = false;
	m_nSndCount = 0;
	m_nRcvCount = 0;

#ifdef WIN32
	static bool s_bInit = false;

	if (!s_bInit) {
		WORD wVersionRequested;
		WSADATA wsaData;

		wVersionRequested = MAKEWORD(2, 0);
		WSAStartup(wVersionRequested, &wsaData);
		s_bInit = true;
	}
#endif

	Close();

	success = (m_hSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET;

	int one = 1;
	setsockopt(m_hSock, SOL_TCP, TCP_NODELAY, &one, sizeof(one));

	return success;

}

void CSocket::Close() {
	if (m_hSock == INVALID_SOCKET)
		return;

#ifdef WIN32
	shutdown(m_hSock, SD_SEND);
	closesocket(m_hSock);
#else
	shutdown(m_hSock, SHUT_WR);
	close(m_hSock);
#endif

	m_hSock = INVALID_SOCKET;
}

void CSocket::AttachFrom(CSocket& s) {
	m_hSock = s.m_hSock;
}

void CSocket::Detach() {
	m_hSock = INVALID_SOCKET;
}

std::string CSocket::GetIP() const {
	sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	if (getsockname(m_hSock, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0)
		return "";
	return inet_ntoa(addr.sin_addr);
}

uint16_t CSocket::GetPort() const {
	sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	if (getsockname(m_hSock, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0)
		return 0;
	return ntohs(addr.sin_port);
}

bool CSocket::Bind(uint16_t nPort, std::string ip) {
	// Bind the socket to its port
	sockaddr_in sockAddr;
	std::memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;

	if (ip != "") {
		int on = 1;
		setsockopt(m_hSock, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on));
		setsockopt(m_hSock, SOL_TCP, TCP_NODELAY, &on, sizeof(on));

		sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

		if (sockAddr.sin_addr.s_addr == INADDR_NONE) {
			hostent* phost;
			phost = gethostbyname(ip.c_str());
			if (phost != NULL)
				sockAddr.sin_addr.s_addr = ((in_addr*) phost->h_addr)->s_addr;
				else
				return false;
			}
		}
		else
		{
			sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		}

	sockAddr.sin_port = htons(nPort);

	return bind(m_hSock, (sockaddr *) &sockAddr, sizeof(sockaddr_in)) >= 0;
}

bool CSocket::Listen(int nQLen) {
	return listen(m_hSock, nQLen) >= 0;
}

bool CSocket::Accept(CSocket& sock) {
	sock.m_hSock = accept(m_hSock, NULL, 0);
	if (sock.m_hSock == INVALID_SOCKET)
		return false;

	return true;
}

bool CSocket::Connect(std::string ip, uint16_t port, long lTOSMilisec) {
	sockaddr_in sockAddr;
	std::memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (sockAddr.sin_addr.s_addr == INADDR_NONE) {
		hostent* lphost;
		lphost = gethostbyname(ip.c_str());
		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((in_addr*) lphost->h_addr)->s_addr;
			else
			return false;
		}

	sockAddr.sin_port = htons(port);

#ifdef WIN32

	DWORD dw = 100000;

	if( lTOSMilisec > 0 )
	{
		setsockopt(m_hSock, SOL_SOCKET, SO_RCVTIMEO, (char*) &lTOSMilisec, sizeof(lTOSMilisec));
	}

	int ret = connect(m_hSock, (sockaddr*)&sockAddr, sizeof(sockAddr));

	if( ret >= 0 && lTOSMilisec > 0 )
	setsockopt(m_hSock, SOL_SOCKET, SO_RCVTIMEO, (char*) &dw, sizeof(dw));

#else

	timeval tv;

	if (lTOSMilisec > 0) {
		tv.tv_sec = lTOSMilisec / 1000;
		tv.tv_usec = (lTOSMilisec % 1000) * 1000;

		setsockopt(m_hSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	}

	int one = 1;
	setsockopt(m_hSock, SOL_TCP, TCP_NODELAY, &one, sizeof(one));

	int ret = connect(m_hSock, (sockaddr*) &sockAddr, sizeof(sockAddr));

	if (ret >= 0 && lTOSMilisec > 0) {
		tv.tv_sec = 100000;
		tv.tv_usec = 0;

		setsockopt(m_hSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	}

#endif
	return ret >= 0;
}

uint64_t CSocket::Receive(void* pBuf, uint64_t nLen, int nFlags) {
	char* p = (char*) pBuf;
	uint64_t n = nLen;
#ifdef WIN32
	int ret = 0;
#else // POSIX
	ssize_t ret = 0;
#endif

	{
		std::lock_guard<std::mutex> lock(m_nRcvCount_mutex_);
		m_nRcvCount += nLen;
	}

	while (n > 0) {
		ret = recv(m_hSock, p, n, nFlags);
#ifdef WIN32
		if( ret <= 0 )
		{
			return nLen - n;
		}
#else
		if (ret < 0) {
			if (errno == EAGAIN) {
				std::cerr << "socket recv eror: EAGAIN" << std::endl;
				SleepMiliSec(200);
				continue;
			} else {
				std::cerr << "socket recv error: " << errno << std::endl;
				perror("Socket error ");
				return nLen - n;
			}
		} else if (ret == 0) {
			std::cerr << "socket recv: unexpected shutdown by peer\n";
			return nLen - n;
		}
#endif

		p += ret;
		n -= static_cast<uint64_t>(ret);
	}
	return nLen;
}

int CSocket::Send(const void* pBuf, uint64_t nLen, int nFlags) {
	char* p = (char*) pBuf;
	uint64_t n = nLen;
#ifdef WIN32
	int ret = 0;
#else // POSIX
	ssize_t ret = 0;
#endif
	{
		std::lock_guard<std::mutex> lock(m_nSndCount_mutex_);
		m_nSndCount += nLen;
	}

	while (n > 0) {
		ret = send(m_hSock, p, n, nFlags);
#ifdef WIN32
		if( ret <= 0 )
		{
			return nLen - n;
		}
#else
		if (ret < 0) {
			if ( errno == EAGAIN) {
				std::cerr << "socket send eror: EAGAIN" << std::endl;
				SleepMiliSec(200);
				continue;
			} else {
				std::cerr << "socket send error: " << errno << std::endl;
				perror("Socket error ");
				return nLen - n;
			}
		}
#endif

		p += ret;
		n -= static_cast<uint64_t>(ret);
	}
	return nLen;
}
