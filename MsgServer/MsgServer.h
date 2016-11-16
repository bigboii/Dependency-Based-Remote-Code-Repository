#ifndef MSGSERVER_H
#define MSGSERVER_H

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
/*
* This package implements a server that receives HTTP style messages and
* files from multiple concurrent clients and simply displays the messages
* and stores files.
*
* It's purpose is to provide a very simple illustration of how to use
* the Socket Package provided for Project #4.
*/

#include "../MockChannel/MockChannel.h"
#include "../Sockets/Sockets.h"

class MsgServer
{
public:
	void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
	void setServerInfo(size_t portNum, size_t portNum2, Socket::IpVer ip);

private:
	ISendr* pSendr;
	IRecvr* pRecvr;
	std::string clientEndPoint;
	size_t clientPortNum;
};


#endif