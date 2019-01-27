#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include "ClInterprocMsgQueue.hpp"

ClInterprocMsgQueue::ClInterprocMsgQueue(std::string sSharedMemName, bool bCreate) :
	m_bCreated(bCreate),
	m_pSharedMem(nullptr),
	m_sSharedMemName(sSharedMemName)
{
	if (m_bCreated)
	{
		RemoveSharedMem();
		m_pSharedMem = new boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, m_sSharedMemName.c_str(), boost::interprocess::read_write);
		m_pSharedMem->truncate(sizeof(StSharedMem));
		boost::interprocess::mapped_region oRegion(*m_pSharedMem, boost::interprocess::read_write);
		void * pAddr = oRegion.get_address();
		//Construct the shared structure in memory
		StSharedMem * pTmp = new (pAddr) StSharedMem;
	}
	else
	{
		m_pSharedMem = new boost::interprocess::shared_memory_object(boost::interprocess::open_only, m_sSharedMemName.c_str(), boost::interprocess::read_write);
	}
}

void ClInterprocMsgQueue::RemoveSharedMem()
{
	boost::interprocess::shared_memory_object::remove(m_sSharedMemName.c_str());
}

ClInterprocMsgQueue::~ClInterprocMsgQueue()
{
	if (m_bCreated)
	{
		RemoveSharedMem();
	}
}

void ClInterprocMsgQueue::receive(std::vector<StMessage> & vMessages)
{
	boost::interprocess::mapped_region oRegion(*m_pSharedMem, boost::interprocess::read_write);
	StSharedMem * pSharedMem = static_cast<StSharedMem *>(oRegion.get_address());

	if (!pSharedMem)
	{
		throw std::runtime_error("ClInterprocMsgQueue::receive: Null pointer exception, m_pSharedMem is NULL");
	}

	{
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> oLock(pSharedMem->oMutex);
		pSharedMem->oCondNewMsg.wait(oLock);
		vMessages.clear();
		//count number of received messages in shared memory...
		size_t nMsgReceiveCount = 0;
		for (size_t nIdx = 0; nIdx < MESSAGE_QUEUE_SIZE; ++nIdx)
		{
			if (pSharedMem->nMsgSlotFlag[nIdx] == 1)
			{
				nMsgReceiveCount++;
			}
		}
		//and allocate local buffer to copy messages
		size_t nReceivedSize = MESSAGESIZE * nMsgReceiveCount;
		char * pBuffer = static_cast<char*>(malloc(nReceivedSize));

		size_t nMsgReceiverCounter = 0;
		for (size_t nIdxSharedBuffer = MESSAGE_QUEUE_SIZE; nIdxSharedBuffer >= 0; --nIdxSharedBuffer) //receive last message first
		{
			if (pSharedMem->nMsgSlotFlag[nIdxSharedBuffer] == 1)
			{
				//copy message to local buffer
				memcpy(pBuffer + nMsgReceiverCounter * MESSAGESIZE, &(pSharedMem->oMsgsBuffer[nIdxSharedBuffer*MESSAGESIZE]), MESSAGESIZE);
				//parse buffer to StMessage
				StMessage stMsg;
				stMsg.parse(std::string(pBuffer + nMsgReceiverCounter * MESSAGESIZE, pBuffer + (nMsgReceiverCounter + 1)*MESSAGESIZE));
				++nMsgReceiverCounter;
				//free this place in message queue	
				pSharedMem->nMsgSlotFlag[nIdxSharedBuffer] = 0; 
				//add message to return value
				vMessages.push_back(stMsg);
				//as we know how many messages are in the buffer, we can stop when all messages are received
				nMsgReceiveCount--;
			}
			if (nMsgReceiveCount == 0)
			{
				break;
			}
		}

		free(pBuffer);
	}
}

void ClInterprocMsgQueue::send(const void * pBuffer, size_t nBufferSize, int priority)
{
	if (!pBuffer)
	{
		throw std::runtime_error("ClInterprocMsgQueue::send: Null pointer exception, pBuffer is NULL");
	}

	boost::interprocess::mapped_region oRegion(*m_pSharedMem, boost::interprocess::read_write);
	StSharedMem * pTmp = static_cast<StSharedMem *>(oRegion.get_address());
	if (!m_pSharedMem)
	{
		throw std::runtime_error("ClInterprocMsgQueue::send: Null pointer exception, m_pSharedMem is NULL");
	}
	boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> oLock(pTmp->oMutex);
	size_t nIdxFree = MESSAGE_QUEUE_SIZE;
	for (size_t nIdx = 0; nIdx < MESSAGE_QUEUE_SIZE; ++nIdx)
	{
		if (pTmp->nMsgSlotFlag[nIdx] == 0)
		{
			nIdxFree = nIdx;
			break;
		}
	}
	if (nIdxFree >= MESSAGE_QUEUE_SIZE)
	{
		throw std::runtime_error("ClInterprocMsgQueue::send: Can't send, message queue is full.");
	}
	//copy msg to shared buffer
	char * pData = &(pTmp->oMsgsBuffer[nIdxFree*MESSAGESIZE]);
	memcpy(pData, pBuffer, nBufferSize);
	//notify
	pTmp->nMsgSlotFlag[nIdxFree] = 1;
	pTmp->oCondNewMsg.notify_one();
}
