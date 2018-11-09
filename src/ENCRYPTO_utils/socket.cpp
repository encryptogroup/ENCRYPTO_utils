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

#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
using boost::asio::ip::tcp;


struct CSocket::CSocketImpl {
	CSocketImpl()
		: io_context(std::make_shared<boost::asio::io_context>()),
		socket(*io_context), acceptor(*io_context)
	{}
	std::shared_ptr<boost::asio::io_context> io_context;
	tcp::socket socket;
	tcp::acceptor acceptor;
};

CSocket::CSocket()
	: impl_(std::make_unique<CSocketImpl>()), send_count_(0), recv_count_(0)
{}

CSocket::~CSocket() {
	Close();
}

uint64_t CSocket::getSndCnt() const {
	std::lock_guard<std::mutex> lock(m_nSndCount_mutex_);
	return send_count_;
}
uint64_t CSocket::getRcvCnt() const {
	std::lock_guard<std::mutex> lock(m_nRcvCount_mutex_);
	return recv_count_;
}
void CSocket::ResetSndCnt() {
	std::lock_guard<std::mutex> lock(m_nSndCount_mutex_);
	send_count_ = 0;
}
void CSocket::ResetRcvCnt() {
	std::lock_guard<std::mutex> lock(m_nRcvCount_mutex_);
	recv_count_ = 0;
}

bool CSocket::Socket() {
	send_count_ = 0;
	recv_count_ = 0;
	Close();
	return true;
}

void CSocket::Close() {
	impl_->socket.close();
}

void CSocket::AttachFrom(CSocket& s) {
	// TODO
}

void CSocket::Detach() {
	// TODO
}

std::string CSocket::GetIP() const {
	boost::system::error_code ec;
	auto endpoint = impl_->socket.local_endpoint(ec);
	if (ec) {
		return "";
	}
	return endpoint.address().to_string();
}

uint16_t CSocket::GetPort() const {
	boost::system::error_code ec;
	auto endpoint = impl_->socket.local_endpoint(ec);
	if (ec) {
		return 0;
	}
	return endpoint.port();
}

bool CSocket::Bind(uint16_t port, const std::string& ip) {
	boost::system::error_code ec;
	boost::asio::ip::address address;

	if (ip.empty()) {
		// Use "::" if no address is given
		address = boost::asio::ip::address_v6();
	} else {
		// Try to parse given address
		address = boost::asio::ip::make_address(ip, ec);
		if (ec) {
			std::cerr << "make_address failed: " << ec.message() << "\n";
			std::cerr << "with argument: " << ip << "\n";
			return false;
		}
	}

	tcp::endpoint endpoint(address, port);

	impl_->acceptor.open(endpoint.protocol(), ec);
	if (ec) {
		std::cerr << "acceptor socket open failed: " << ec.message() << "\n";
		std::cerr << "endpoint: " << endpoint << "\n";
		return false;
	}

	// Use dual stack IPv4 and IPv6
	if (endpoint.protocol() == tcp::v6()) {
		boost::asio::ip::v6_only opt(false);
		impl_->acceptor.set_option(opt, ec);
		if (ec) {
			std::cerr << "acceptor disable option IPPROTO_IPV6/IP_V6ONLY failed: "
				<< ec.message() << "\n";
			return false;
		}
	}
	
	// Set socket options
	boost::asio::socket_base::reuse_address opt_reuse_addr(true);
	tcp::no_delay opt_tcp_no_delay(true);
	impl_->acceptor.set_option(opt_reuse_addr, ec);
	if (ec) {
		std::cerr << "acceptor set option SO_REUSEADDR failed: " << ec.message() << "\n";
		std::cerr << "endpoint: " << endpoint << "\n";
		return false;
	}
	impl_->acceptor.set_option(opt_tcp_no_delay, ec);
	if (ec) {
		std::cerr << "acceptor set option TCP_NODELAY failed: " << ec.message() << "\n";
		std::cerr << "endpoint: " << endpoint << "\n";
		return false;
	}

	impl_->acceptor.bind(endpoint, ec);
	if (ec) {
		std::cerr << "bind failed: " << ec.message() << "\n";
		std::cerr << "endpoint: " << endpoint << "\n";
		return false;
	}
	return true;
}

bool CSocket::Listen(int backlog) {
	boost::system::error_code ec;
	impl_->acceptor.listen(backlog);
	if (ec) {
		std::cerr << "listen failed: " << ec.message() << "\n";
		std::cerr << "endpoint: " << impl_->acceptor.local_endpoint() << "\n";
		return false;
	}
	return true;
}

bool CSocket::Accept(CSocket& sock) {
	boost::system::error_code ec;
	auto socket = impl_->acceptor.accept(ec);
	if (ec) {
		std::cerr << "accept failed: " << ec.message() << "\n";
		std::cerr << "endpoint: " << impl_->acceptor.local_endpoint() << "\n";
		return false;
	}
	sock.impl_->io_context = impl_->io_context;
	sock.impl_->socket = std::move(socket);
	return true;
}

bool CSocket::Connect(const std::string& host, uint16_t port) {
	boost::system::error_code ec;
	tcp::resolver resolver(*impl_->io_context);

	auto endpoints = resolver.resolve(host, std::to_string(port), ec);
	if (ec) {
		std::cerr << "resolve failed: " << ec.message() << "\n";
		return false;
	}

	boost::asio::connect(impl_->socket, endpoints, ec);
	if (ec) {
		std::cerr << "connect failed: " << ec.message() << "\n";
		return false;
	}

	tcp::no_delay opt_tcp_no_delay(true);
	impl_->socket.set_option(opt_tcp_no_delay, ec);
	if (ec) {
		std::cerr << "socket set option TCP_NODELAY failed: " << ec.message() << "\n";
		return false;
	}
	return true;
}

size_t CSocket::Receive(void* buf, size_t bytes) {
	boost::system::error_code ec;
	auto bytes_transferred =
		boost::asio::read(impl_->socket, boost::asio::buffer(buf, bytes), ec);
	if (ec) {
		std::cerr << "read failed: " << ec.message() << "\n";
	}
	return bytes_transferred;
}

size_t CSocket::Send(const void* buf, size_t bytes) {
	boost::system::error_code ec;
	auto bytes_transferred =
		boost::asio::write(impl_->socket, boost::asio::buffer(buf, bytes), ec);
	if (ec) {
		std::cerr << "write failed: " << ec.message() << "\n";
	}
	return bytes_transferred;
}
