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
		cout << "Unsetting channel " << (uint32_t) channelid << endl;
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
	cout << "Registering listener on channel " << (uint32_t) channelid << endl;
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

	//cout << "Successfully registered on channel " << (uint32_t) channelid << endl;

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
		//cout << "Starting to receive data" << endl;
		rcv_len = 0;
		rcv_len += mysock->Receive(&channelid, sizeof(uint8_t));
		rcv_len += mysock->Receive(&rcvbytelen, sizeof(uint64_t));

		if(rcv_len > 0) {
#ifdef DEBUG_RECEIVE_THREAD
			cout << "Received value on channel " << (uint32_t) channelid << " with " << rcvbytelen <<
					" bytes length (" << rcv_len << ")" << endl;
#endif

			if(channelid == ADMIN_CHANNEL) {
				std::vector<uint8_t> tmprcvbuf(rcvbytelen);
				mysock->Receive(tmprcvbuf.data(), rcvbytelen);

				//TODO: Right now finish, can be used for other maintenance tasks
				//cout << "Got message on Admin channel, shutting down" << endl;
#ifdef DEBUG_RECEIVE_THREAD
				cout << "Receiver thread is being killed" << endl;
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
