#pragma once
#include "ClInterprocMsgQueue.hpp"
#include "InterprocError.hpp"
#include <vector>

class ClInterprocBase
{
public:
	ClInterprocBase(std::string sMsqQueueName = "msg_queue1");
	~ClInterprocBase();

protected:
	EInterprocError OpenQueue(long nMaxMsg = 100, long nMsgSizeMax = MESSAGESIZE);
	EInterprocError CreateQueue(long nMaxMsg = 100, long nMsgSizeMax = MESSAGESIZE);
	std::shared_ptr<ClInterprocMsgQueue> m_spMsgQueue;
	std::string m_sMsgQueueName;
	StMessage m_oStopMessage;
};
