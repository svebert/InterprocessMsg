#pragma once
#include "ClInterprocClient.hpp"
#include "ClInterprocListener.hpp"

#include <vector>

class ClInterprocMsgManager
{
public:
	ClInterprocMsgManager(const std::string sSenderName);
	~ClInterprocMsgManager();
	//send message to queue; First send will initiate queue; Other processes can only connect to sSenderName after the first message was send
	EInterprocError Send(StMessage stMsg);
	//subscribe to listen to other processes; In case an other process sends a message with the topic sTopic, fnCallbackOnNewMsg will be called
	EInterprocError Subscribe(const std::string sAddressName, ClInterprocListener::fnNewMessage fnCallbackOnNewMsg, void * pCBCtx, const  std::string sTopic = "default");
	//start listening to other processes; if no addressName is given, manager will start to listen to all subscribted processes
	EInterprocError Start(const std::string sAddressName = "");
private:
	std::string m_sSenderName;
	std::vector<std::unique_ptr<ClInterprocListener> > m_vspInterprocListener;
	ClInterprocClient m_oInterprocClient;
};