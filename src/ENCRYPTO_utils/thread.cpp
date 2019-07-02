/**
 \file 		thread.cpp
 \author 	Seung Geol Choi
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

#include "thread.h"
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>

CThread::CThread() : m_bRunning(false) {
}
CThread::~CThread() {
	assert(!m_bRunning);
}

bool CThread::Start() {
	thread_ = std::thread([this] { ThreadMain(); });
	m_bRunning = true;
	return true;
}

bool CThread::Wait() {
	if (!m_bRunning)
		return true;
	m_bRunning = false;
	thread_.join();
	return true;
}

bool CThread::IsRunning() const {
	return m_bRunning;
}

void CLock::Lock() {
	mutex_.lock();
}
void CLock::Unlock() {
	mutex_.unlock();
}

void CLock::lock() {
	Lock();
}
void CLock::unlock() {
	Unlock();
}


CEvent::CEvent(bool bManualReset, bool bInitialSet)
: m_bManual(bManualReset), m_bSet(bInitialSet)
{
}

bool CEvent::Set() {
	std::unique_lock<std::mutex> lock(mutex_);
	if (m_bSet)
		return true;

	m_bSet = true;
	lock.unlock();
	cv_.notify_one();
	return true;
}

bool CEvent::Wait() {
	std::unique_lock<std::mutex> lock(mutex_);
	cv_.wait(lock, [this]{ return m_bSet; });

	if (!m_bManual)
		m_bSet = false;
	return true;
}

bool CEvent::IsSet() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return m_bSet;
}

bool CEvent::Reset() {
	std::lock_guard<std::mutex> lock(mutex_);
	m_bSet = false;
	return true;
}
