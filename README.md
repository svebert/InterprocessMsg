## Description

InterprocessMsg is a C++11 library for bidirectional communication based on messages between two processes on the same computer.
The main dependency is <a href="https://www.boost.org/doc/libs/1_63_0/doc/html/interprocess.html" >boost interprocess</a>. 
InterprocessMsg is cross-platform compatible and was tested on Windows 10 (Intel processor) and Ubuntu Mate 18.04 (ARM processor).

## Prerequisites

This repository consists of the source code (.cpp and .hpp files) of the lib, which may be compiled with any compiler capabale of C++11, 
and <a href="https://visualstudio.microsoft.com/de/">Visual Studio</a> project files. The InterprocessMsg-Linux.vcxproj file may be used for a remote build on a ARM processor (like an <a href="https://www.hardkernel.com/">Odroid</a> or <a href="https://www.raspberrypi.org/">Raspberry-PI</a>)
InterprocessMsg.vcxproj is the project file for a standard Windows built with Visual Studio.

### Dependencies Ubuntu
Use 
```
sudo apt-get install libboost-all-dev
```
to install boost on your ubuntu system, if it was not installed already.
### Dependencies Windows
I recommend <a href="https://github.com/Microsoft/vcpkg">vcpkg</a> to handle your dependencies on Windows. Just clone vcpkg on your PC and build it.
After that run vcpkg from commandline and install boost:
```
vcpkg install boost-interprocess
```

## Example usage
1. Instantiate a [ClInterproceMsgManager](ClInterprocMsgManager.hpp) within each process.

2. Subscribe handler functions to the manager to handle messages with a specific topics:
```c++
EInterprocError ClInterprocMsgManager::Subscribe(const std::string sAddressName, ClInterprocListener::fnNewMessage fnCallbackOnNewMsg, void * pCBCtx, const  std::string sTopic = "default");
```
The handler function prototype looks like:
```c++
	typedef long(*fnNewMessage)(StMessage &, void*);
```
with [StMessage](ClInterprocMsgQueue.hpp) being a simple struct, defining the message structure. 

3. start listening and wait for messages
```c++
EInterprocError ClInterprocMsgManager::Start(const std::string sAddressName = "");
```
4. send a message from process 1 to process 2
```c++
StMessage stMsg("process 1", "process 2", "default", "hello world!");
nErr = m_oInterProcMsgManager.Send(stMsg);
```
<b>Example for process 1</b>
```c++
//1.)
#include "ClInterprocMsgManager.hpp"
#include <iostream>
ClInterprocMsgManager m_oInterProcMsgManager("process1"); // this process is process 1
//2.)
m_oInterProcMsgManager.Subscribe("process 2", fndefaultMsgHandler, this); //fndefaultMsgHandler is a function, which will handle incoming messages from process 2 with the topic "default"
m_oInterProcMsgManager.Subscribe("process 2", fnstatusMsgHandler, this, "status"); //fnstatusMsgHandler is a function, which will handle incoming messages from process 2 with the topic "status"
//3.)
EInterprocError nErr =  m_oInterProcMsgManager.Start(); //this will spawn a thread, which will listen for new messages from process 2. If new messages are received, one of the subscribed handlers (2.) will be called
if (nErr == INTERPROC_SUCCESS)
{
	std::cout << "start listening to process 2" << std::endl;
}
else
{
	std::cout << "Error: failed listening to process 2!" << std::endl;
}
//4.
StMessage stMsg("process 1", "process 2", "default", "hello world!");
nErr = m_oInterProcMsgManager.Send(stMsg);
```

## Notes
* Internally one of the two communicating processes creates shared memory and both processes will access this memory. This shared memory is structured as a message queue.
* The subscribing process will create the shared memory, the sender will only open it.
* In case both processes listen and receive, two shared memory blocks will be created. Both processes will create one block and open one block.
* This lib was designed for interprocess communication between two processes, only. 
* Boost itsself provides <a href="https://www.boost.org/doc/libs/1_55_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.message_queue">interprocess.message_queue</a>. 
Unfortunatly it cannot handle a bidirectional communication, only unidirectional. Also undefined behaviour occurs when instantiating two boost-interprocess-message_queue. This is the actual reason for the developement of InterprocessMsg.

