/**
 \file 		channel.cpp
 \author 	michael.zohner@ec-spride.de
 \copyright	ABY - A Framework for Efficient Mixed-protocol Secure Two-party Computation
			Copyright (C) 2019 Engineering Cryptographic Protocols Group, TU Darmstadt
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
 */

#include "channel.h"

#include "typedefs.h"
#include "rcvthread.h"
#include "sndthread.h"
#include <cassert>
#include <cstring>


channel::channel(uint8_t channelid, RcvThread* rcver, SndThread* snder)
	: m_bChannelID(channelid), m_cRcver(rcver), m_cSnder(snder),
	m_eRcved(std::make_unique<CEvent>()), m_eFin(std::make_unique<CEvent>()),
	m_bSndAlive(true), m_bRcvAlive(true),
	m_qRcvedBlocks(rcver->add_listener(channelid, m_eRcved.get(), m_eFin.get())),
	m_qRcvedBlocks_mutex_(rcver->get_listener_mutex(channelid))
{
	assert(rcver->getlock() == snder->getlock());
}

channel::~channel() {
	if(m_bRcvAlive) {
		m_cRcver->remove_listener(m_bChannelID);
	}
}

void channel::send(uint8_t* buf, uint64_t nbytes) {
	assert(m_bSndAlive);
	m_cSnder->add_snd_task(m_bChannelID, nbytes, buf);
}


void channel::blocking_send(CEvent* eventcaller, uint8_t* buf, uint64_t nbytes) {
	assert(m_bSndAlive);
	m_cSnder->add_event_snd_task(eventcaller, m_bChannelID, nbytes, buf);
	eventcaller->Wait();
}

void channel::send_id_len(uint8_t* buf, uint64_t nbytes, uint64_t id, uint64_t len) {
	assert(m_bSndAlive);
	m_cSnder->add_snd_task_start_len(m_bChannelID, nbytes, buf, id, len);
}

void channel::blocking_send_id_len(CEvent* eventcaller, uint8_t* buf, uint64_t nbytes, uint64_t id, uint64_t len) {
	assert(m_bSndAlive);
	m_cSnder->add_event_snd_task_start_len(eventcaller, m_bChannelID, nbytes, buf, id, len);
	eventcaller->Wait();
}

//buf needs to be freed, data contains the payload
uint8_t* channel::blocking_receive_id_len(uint8_t** data, uint64_t* id, uint64_t* len) {
	uint8_t* buf = blocking_receive();
	*data = buf;
	*id = *((uint64_t*) *data);
	(*data)  += sizeof(uint64_t);
	*len = *((uint64_t*) *data);
	(*data) += sizeof(uint64_t);

	return buf;
}

bool channel::queue_empty() const {
	std::lock_guard<std::mutex> lock(m_qRcvedBlocks_mutex_);
	bool qempty = m_qRcvedBlocks->empty();
	return qempty;
}

uint8_t* channel::blocking_receive() {
	assert(m_bRcvAlive);
	while(queue_empty())
		m_eRcved->Wait();
	rcv_ctx* ret = nullptr;
	uint8_t* ret_block = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_qRcvedBlocks_mutex_);
		ret = (rcv_ctx*) m_qRcvedBlocks->front();
		ret_block = ret->buf;
		m_qRcvedBlocks->pop();
	}
	free(ret);

	return ret_block;
}

void channel::blocking_receive(uint8_t* rcvbuf, uint64_t rcvsize) {
	assert(m_bRcvAlive);
	while(queue_empty())
		m_eRcved->Wait();

	std::unique_lock<std::mutex> lock(m_qRcvedBlocks_mutex_);
	rcv_ctx* ret = (rcv_ctx*) m_qRcvedBlocks->front();
	uint8_t* ret_block = ret->buf;
	uint64_t rcved_this_call = ret->rcvbytes;
	if(rcved_this_call == rcvsize) {
		m_qRcvedBlocks->pop();
		lock.unlock();
		free(ret);
	} else if(rcvsize < rcved_this_call) {
		//if the block contains too much data, copy only the receive size
		ret->rcvbytes -= rcvsize;
		uint8_t* newbuf = (uint8_t*) malloc(ret->rcvbytes);
		memcpy(newbuf, ret->buf+rcvsize, ret->rcvbytes);
		ret->buf = newbuf;
		lock.unlock();
		rcved_this_call = rcvsize;
	} else {
		//I want to receive more data than are in that block. Perform recursive call (might become troublesome for too many recursion steps)
		m_qRcvedBlocks->pop();
		lock.unlock();
		free(ret);
		uint8_t* new_rcvbuf_start = rcvbuf + rcved_this_call;
		uint64_t new_rcvsize = rcvsize -rcved_this_call;

		blocking_receive(new_rcvbuf_start, new_rcvsize);
	}
	memcpy(rcvbuf, ret_block, rcved_this_call);
	free(ret_block);
}


bool channel::is_alive() {
	return (!(queue_empty() && m_eFin->IsSet()));
}

bool channel::data_available() {
	return !queue_empty();
}

void channel::signal_end() {
	m_cSnder->signal_end(m_bChannelID);
	m_bSndAlive = false;
}

void channel::wait_for_fin() {
	m_eFin->Wait();
	m_bRcvAlive = false;
}

void channel::synchronize_end() {
	if(m_bSndAlive)
		signal_end();
	if(m_bRcvAlive)
		m_cRcver->flush_queue(m_bChannelID);
	if(m_bRcvAlive)
		wait_for_fin();

}
