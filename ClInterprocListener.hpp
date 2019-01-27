#pragma once
#include "ClInterprocBase.hpp"
#include <thread>
#include <mutex>

class ClInterprocListener : ClInterprocBase
{
public:
	typedef long(*fnNewMessage)(StMessage &, void*);
	//instantiate ClInterprocListener; This listener will listen to a queue in shared memory, with the name sListenTo
	ClInterprocListener(std::string sListenTo = "msg_queue1");
	~ClInterprocListener();
	//start listening to sMsgQueue 
	EInterprocError Start();
	//stop listening to sMsgQueue
	EInterprocError Stop();
	//register callback function;
	//If listener receives new message with sTopic, it will call the registered function fnNewMessageCb 
	//pCtx is a user-defined context, usually a this-pointer of the instance from which fnNewMessageCb originates
	long RegisterCallback(fnNewMessage fnNewMessageCb, void * pCtx, std::string sTopic);
	//returns the message queue name, which was passed on construction
	std::string GetListenToName() const;
private:
	static void ListenerThread(void * pCtx);
	std::vector<std::pair< std::pair<fnNewMessage, void *>, std::string> > m_vPairCallbackFunctions;
	bool m_bListen = false;
	std::thread m_oListenerThread;
};
