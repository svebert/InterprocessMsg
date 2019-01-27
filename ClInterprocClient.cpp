#include<vector>
#include<string>
#include <iostream>
#include "ClInterprocClient.hpp"

ClInterprocClient::ClInterprocClient(std::string sSenderName) :
	ClInterprocBase(sSenderName)
{
}

ClInterprocClient::~ClInterprocClient()
{
}

void ClInterprocClient::Send(StMessage & stMessage)
{
	if(!m_spMsgQueue)
	{
		if (OpenQueue()!=0)
		{
			std::cout << "ClInterprocClient::Send: Failed to open message queue for sending" << std::endl;
			throw std::runtime_error("ClInterprocClient::Send: failed to open message queue for sending ");
		}
	}
	std::string sEncoded = stMessage.to_string();
	m_spMsgQueue->send(sEncoded.data(), sEncoded.size(), 0);
}
