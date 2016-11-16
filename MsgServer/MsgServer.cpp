/////////////////////////////////////////////////////////////////////////
// MsgServer.h - Server that also acts as a Code Repository.           //
//               Can send and receive HttpMessages.                    //
//               Also handles buffer inputs for file storing.          //
//               Can also perform file transfer to client              //
//                                                                     //
// Author :     Young Kyu Kim, ykim127@syr.edu                         //
// Source :     Jim Fawcett, CSE687 - OOD, Spring 2016                 //
// Application: OOD Project #4                                         //
/////////////////////////////////////////////////////////////////////////

#include "MsgServer.h"

#include <string>
#include <iostream>

using Show = StaticLogger<1>;
using namespace Utilities;
//using namespace FileSystem;                                              //allows me to create directories


//Msg Server's execution
void MsgServer::execute(const size_t TimeBetweenMessages, const size_t NumMessages)
{
	Show::attach(&std::cout);
	Show::start();

	//BlockingQueue<HttpMessage> msgQ = pRecvr->queue();

	try
	{
		//1. Setup Socket Connector???
		Show::title("\nCreating Socket Connector for Client", '=');
		pSendr->startConnecting();                                          //connect to client's port 9080
		Show::write("\n *** Repository's SocketConnector connected client's listener");

		//2. Setup Socket Listener
		Show::title("\nCreating Socket Listener for Socket Client", '=');
		pRecvr->startListening(pSendr, pRecvr, "Client");

		//3. Setup receive Thread
		/*
		* Since this is a server the loop below never terminates.
		* We could easily change that by sending a distinguished
		* message for shutdown.
		*/
		while (true)                                                         //This acts as my receiving thread
		{
			
			if (pRecvr->getQSize() > 0)
			{
				HttpMessage msg = pRecvr->getMessage();
				Show::write("\n\n  server recvd message contents:\n" + msg.bodyString());
			}
			
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}

//Set address of MsgServer
void MsgServer::setServerInfo(size_t my, size_t your, Socket::IpVer ip)
{

	ObjectFactory* pObjFact = new ObjectFactory;           //use object factory to populate sendr/recvr

	pSendr = pObjFact->createSendr(your);                 //
	//pRecvr = pObjFact->createRecvr();
	pRecvr = pObjFact->createRecvr(my/*, ip*/);
	//clientEndPoint = "localhost:" + portNum;
	//clientPortNum = portNum;
}


//----< test stub >--------------------------------------------------

#define SERVER_TEST
#ifdef SERVER_TEST
int main(int argc, char* argv[])
{
  ::SetConsoleTitle(L"HttpMessage Server - Runs Forever");

  Show::attach(&std::cout);
  Show::start();
  Show::title("\n  HttpMessage Server started");

	//Calc ServerPort
	size_t result1;                                        //clients port
	std::stringstream ss1(argv[1]);
	ss1 >> result1;                                        //convert string portnumber to size_t port number
	ss1.str("");
	ss1.clear();//Client's sender

	//Calc Clientport
	size_t result2;                                        //clients port
	std::stringstream ss2(argv[2]);
	ss2 >> result2;                                        //convert string portnumber to size_t port number
	ss2.str("");
	ss2.clear();//Client's sender

	SocketSystem ss;
	MsgServer s1;
	s1.setServerInfo(result1, result2, Socket::IP6);               //server's address, initialize ISendr + IRecvr
	std::thread t1(
		[&]() { s1.execute(100, 1); } // 20 messages 100 millisec apart
	);

	t1.join();

}
#endif