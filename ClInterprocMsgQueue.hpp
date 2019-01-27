#pragma once
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <string>
#include <vector>

constexpr auto MESSAGESIZE = 1024;
constexpr auto MESSAGE_QUEUE_SIZE = 20;

struct StMessage
{
	enum EMessageType
	{
		command = 0,
		status = 1,
		data = 2,
		undefined = 3
	};
	//instantiate a message
	StMessage(std::string sSender = "Unknown", std::string sAddress = "Unknown", std::string sTopic = "default", std::string sMessage = "", EMessageType eMassageType = EMessageType::undefined) :
		m_sSender(sSender), m_sAddress(sAddress), m_sHandler(sTopic), m_sMessage(sMessage), m_eMessageType(eMassageType)
	{
		if (to_string().size() > MESSAGESIZE) { throw std::runtime_error("message must not exceed length of " + std::to_string(MESSAGESIZE)); }
	};
	//message string
	std::string m_sMessage;
	//message type
	EMessageType m_eMessageType = EMessageType::undefined;
	//sender 
	std::string m_sSender;
	//address
	std::string m_sAddress;
	//topic (defines which handler will be called when message is received)
	std::string m_sHandler;
	//serialize message
	const std::string to_string() {
		return "from: " + m_sSender + "; adress: " + m_sAddress + "; message={" + m_sMessage + "}; handler=" + m_sHandler + ";type=" + std::to_string(m_eMessageType) + ";";
	};

	bool operator==(const StMessage& rhs)
	{
		return m_sMessage == rhs.m_sMessage && m_eMessageType == rhs.m_eMessageType
			&& m_sSender == rhs.m_sSender && m_sAddress == rhs.m_sAddress && m_sHandler == rhs.m_sHandler;
	};

	void parse(std::string sEncoded)
	{
		if (sEncoded == "")
		{
			return;
		}
		std::string sSearchTerm1 = "from: ";
		std::string sSearchTerm2 = "; adress: ";
		size_t nStart = sEncoded.find(sSearchTerm1) + sSearchTerm1.size();
		size_t nEnd = sEncoded.find(sSearchTerm2);
		m_sSender = sEncoded.substr(nStart, nEnd - nStart);

		nStart = nEnd + sSearchTerm2.size();
		sSearchTerm2 = "; message={";
		nEnd = sEncoded.find(sSearchTerm2, nStart);
		m_sAddress = sEncoded.substr(nStart, nEnd - nStart);

		nStart = nEnd + sSearchTerm2.size();
		sSearchTerm2 = "}; handler=";
		nEnd = sEncoded.find(sSearchTerm2, nStart);
		m_sMessage = sEncoded.substr(nStart, nEnd - nStart);

		nStart = nEnd + sSearchTerm2.size();
		sSearchTerm2 = ";type=";
		nEnd = sEncoded.find(sSearchTerm2, nStart);
		m_sHandler = sEncoded.substr(nStart, nEnd - nStart);

		nStart = nEnd + sSearchTerm2.size();
		sSearchTerm2 = ";";
		nEnd = sEncoded.find(sSearchTerm2, nStart);
		m_eMessageType = static_cast<EMessageType>(std::atoi(sEncoded.substr(nStart, nEnd - nStart).c_str()));
	};
};

class ClInterprocMsgQueue
{
	struct StSharedMem
	{
		StSharedMem() :
			oMutex(),
			oCondNewMsg()
		{
			for (int nIdx = 0; nIdx < MESSAGE_QUEUE_SIZE; ++nIdx) { nMsgSlotFlag[nIdx] = 0; }
		};
		boost::interprocess::interprocess_mutex oMutex;
		boost::interprocess::interprocess_condition oCondNewMsg;
		char oMsgsBuffer[MESSAGESIZE*MESSAGE_QUEUE_SIZE];
		int nMsgSlotFlag[MESSAGE_QUEUE_SIZE];
	};
public:
	ClInterprocMsgQueue(std::string sSharedMemName, bool bCreate);
	~ClInterprocMsgQueue();
	void receive(std::vector<StMessage> & vMessages);
	void send(const void * pBuffer, size_t nBufferSize, int priority);
private:
	void RemoveSharedMem();
	boost::interprocess::shared_memory_object * m_pSharedMem;
	bool m_bCreated;
	std::string m_sSharedMemName;
};