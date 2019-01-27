#include "ClInterprocBase.hpp"
ClInterprocBase::ClInterprocBase(std::string sMsgQueueName) :
	m_sMsgQueueName(sMsgQueueName),
	m_spMsgQueue(),
	m_oStopMessage("self", "self", "system:stop", "stop-listener", StMessage::EMessageType::command)
{
}

ClInterprocBase::~ClInterprocBase()
{
}

EInterprocError ClInterprocBase::OpenQueue(long nMsgMax, long nMsgSizeMax)
{
	try
	{
		m_spMsgQueue.reset(new ClInterprocMsgQueue(m_sMsgQueueName, false));
	}
	catch(std::exception & oException)
	{
		return INTERPROC_ERROR_OPEN;
	}
	return INTERPROC_SUCCESS;
}

EInterprocError ClInterprocBase::CreateQueue(long nMaxMsg, long nMsgSizeMax)
{
	try
	{
		m_spMsgQueue.reset(new ClInterprocMsgQueue(m_sMsgQueueName, true));
	}
	catch (std::exception & oException)
	{
		return INTERPROC_ERROR_CREATE;
	}
	return INTERPROC_SUCCESS;
}
