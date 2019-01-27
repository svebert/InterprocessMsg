#include "ClInterprocListener.hpp"
#include <vector>
#include <chrono>

ClInterprocListener::ClInterprocListener(std::string sListenTo) :
	ClInterprocBase(sListenTo),
	m_vPairCallbackFunctions(),
	m_bListen(false),
	m_oListenerThread()
{
}

ClInterprocListener::~ClInterprocListener()
{
	Stop();
}

EInterprocError ClInterprocListener::Start()
{
	Stop();
	if (!m_spMsgQueue)
	{
		EInterprocError nErr = CreateQueue();
		if (nErr != INTERPROC_SUCCESS)
		{
			return nErr;
		}
	}

	m_oListenerThread = std::thread(&ClInterprocListener::ListenerThread, this);
	return INTERPROC_SUCCESS;
}

void ClInterprocListener::ListenerThread(void * pCtx)
{
	if (!pCtx)
	{
		throw std::runtime_error("ClInerprocListener::ListenerThread: null pointer exception, context of ListenerThread is missing");
	}
	ClInterprocListener * pInterprocListener = static_cast<ClInterprocListener*>(pCtx);

	pInterprocListener->m_bListen = true;

	if (!pInterprocListener->m_spMsgQueue)
	{
		throw std::runtime_error("ClInerprocListener::ListenerThread: null pointer exception: Shared memory for interprocess communication does not exist");
	}

	while (pInterprocListener->m_bListen)
	{
		std::size_t nRecvdSize=0;
		unsigned int priority=0;
		size_t nBuffersize = 0;
		char * pBuffer = nullptr;

		std::vector<StMessage> vMsgs;
		pInterprocListener->m_spMsgQueue->receive(vMsgs);
		
		for (auto stMsg : vMsgs)
		{
			//stop this thread, if there was a stop message
			if (stMsg == pInterprocListener->m_oStopMessage)
			{
				return;
			}

			//loop over all registered callbacks and run them
			for (auto oPairCallbackFunction : pInterprocListener->m_vPairCallbackFunctions)
			{
				//only run callback with correct topic/handler-string
				if (stMsg.m_sHandler == oPairCallbackFunction.second)
				{
					//null pointer check of function
					if (oPairCallbackFunction.first.first)
					{
						oPairCallbackFunction.first.first(stMsg, oPairCallbackFunction.first.second);
					}
				}
			}
		}
	}
}

EInterprocError ClInterprocListener::Stop()
{
	//Stop listener thread
	if (m_bListen)
	{
		m_bListen = false;
		
		if (m_spMsgQueue)
		{
			const std::string sStopMsgBuffer = m_oStopMessage.to_string();
			m_spMsgQueue->send(sStopMsgBuffer.data(), sStopMsgBuffer.size(), 0);
		}
		//wait for listener thread to return
		m_oListenerThread.join();
	}
	return INTERPROC_SUCCESS;
}

std::string ClInterprocListener::GetListenToName() const
{
	return m_sMsgQueueName;
}

long ClInterprocListener::RegisterCallback(fnNewMessage fnNewMessageCb, void * pCtx, std::string sTopic)
{
	auto oPair = std::pair< std::pair<fnNewMessage, void *>, std::string>(std::pair<fnNewMessage, void *>(fnNewMessageCb, pCtx), sTopic);
	m_vPairCallbackFunctions.push_back(oPair);
	return 0;
}
