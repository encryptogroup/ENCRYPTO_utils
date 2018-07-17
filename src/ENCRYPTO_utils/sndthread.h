/*
 * snd_thread.h
 *
 *  Created on: Mar 9, 2015
 *      Author: mzohner
 */

#ifndef SND_THREAD_H_
#define SND_THREAD_H_

#include "thread.h"
#include <memory>
#include <queue>

class CSocket;


class SndThread: public CThread {
public:
	SndThread(CSocket* sock, CLock *glock);

	void stop();

	~SndThread();

	CLock* getlock() const;

    void setlock(CLock *glock);

	void add_snd_task_start_len(uint8_t channelid, uint64_t sndbytes, uint8_t* sndbuf, uint64_t startid, uint64_t len);


	void add_snd_task(uint8_t channelid, uint64_t sndbytes, uint8_t* sndbuf);

	void signal_end(uint8_t channelid);

	void kill_task();

	void ThreadMain();

private:
	struct snd_task {
		uint8_t channelid;
		std::vector<uint8_t> snd_buf;
	};

	void push_task(std::unique_ptr<snd_task> task);

	CSocket* mysock;
	CLock* sndlock;
	std::unique_ptr<CEvent> send;
	std::queue<std::unique_ptr<snd_task>> send_tasks;
};



#endif /* SND_THREAD_H_ */
