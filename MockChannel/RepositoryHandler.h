#ifndef REPOSITORYHANDLER_H
#define REPOSITORYHANDLER_H

/////////////////////////////////////////////////////////////////////////
// RepositoryHandler.h - Handler for incoming messages from Repository //
//                                                                     //
// Author :     Young Kyu Kim, ykim127@syr.edu                         //
// Source :     Jim Fawcett, CSE687 - OOD, Spring 2016                 //
// Application: OOD Project #4                                         //
/////////////////////////////////////////////////////////////////////////

#include "MockChannel.h"

#include "../HttpMessage/HttpMessage.h"        //have
#include "../Sockets/Sockets.h"                //have
#include "../FileSystem/FileSystem.h"          //have
#include "../Logger/Logger.h"                  //have   
#include "../Utilities/Utilities.h"            //have

using Show = StaticLogger<1>;
using namespace FileSystem;                    //allows me to create directories
using namespace Utilities;

class RepositoryHandler
{
public:
	RepositoryHandler(ISendr* sendr, IRecvr* recvr) : pSendr(sendr), pRecvr(recvr) {}
	void operator()(Socket socket);

private:
	bool connectionClosed_;

	ISendr* pSendr;
	IRecvr* pRecvr;

	HttpMessage readMessage(Socket& socket);                                      //Read message and enQ in to blocking queue     
	bool readFile(const std::string& filename, size_t fileSize, Socket& socket);  //read a binary file from socket and save
	bool handleExtractResults(Socket& socket, HttpMessage& msg);

																																								//Q's Variables
	bool isCheckIn = false;                       //determine checkin status
	std::string pkgName = "";                     //temporary package name when checking in
	std::string pkgPath = "";                     //temporary package path for building metadata
	std::vector<std::string> checkInFileList;     //names of files to be checked in
};

//----< Repostory Handler - setup functor >------
void RepositoryHandler::operator()(Socket socket)
{
	/*
	* There is a potential race condition due to the use of connectionClosed_.
	* If two clients are sending files at the same time they may make changes
	* to this member in ways that are incompatible with one another.  This
	* race is relatively benign in that it simply causes the readMessage to
	* be called one extra time.
	*
	* The race is easy to fix by changing the socket listener to pass in a
	* copy of the clienthandler to the clienthandling thread it created.
	* I've briefly tested this and it seems to work.  However, I did not want
	* to change the socket classes this late in your project cycle so I didn't
	* attempt to fix this.
	*/
	while (true)
	{
		HttpMessage msg = readMessage(socket);
		if (connectionClosed_ || msg.bodyString() == "quit")
		{
			Show::write("\n\n  repositoryhandler thread is terminating");
			break;
		}
		pRecvr->enQ(msg);
	}
}


//function that will be used to pass into std::function

//----< this defines processing to frame messages >------------------
HttpMessage RepositoryHandler::readMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;

	while (true)                                        	                               //1. read message attributes
	{
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1)
		{
			HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);         // parse long string into attribute
			msg.addAttribute(attrib);
		}
		else
		{
			break;
		}
	}
	if (msg.attributes().size() == 0)                     	//2. If client is done, connection breaks and recvString returns empty string
	{
		connectionClosed_ = true;
		return msg;
	}
	if (msg.attributes()[0].first == "EXTRACT_RESULTS")          	//3A) Check if EXTRACT results
	{
		std::cout << "\n\tEXTRACT_RESULTS received";
		if(handleExtractResults(socket, msg))
			return msg;
	}
	else
	{
		size_t numBytes = 0;                                  		  //3B) read message body of non post related stuff
		size_t pos = msg.findAttribute("content-length");
		if (pos < msg.attributes().size())
		{
			numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
			Socket::byte* buffer = new Socket::byte[numBytes + 1];
			socket.recv(numBytes, buffer);
			buffer[numBytes] = '\0';
			std::string msgBody(buffer);
			msg.addBody(msgBody);
			delete[] buffer;
		}
	}
	if (msg.attributes()[0].first == "GET_FILELIST_RESULT")                    //Client Starting Checkin
	{
		std::cout << "\n\tGET_FILELIST_RESULT received";
	}
	return msg;
}

bool RepositoryHandler::handleExtractResults(Socket& socket, HttpMessage& msg)
{
	//A) is this a file message?
	std::string filename = msg.findValue("file");
	if (filename != "")
	{
		size_t contentSize;
		std::string sizeString = msg.findValue("content-length");
		if (sizeString != "")
			contentSize = Converter<size_t>::toValue(sizeString);
		else
			return true;

		readFile(filename, contentSize, socket);                    //read a binary file from socket and save
	}

	//B)get filename
	if (filename != "")
	{
		// construct message body
		msg.removeAttribute("content-length");
		std::string bodyString = "<file>" + filename + "</file>";
		std::string sizeString = Converter<size_t>::toString(bodyString.size());
		msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		msg.addBody(bodyString);
	}

	return false;
}


//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool RepositoryHandler::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{

	//1. Construct appropriate path for writing
	//std::string clientPath = "../TestFilesClient/" + filename + ".extracted";           //NON GUI VERSION
	std::string clientPath = "./TestFilesClient/" + filename + ".extracted";           //GUI VERSION
	//std::string fqname = "../TestFiles/" + filename + ".snt";
	std::string fqname = clientPath;
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
		/*
		* This error handling is incomplete.  The client will continue
		* to send bytes, but if the file can't be opened, then the server
		* doesn't gracefully collect and dump them as it should.  That's
		* an exercise left for students.
		*/
		Show::write("\n\n  can't open file " + fqname);
		return false;
	}

	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];

	size_t bytesToRead;
	while (true)
	{
		if (fileSize > BlockSize)
			bytesToRead = BlockSize;
		else
			bytesToRead = fileSize;

		socket.recv(bytesToRead, buffer);

		FileSystem::Block blk;
		for (size_t i = 0; i < bytesToRead; ++i)
			blk.push_back(buffer[i]);

		file.putBlock(blk);
		if (fileSize < BlockSize)
			break;
		fileSize -= BlockSize;
	}
	file.close();

	return true;
}

#endif
