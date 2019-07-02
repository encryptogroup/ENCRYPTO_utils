/**
 \file 		rcvthread.cpp
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
 \brief		Receiver Thread Implementation
 */


#include "rcvthread.h"
#include "typedefs.h"
#include "constants.h"
#include "socket.h"
#include <cassert>
#include <cstdlib>
#include <iostream>


RcvThread::RcvThread(CSocket* sock, CLock *glock)
	:rcvlock(glock),  mysock(sock), listeners()
{
	listeners[ADMIN_CHANNEL].inuse = true;
}

RcvThread::~RcvThread() {
	this->Wait();
	for(size_t i = 0; i < listeners.size(); i++) {
		flush_queue(i);
	}
	//delete rcvlock;
}

CLock* RcvThread::getlock() const {
	return rcvlock;
}

void RcvThread::setlock(CLock *glock) {
	rcvlock = glock;
}

void RcvThread::flush_queue(uint8_t channelid) {
	std::lock_guard<std::mutex> lock(listeners[channelid].rcv_buf_mutex);
	while(!listeners[channelid].rcv_buf.empty()) {
		rcv_ctx* tmp = listeners[channelid].rcv_buf.front();
		free(tmp->buf);
		free(tmp);
		listeners[channelid].rcv_buf.pop();
	}
}

void RcvThread::remove_listener(uint8_t channelid) {
	rcvlock->Lock();
	if(listeners[channelid].inuse) {
		listeners[channelid].fin_event->Set();
		listeners[channelid].inuse = false;

#ifdef DEBUG_RECEIVE_THREAD
		std::cout << "Unsetting channel " << (uint32_t) channelid << std::endl;
#endif
	} else {
		listeners[channelid].forward_notify_fin = true;
	}
	rcvlock->Unlock();

}

std::queue<rcv_ctx*>*
RcvThread::add_listener(uint8_t channelid, CEvent* rcv_event, CEvent* fin_event) {
	rcvlock->Lock();
#ifdef DEBUG_RECEIVE_THREAD
	std::cout << "Registering listener on channel " << (uint32_t) channelid << std::endl;
#endif

	if(listeners[channelid].inuse || channelid == ADMIN_CHANNEL) {
		std::cerr << "A listener has already been registered on channel " << (uint32_t) channelid << std::endl;
		assert(!listeners[channelid].inuse);
		assert(channelid != ADMIN_CHANNEL);
	}

	//listeners[channelid].rcv_buf = rcv_buf;
	listeners[channelid].rcv_event = rcv_event;
	listeners[channelid].fin_event = fin_event;
	listeners[channelid].inuse = true;
//		assert(listeners[channelid].rcv_buf->empty());

	//std::cout << "Successfully registered on channel " << (uint32_t) channelid << std::endl;

	rcvlock->Unlock();

	if(listeners[channelid].forward_notify_fin) {
		listeners[channelid].forward_notify_fin = false;
		remove_listener(channelid);
	}
	return &listeners[channelid].rcv_buf;
}

std::mutex& RcvThread::get_listener_mutex(uint8_t channelid)
{
	return listeners[channelid].rcv_buf_mutex;
}


void RcvThread::ThreadMain() {
	uint8_t channelid;
	uint64_t rcvbytelen;
	uint64_t rcv_len;
	while(true) {
		//std::cout << "Starting to receive data" << std::endl;
		rcv_len = 0;
		rcv_len += mysock->Receive(&channelid, sizeof(uint8_t));
		rcv_len += mysock->Receive(&rcvbytelen, sizeof(uint64_t));

		if(rcv_len > 0) {
#ifdef DEBUG_RECEIVE_THREAD
			std::cout << "Received value on channel " << (uint32_t) channelid << " with " << rcvbytelen <<
					" bytes length (" << rcv_len << ")" << std::endl;
#endif

			if(channelid == ADMIN_CHANNEL) {
				std::vector<uint8_t> tmprcvbuf(rcvbytelen);
				mysock->Receive(tmprcvbuf.data(), rcvbytelen);

				//TODO: Right now finish, can be used for other maintenance tasks
				//std::cout << "Got message on Admin channel, shutting down" << std::endl;
#ifdef DEBUG_RECEIVE_THREAD
				std::cout << "Receiver thread is being killed" << std::endl;
#endif
				return;//continue;
			}

			if(rcvbytelen == 0) {
				remove_listener(channelid);
			} else {
				rcv_ctx* rcv_buf = (rcv_ctx*) malloc(sizeof(rcv_ctx));
				rcv_buf->buf = (uint8_t*) malloc(rcvbytelen);
				rcv_buf->rcvbytes = rcvbytelen;

				mysock->Receive(rcv_buf->buf, rcvbytelen);
				rcvlock->Lock();

				{
					std::lock_guard<std::mutex> lock(listeners[channelid].rcv_buf_mutex);
					listeners[channelid].rcv_buf.push(rcv_buf);
				}

				bool cond = listeners[channelid].inuse;
				rcvlock->Unlock();

				if(cond)
					listeners[channelid].rcv_event->Set();
			}
		} else {
			// We received 0 bytes, probably due to some major error. Just return.
			// TODO: Probably add some more elaborate error handling.
			return;
		}

	}

}
