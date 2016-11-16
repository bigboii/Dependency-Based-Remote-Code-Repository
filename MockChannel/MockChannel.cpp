/////////////////////////////////////////////////////////////////////////////
//  MockChannel.cpp - Reads words from a file                              //
// - build as DLL to show how C++\CLI client can use native code channel   //
// - MockChannel reads from sendQ and writes to recvQ                      //
//                                                                         //
//  ver 1.1                                                                //
//  Application:   CSE687 Project 4, Sp2016                                //
//  Author:        Young Kyu Kim, ykim127@syr.edu                          //
//  Source:        Jim Fawcett, CST 4-187, Syracuse University             //
//                 (315) 443-3948, jfawcett@twcny.rr.com                   //
/////////////////////////////////////////////////////////////////////////////

#define IN_DLL
#include "MockChannel.h"
#include "RepositoryHandler.h"
#include "ClientHandler.h"

#include "../MsgClient/MsgClient.h"
using BQueue = BlockingQueue < HttpMessage >;

/*
	SENDER
*/
class Sendr : public ISendr
{
public:
	Sendr(size_t servernum);
	void postMessage(const HttpMessage& msg);
	void sendFile(size_t bytes, char* buffer);
	void startConnecting();
	SocketConnecter& getSocketConnecter();
	BQueue& queue();

	void updateFilePath(std::string path);      //for wpf
	std::string getFilePath();  //for wpf
private:
	BQueue sendQ_;
	SocketConnecter sc;
	std::thread sendThread;
	size_t serverPortNum;
	std::string filesPath;
};

Sendr::Sendr(size_t servernum)                                //Constructor that initializes SocketConnector
{
	serverPortNum = servernum;
	sendThread = std::thread([this]             //sendThread constantly checks sendQ to send messages to server
	{
		while (true)
		{
			if (sendQ_.size() > 0)
			{
				//std::cout << "\n  channel deQing message from Send Q";
				HttpMessage msg = sendQ_.deQ();  // will block here so send quit message when stopping

																				 //Send Message using socket
				std::string msgString = msg.toString();
				sc.send(msgString.size(), (Socket::byte*)msgString.c_str());
			}
		}
		std::cout << "\n  Server stopping\n\n";
	});
}

//establishes socket connection with peer
void Sendr::startConnecting()
{
	while (!sc.connect("localhost", serverPortNum))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	Show::write("\n *** SocketConnector connected peer's listener");
}

//Post Message
void Sendr::postMessage(const HttpMessage& msg)
{
	sendQ_.enQ(msg);
}

//send file
void Sendr::sendFile(size_t bytes, char* buffer)
{
	sc.send(bytes, buffer);
}

//return reference to socket connector
SocketConnecter& Sendr::getSocketConnecter()
{
	return sc;
}

//return reference to sendQ
BQueue& Sendr::queue() { return sendQ_; }

//Update Client's filepath 
void Sendr::updateFilePath(std::string path)
{
	filesPath = path;
}

//return filepath
std::string Sendr::getFilePath()
{
	return filesPath;
}

/*
	RECVR
*/

class Recvr : public IRecvr
{
public:
	Recvr(size_t portNum, Socket::IpVer ip);
	HttpMessage getMessage();
	void enQ(HttpMessage msg);
	//SocketListener& getSocketListener();
	//void startListening(std::function<void(IRecvr* irecvr)> func);
	void startListening(ISendr* sendr, IRecvr* recvr, std::string handlerName);
	int getQSize();

	BQueue& queue();
private:
	BQueue recvQ_;
	SocketListener sl;
	RepositoryHandler* rh;
	ClientHandler* ch;
};

Recvr::Recvr(size_t portNum, Socket::IpVer ip) : sl(portNum, ip)               //constructor: set my portNum
{

}


void Recvr::startListening(ISendr* sendr, IRecvr* recvr, std::string handlerName)
{
	if (handlerName == "Repository")
	{
		rh = new RepositoryHandler(sendr, recvr);
		sl.start(*rh);
	}
	if (handlerName == "Client")
	{
		ch = new ClientHandler(sendr, recvr);
		sl.start(*ch);
	}

}



//Returns reference to SocketListener
/*
SocketListener& Recvr::getSocketListener()
{
	return sl;
}
*/

HttpMessage Recvr::getMessage()
{
	return recvQ_.deQ();
}

//enQ recieved and processed http message from sender into recvQ
void Recvr::enQ(HttpMessage msg)
{
	recvQ_.enQ(msg);
}

BQueue& Recvr::queue()
{
	return recvQ_;
}

//returns size of Q
int Recvr::getQSize()
{
	return recvQ_.size();
}

/*
	MOCK CHANNEL
*/
class MockChannel : public IMockChannel
{
public:
	MockChannel(ISendr* pSendr, IRecvr* pRecvr/*, size_t c, size_t s*/);
	void start();
	void stop();
	void updateFilePath(std::string path);
	void sendFile(std::string filename, std::string pkgname);
private:
	std::thread thread_;
	ISendr* pISendr_;
	IRecvr* pIRecvr_;
	bool stop_ = false;
	MsgClient c1;
	//size_t clientPortNum;
	//size_t serverPortNum;
};

//----< pass pointers to Sender and Receiver >-------------------------------

MockChannel::MockChannel(ISendr* pSendr, IRecvr* pRecvr/*, size_t cnum, size_t snum*/) : pISendr_(pSendr), pIRecvr_(pRecvr)/*, clientPortNum(cnum), serverPortNum(snum)*/{}

//----< creates thread to read from sendQ and echo back to the recvQ >-------
void MockChannel::start()
{
	std::cout << "\n  MockChannel starting up";
	thread_ = std::thread([this]                                   //Q's MockChannel implementation
	{
		Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);              //1. Convert ISendr & IRecvr to child version
		Recvr* pRecvr = dynamic_cast<Recvr*>(pIRecvr_);              
		if (pSendr == nullptr || pRecvr == nullptr)
		{
			std::cout << "\n  failed to start Mock Channel\n\n";
			return;
		}
		SocketSystem ss;                               		//2. Setup MsgClient
		c1.setChannelReference(pSendr, pRecvr);           // Client's address: 9080, initialize ISendr + IRecvr 
		std::thread t1(
			[&]() { c1.execute(100, 1); }                    // 20 messages 100 millisec apart
		);

		//3. Keep Thread running logic
		while (true)
		{

		}
	});
}

//----< signal server thread to stop >---------------------------------------
void MockChannel::stop() { stop_ = true; }

//----< Update File Path >---------------------------------------
void MockChannel::updateFilePath(std::string path)
{
	Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
	pSendr->updateFilePath(path);
}

void MockChannel::sendFile(std::string filename, std::string pkgname)
{
	c1.sendFile(filename, pkgname);
}


/*
	OBJECT FACTORY
*/
//----< factory functions >--------------------------------------------------
ISendr* ObjectFactory::createSendr(size_t portNum) { return new Sendr(portNum); }

//IRecvr* ObjectFactory::createRecvr() { return new Recvr; }

IRecvr* ObjectFactory::createRecvr(size_t portNum/*, Socket::IpVer ip*/) { return new Recvr(portNum, Socket::IP6); }

IMockChannel* ObjectFactory::createMockChannel(ISendr* pISendr, IRecvr* pIRecvr/*, std::string sendrAddr, std::string recvrAddr*/)
{

	return new MockChannel(pISendr, pIRecvr);
}







#ifdef TEST_MOCKCHANNEL

//----< test stub >----------------------------------------------------------

int main()
{
	/*
  ObjectFactory objFact;
  ISendr* pSendr = objFact.createSendr();
  IRecvr* pRecvr = objFact.createRecvr();
  IMockChannel* pMockChannel = objFact.createMockChannel(pSendr, pRecvr);
  pMockChannel->start();
  pSendr->postMessage("Hello World");
  pSendr->postMessage("CSE687 - Object Oriented Design");
  Message msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  pSendr->postMessage("stopping");
  msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  pMockChannel->stop();
  pSendr->postMessage("quit");
  std::cin.get();
	*/
}
#endif
