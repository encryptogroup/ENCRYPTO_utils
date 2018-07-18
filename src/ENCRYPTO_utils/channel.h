/*
 * channel.h
 *
 *  Created on: Mar 9, 2015
 *      Author: mzohner
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>

class RcvThread;
class SndThread;
struct rcv_ctx;
class CEvent;
class CLock;

class channel {
public:
	channel(uint8_t channelid, RcvThread* rcver, SndThread* snder);

	~channel();

	void send(uint8_t* buf, uint64_t nbytes);
	void send_id_len(uint8_t* buf, uint64_t nbytes, uint64_t id, uint64_t len);

	//buf needs to be freed, data contains the payload
	uint8_t* blocking_receive_id_len(uint8_t** data, uint64_t* id, uint64_t* len);

    bool queue_empty() const;

	uint8_t* blocking_receive();

	void blocking_receive(uint8_t* rcvbuf, uint64_t rcvsize);

	bool is_alive();

	bool data_available();

	void signal_end();

	void wait_for_fin();

	void synchronize_end();

private:
	uint8_t m_bChannelID;
	RcvThread* m_cRcver;
	SndThread* m_cSnder;
	std::unique_ptr<CEvent> m_eRcved;
	std::unique_ptr<CEvent> m_eFin;
	bool m_bSndAlive;
	bool m_bRcvAlive;
	std::queue<rcv_ctx*>* m_qRcvedBlocks;
	std::mutex& m_qRcvedBlocks_mutex_;
};


#endif /* CHANNEL_H_ */
