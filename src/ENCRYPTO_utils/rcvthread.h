/*
 * rcv_thread.h
 *
 *  Created on: Mar 9, 2015
 *      Author: mzohner
 */

#ifndef RCV_THREAD_H_
#define RCV_THREAD_H_

#include "constants.h"
#include "thread.h"
#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>

class CSocket;

struct rcv_ctx {
	uint8_t *buf;
	uint64_t rcvbytes;
};



class RcvThread: public CThread {
public:
	RcvThread(CSocket* sock, CLock* glock);
	~RcvThread();

	CLock* getlock() const;

    void setlock(CLock *glock);

	void flush_queue(uint8_t channelid);

	void remove_listener(uint8_t channelid);

	std::queue<rcv_ctx*>* add_listener(uint8_t channelid, CEvent* rcv_event, CEvent* fin_event);
	std::mutex& get_listener_mutex(uint8_t channelid);

	void ThreadMain();

private:
	//A receive task listens to a particular id and writes incoming data on that id into rcv_buf and triggers event
	struct rcv_task {
		std::queue<rcv_ctx*> rcv_buf;
		std::mutex rcv_buf_mutex;
		//std::queue<uint64_t> rcvbytes;
		CEvent* rcv_event;
		CEvent* fin_event;
		bool inuse;
		bool forward_notify_fin;
	};

	CLock* rcvlock;
	CSocket* mysock;
	std::array<rcv_task, MAX_NUM_COMM_CHANNELS> listeners;
};



#endif /* RCV_THREAD_H_ */
