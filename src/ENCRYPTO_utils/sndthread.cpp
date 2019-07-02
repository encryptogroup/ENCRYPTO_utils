/**
 \file 		sndthread.cpp
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

void SndThread::add_event_snd_task_start_len(CEvent* eventcaller, uint8_t channelid, uint64_t sndbytes, uint8_t* sndbuf, uint64_t startid, uint64_t len) {
	assert(channelid != ADMIN_CHANNEL);
	auto task = std::make_unique<snd_task>();
	task->channelid = channelid;
	task->eventcaller = eventcaller;
	size_t bytelen = sndbytes + 2 * sizeof(uint64_t);
	task->snd_buf.resize(bytelen);
	memcpy(task->snd_buf.data(), &startid, sizeof(uint64_t));
	memcpy(task->snd_buf.data()+sizeof(uint64_t), &len, sizeof(uint64_t));
	memcpy(task->snd_buf.data()+2*sizeof(uint64_t), sndbuf, sndbytes);

	//std::cout << "Adding a new task that is supposed to send " << task->bytelen << " bytes on channel " << (uint32_t) channelid  << std::endl;
	push_task(std::move(task));
}

void SndThread::add_snd_task_start_len(uint8_t channelid, uint64_t sndbytes, uint8_t* sndbuf, uint64_t startid, uint64_t len) {
	//Call the method blocking but since callback is nullptr nobody gets notified, other functionallity is equal
	add_event_snd_task_start_len(nullptr, channelid, sndbytes, sndbuf, startid, len);
}


void SndThread::add_event_snd_task(CEvent* eventcaller, uint8_t channelid, uint64_t sndbytes, uint8_t* sndbuf) {
	assert(channelid != ADMIN_CHANNEL);
	auto task = std::make_unique<snd_task>();
	task->channelid = channelid;
	task->eventcaller = eventcaller;
	task->snd_buf.resize(sndbytes);
	memcpy(task->snd_buf.data(), sndbuf, sndbytes);

	push_task(std::move(task));
	//std::cout << "Event set" << std::endl;

}

void SndThread::add_snd_task(uint8_t channelid, uint64_t sndbytes, uint8_t* sndbuf) {
	//Call the method blocking but since callback is nullptr nobody gets notified, other functionallity is equal
	add_event_snd_task(nullptr, channelid, sndbytes, sndbuf);
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
			if(task->eventcaller != nullptr) {
				task->eventcaller->Set();
			}
		}
	}
}
;
