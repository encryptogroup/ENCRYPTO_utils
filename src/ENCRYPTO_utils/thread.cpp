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
