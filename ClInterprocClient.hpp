#pragma once
#include "ClInterprocBase.hpp"

#include <thread>
#include <map>
#include <string>
class ClInterprocClient : ClInterprocBase
{
public:
	typedef long(*fnController)(StMessage &, void*);
	ClInterprocClient(std::string sSenderName = "msg_queue1");
	~ClInterprocClient();
	//send a message on sSenderName message queue
	void Send(StMessage & stMessage);
};
