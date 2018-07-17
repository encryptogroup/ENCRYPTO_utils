#include "sndthread.h"
#include "socket.h"
#include "constants.h"
#include <cassert>
#include <cstring>


SndThread::SndThread(CSocket* sock, CLock *glock)
: mysock(sock), sndlock(glock), send(std::make_unique<CEvent>())
{
}

void SndThread::stop() {
	kill_task();
}

SndThread::~SndThread() {
	kill_task();
	this->Wait();
}

CLock* SndThread::getlock() const {
	return sndlock;
}

void SndThread::setlock(CLock *glock) {
	sndlock = glock;
}

void SndThread::push_task(std::unique_ptr<snd_task> task)
{
	sndlock->Lock();
	send_tasks.push(std::move(task));
	sndlock->Unlock();
	send->Set();
}

void SndThread::add_snd_task_start_len(uint8_t channelid, uint64_t sndbytes, uint8_t* sndbuf, uint64_t startid, uint64_t len) {
	assert(channelid != ADMIN_CHANNEL);
	auto task = std::make_unique<snd_task>();
	task->channelid = channelid;
	size_t bytelen = sndbytes + 2 * sizeof(uint64_t);
	task->snd_buf.resize(bytelen);
	memcpy(task->snd_buf.data(), &startid, sizeof(uint64_t));
	memcpy(task->snd_buf.data()+sizeof(uint64_t), &len, sizeof(uint64_t));
	memcpy(task->snd_buf.data()+2*sizeof(uint64_t), sndbuf, sndbytes);

	//std::cout << "Adding a new task that is supposed to send " << task->bytelen << " bytes on channel " << (uint32_t) channelid  << std::endl;
	push_task(std::move(task));
}


void SndThread::add_snd_task(uint8_t channelid, uint64_t sndbytes, uint8_t* sndbuf) {
	assert(channelid != ADMIN_CHANNEL);
	auto task = std::make_unique<snd_task>();
	task->channelid = channelid;
	task->snd_buf.resize(sndbytes);
	memcpy(task->snd_buf.data(), sndbuf, sndbytes);

	push_task(std::move(task));
	//std::cout << "Event set" << std::endl;

}

void SndThread::signal_end(uint8_t channelid) {
	add_snd_task(channelid, 0, nullptr);
	//std::cout << "Signalling end on channel " << (uint32_t) channelid << std::endl;
}

void SndThread::kill_task() {
	auto task = std::make_unique<snd_task>();
	task->channelid = ADMIN_CHANNEL;
	task->snd_buf = {0};

	push_task(std::move(task));
#ifdef DEBUG_SEND_THREAD
	std::cout << "Killing channel " << (uint32_t) task->channelid << std::endl;
#endif
}

void SndThread::ThreadMain() {
	uint8_t channelid;
	uint32_t iters;
	bool run = true;
	bool empty = true;
	while(run) {
		sndlock->Lock();
		empty = send_tasks.empty();
		sndlock->Unlock();

		if(empty){
			send->Wait();
		}
		//std::cout << "Awoken" << std::endl;

		sndlock->Lock();
		iters = send_tasks.size();
		sndlock->Unlock();

		while((iters--) && run) {
			sndlock->Lock();
			auto task = std::move(send_tasks.front());
			send_tasks.pop();
			sndlock->Unlock();
			channelid = task->channelid;
			mysock->Send(&channelid, sizeof(uint8_t));
			uint64_t bytelen = task->snd_buf.size();
			mysock->Send(&bytelen, sizeof(bytelen));
			if(bytelen > 0) {
				mysock->Send(task->snd_buf.data(), task->snd_buf.size());
			}

#ifdef DEBUG_SEND_THREAD
			std::cout << "Sending on channel " <<  (uint32_t) channelid << " a message of " << task->bytelen << " bytes length" << std::endl;
#endif

			if(channelid == ADMIN_CHANNEL) {
				//delete sndlock;
				run = false;
			}
		}
	}
}
;
