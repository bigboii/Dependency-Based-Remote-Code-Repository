#ifndef MSGCLIENT_H
#define MSGCLIENT_H

/////////////////////////////////////////////////////////////////////////////
//  MsgClient.h - Reads words from a file                                  //
// - was created as a class so more than one instance could be run on      //
//   child thread                                                          //
//                                                                         //
//  ver 1.1                                                                //
//  Application:   CSE687 Project 4, Sp2016                                //
//  Author:        Young Kyu Kim, ykim127@syr.edu                          //
//  Source:        Jim Fawcett, CST 4-187, Syracuse University             //
//                 (315) 443-3948, jfawcett@twcny.rr.com                   //
/////////////////////////////////////////////////////////////////////////////
/*
* This package implements a client that sends HTTP style messages and
* files to a server that simply displays messages and stores files.
*
* It's purpose is to provide a very simple illustration of how to use
* the Socket Package provided for Project #4.
*/

#include<string>
#include "../MockChannel/MockChannel.h"
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include <string>
#include <iostream>

using Show = StaticLogger<1>;
using namespace Utilities;

class MsgClient
{
public:
	using EndPoint = std::string;
	void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
	//void setConnectionInfo(size_t portNum, Socket::IpVer ip);                    //Used for testing in main() test stub of MsgClient
	void setChannelReference(ISendr* sendr, IRecvr* recvr/*, size_t my, size_t your*/);                      //Used for WPF
	bool sendFile(const std::string& fqname, const std::string& pkgname);
	void startTest();

private:
	HttpMessage makeMessage(size_t n, const std::string& cmd, const std::string& msgBody, const EndPoint& ep);

	void sendMessage(HttpMessage& msg, Socket& socket);
	size_t getFileSize(std::string filename);
	bool checkIn(std::string pkgName);
	bool checkInDone(std::string pkgName);
	bool extract(std::string packageName);
	bool getFileList(std::string packageName);

	//Mock Channel Variables
	void createChannel();                            //create channel between peer to another peer
																									 //ObjectFactory objFact;
																									 //IMockChannel* pMockChannel;
	EndPoint myEndPoint;
	size_t myPortNum;
	size_t serverPortNum;
	ISendr* pSendr;
	IRecvr* pRecvr;
};






/////////////////////////////////////////////////////////////////////
// ClientCounter creates a sequential number for each client
//
class ClientCounter
{
public:
	ClientCounter() { ++clientCount; }
	size_t count() { return clientCount; }
private:
	static size_t clientCount;
};

size_t ClientCounter::clientCount = 0;


//setup Client's port number and ip for recvr
/*
void MsgClient::setConnectionInfo(size_t portNum, Socket::IpVer ip)
{
	ObjectFactory* pObjFact = new ObjectFactory;           //use object factory to populate sendr/recvr

	pSendr = pObjFact->createSendr();                 //
	pRecvr = pObjFact->createRecvr(portNum);

	//myEndPoint = "localhost:" + portNum;           //e.g.  localhost:9080
	//serverPortNum = portNum;
	//Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
	//Recvr* pRecvr = dynamic_cast<Recvr*>(pIRecvr_);


}
*/

//set reference of Sendr && Recvr from GUI
void MsgClient::setChannelReference(ISendr* sendr, IRecvr* recvr/*, size_t my, size_t your*/)
{
	this->pSendr = sendr;
	this->pRecvr = recvr;
	//myPortNum = my;
	//serverPortNum = your;
}

//Create Channel between 2 peers
void MsgClient::createChannel()
{
	//pSendr = new Sendr();         // objFact.createSendr();
	//pRecvr = new Recvr();         // objFact.createRecvr();
	//pMockChannel = objFact.createMockChannel(pSendr, pRecvr, "localhost:9080", "localhost8080");
	//pMockChannel->start();
}



//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - To do that the first argument is 1, e.g., index for the type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
HttpMessage MsgClient::makeMessage(size_t n, const std::string& cmd, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:9080";  // ToDo: make this a member of the sender
																					 // given to its constructor.
	switch (n)
	{
	case 1:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute(cmd, cmd));                              //Command (1st field) , (2nd field)???
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));                      //Mode
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));                   //toAddr
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));         //ToDo: update this to take any address

		msg.addBody(body);
		if (body.size() > 0)
		{
			attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			msg.addAttribute(attrib);
		}
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}
//----< send message using socket >----------------------------------

void MsgClient::sendMessage(HttpMessage& msg, Socket& socket)
{
	std::string msgString = msg.toString();
	socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}

//----< calculates filesize as size_t >----------------------------------
size_t MsgClient::getFileSize(std::string filename)
{
	//2. Get file size
	FileSystem::FileInfo fi(filename);
	size_t fileSize = fi.size();
	return fileSize;
	//std::string sizeString = Converter<size_t>::toString(fileSize);
}

//----< send CheckIn message >---------------------------------------
bool MsgClient::checkIn(std::string pkgName)
{
	//1. Make msg body
	std::string msgBody;

	//2. Make HttpMessage and Send
	HttpMessage checkinMsg = makeMessage(1, "CHECK_IN", "Checking In", "localhost::8080");      //Checkin msg with nobody
	checkinMsg.addAttribute(HttpMessage::Attribute("Name", pkgName));
	//sendMessage(checkinMsg, socket);
	pSendr->postMessage(checkinMsg);
	return true;
}

//----< send CheckInDone message >---------------------------------------
bool MsgClient::checkInDone(std::string pkgName)
{
	//1. Make msg body
	std::string msgBody;

	//2. Make HttpMessage and Send
	HttpMessage checkInDoneMsg = makeMessage(1, "CHECK_IN_DONE", "Checking In Finished", "localhost::8080");      //Checkin msg with nobody
	checkInDoneMsg.addAttribute(HttpMessage::Attribute("Name", pkgName));
																																																								//sendMessage(checkInDoneMsg, socket);
	pSendr->postMessage(checkInDoneMsg);

	return true;
}

//----< send Extract message >---------------------------------------
bool MsgClient::extract(std::string packageName)
{
	//2. Make HttpMessage and Send
	HttpMessage extractMsg = makeMessage(1, "EXTRACT", "extracting package", "localhost::8080");      //Checkin msg with nobody
	extractMsg.addAttribute(HttpMessage::Attribute("Name", packageName));                                      //Name of package to retrieve file list
																																																						 //sendMessage(extractMsg, socket);
	pSendr->postMessage(extractMsg);
	return true;
}

//----< send CheckInDone message >---------------------------------------
bool MsgClient::getFileList(std::string packageName)
{
	//2. Make HttpMessage and Send
	HttpMessage getFileListMsg = makeMessage(1, "GET_FILELIST", "getting file list", "localhost::8080");      //Checkin msg with nobody
	getFileListMsg.addAttribute(HttpMessage::Attribute("Name", packageName));                                      //Name of package to retrieve file list
																																																								 //sendMessage(getFileListMsg, socket);
	pSendr->postMessage(getFileListMsg);

	return true;
}


//----< send file using socket >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool MsgClient::sendFile(const std::string& filename, const std::string& pkgname)
{
	// assumes that socket is connected
	//1. Setup File path
	//std::string fqname = "../TestFiles/" + filename;                 //NON GUI Version
	std::string fqname = pSendr->getFilePath() + "/" + filename;       //GUI VERSION
	std::cout << "\nsendFile() path:" << fqname << std::endl;

	//2. Get file size
	FileSystem::FileInfo fi(fqname);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);
	std::cout << "\nsendFile() sizeString:" << sizeString << std::endl;

	//3. Setup FileSystem for Read
	FileSystem::File file(fqname);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;

	//4. Make HttpMessage and Send
	HttpMessage msg = makeMessage(1, "POST_FILE", "", "localhost::8080");     //NOTE!! must keep body field empty using ""
	msg.addAttribute(HttpMessage::Attribute("file", filename));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
	msg.addAttribute(HttpMessage::Attribute("Name", pkgname));
	//sendMessage(msg, socket);
	pSendr->postMessage(msg);

	//5. Setup Buffer + Block
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];

	//6. Read file to be copied and store it into fileblock
	while (true)
	{
		FileSystem::Block blk = file.getBlock(BlockSize);
		if (blk.size() == 0)                                  //if empty block -> exit
			break;
		for (size_t i = 0; i < blk.size(); ++i)               //else -> store into buffer for sending
			buffer[i] = blk[i];
		//socket.send(blk.size(), buffer);                      //send buffer
		pSendr->sendFile(blk.size(), buffer);  //send buffer using sendr

		if (!file.isGood())
			break;
	}
	file.close();
	return true;
}

//----< this defines the behavior of the client >--------------------
void MsgClient::execute(const size_t TimeBetweenMessages, const size_t NumMessages)
{
	ClientCounter counter;              	// send NumMessages messages
	size_t myCount = counter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	Show::attach(&std::cout);
	Show::start();
	Show::title(
		"Starting HttpMessage client" + myCountString +
		" on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id())
	);

	try
	{
		Show::title("\nCreating Socket Listener for Repository", '=');                //1. Setup Socket Listener (enable peer to peer connection)
		pRecvr->startListening(pSendr, pRecvr, "Repository");
		Show::write("\n *** established listening to repository");
		Show::title("\nCreating Socket Connector for Repository", '=');                //2. Setup Socket Connector
		pSendr->startConnecting();                                                //Connect to repository's port, 8080

																																									// send a set of message
		//  send all *.cpp files from TestFiles folder
		//Step 4 below will probably never be called
		//4. shut down server's client handler
		//msg = makeMessage(1, "QUIT", "quitting client", "toAddr:localhost:8080");
		//sendMessage(msg);
		//pSendr->postMessage(msg);
		//Show::write("\n\n  client" + myCountString + " sent\n" + msg.toIndentedString());
		//Show::write("\n");
		//Show::write("\n  All done folks");
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}

//Scripted demo test for TAs
void MsgClient::startTest()
{
	//1. SEND CHECKIN prompt
	checkIn("DUMMY");

	//2. SEND FILES
	std::vector<std::string> files = FileSystem::Directory::getFiles("../TestFilesClient", "*.*");
	for (size_t i = 0; i < files.size(); ++i)
	{
		Show::write("\n\n  sending file " + files[i]);
		sendFile(files[i], "DUMMY");
	}

	//3. SEND CHECK IN DONE prompt
	checkInDone("DUMMY");

	//4.GET FILE LIST
	getFileList("DUMMY");

	//5. Extract
	extract("TEST");

}




#endif