#include "ClInterprocMsgManager.hpp"
ClInterprocMsgManager::ClInterprocMsgManager(const std::string sSenderName) :
	m_sSenderName(sSenderName),
	m_oInterprocClient(sSenderName),
	m_vspInterprocListener()
{
}

ClInterprocMsgManager::~ClInterprocMsgManager()
{
}

EInterprocError ClInterprocMsgManager::Send(StMessage stMsg)
{
	try
	{
		m_oInterprocClient.Send(stMsg);
		return INTERPROC_SUCCESS;
	}
	catch(std::exception & oExeption)
	{
		return INTERPROC_ERROR_SEND;
	}
}

EInterprocError ClInterprocMsgManager::Subscribe(const std::string sAddressName, ClInterprocListener::fnNewMessage fnCallbackOnNewMsg, void * pCBCtx, const std::string sTopic)
{
	//loop over all listener and add new topic handler
	for (auto &oListener : m_vspInterprocListener)
	{
		if (oListener->GetListenToName() == sAddressName)
		{
			oListener->RegisterCallback(fnCallbackOnNewMsg, pCBCtx, sTopic);
			return INTERPROC_SUCCESS;
		}
	}
	//if listener to sAddressName does not exist, instantiate a new listener 
	m_vspInterprocListener.push_back(std::unique_ptr<ClInterprocListener>(new ClInterprocListener(sAddressName)));
	//... and add a new topic handler
	m_vspInterprocListener.back()->RegisterCallback(fnCallbackOnNewMsg, pCBCtx, sTopic);
	return INTERPROC_SUCCESS;
}

EInterprocError ClInterprocMsgManager::Start(const std::string sAddressName)
{
	for (auto &oListener : m_vspInterprocListener)
	{
		if (oListener->GetListenToName() == sAddressName || sAddressName == "")
		{
			oListener->Start();
		}
	}
	return INTERPROC_SUCCESS;
}
