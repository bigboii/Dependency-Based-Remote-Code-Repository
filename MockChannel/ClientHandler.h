#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

/////////////////////////////////////////////////////////////////////
//  ClientHandler.h - Reads words from a file                      //
//  ver 2.0                                                        //
//                                                                 //
//  Language:      Visual C++ 2008, SP1                            //
//  Application:   CSE687 Project 4, Sp2016                        //
//  Author:        Young Kyu Kim, ykim127@syr.edu                  //
//  Source:        Jim Fawcett, CST 4-187, Syracuse University     //
//                 (315) 443-3948, jfawcett@twcny.rr.com           //
/////////////////////////////////////////////////////////////////////

#include "MockChannel.h"                                
#include "../XmlDocument/XmlDocument/XmlDocument.h"                    //Has
#include <time.h> 
#include <string>


//using Show = StaticLogger<1>;
//using namespace Utilities;
//using namespace FileSystem;                                              //allows me to create directories

/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You need to be careful using data members of this class
//   because each client handler thread gets a reference to this 
//   instance so you may get unwanted sharing.
// - I may change the SocketListener semantics (this summer) to pass
//   instances of this class by value.
// - that would mean that all ClientHandlers would need either copy or
//   move semantics.
//

using namespace XmlProcessing;                   //needed to use buildMetaData()
class ClientHandler
{
public:
	ClientHandler(ISendr* sendr, IRecvr* recvr) : pSendr(sendr), pRecvr(recvr) {};
	void operator()(Socket socket);
	bool sendFile(const std::string& filename, std::string pkgname);
	HttpMessage makeMessage(size_t n, const std::string& cmd, const std::string& body, const std::string& ep);
	XmlDocument buildMetaData(HttpMessage msg);
	void saveMetaData(XmlDocument doc, std::string pkgName);

	std::string generatePkgPath(std::string pkgName);
	std::string getPkgDirectory(std::string pkgName);

	//metadata functions()
	bool checkIfPackageExist(std::string pkgPath, std::string pkgName);
	bool checkIfClosed(std::string pkgname);   
	void addDependency(std::string filename, std::string pkgpath, std::string pkgname);
	void closePackage(std::string pkgpath);

private:
	bool connectionClosed_;
	HttpMessage readMessage(Socket& socket);                                      //Read message and enQ in to blocking queue     
	bool readFile(const std::string& filename, const std::string& pkgname, size_t fileSize, Socket& socket);  //read a binary file from socket and save
	bool handleNewMessage(Socket& socket, HttpMessage& msg);
	bool handlePostFile(Socket& socket, HttpMessage& msg);
	bool handleCheckIn(Socket& socket, HttpMessage& msg);
	bool handleGetFileList(Socket& socket, HttpMessage& msg);
	bool handleExtract(Socket& socket, HttpMessage& msg);
																																								//BlockingQueue<HttpMessage>& msgQ_;

																																								//Q's Variables
	bool isCheckIn = false;                       //determine checkin status
	//std::string pkgName = "";                     //temporary package name when checking in
	//std::string pkgPath = "";                     //temporary package path for building metadata
	//std::vector<std::string> checkInFileList;     //names of files to be checked in

																								//Mock Channel Variables
	void createChannel();                            //create channel between peer to another peer
	ISendr* pSendr;                                //reference to sender
	IRecvr* pRecvr;
	//receiveThread?

};

//Create Channel between 2 peers
void ClientHandler::createChannel()
{
	//pSendr = objFact.createSendr();
	//pRecvr = objFact.createRecvr();
	//pMockChannel = objFact.createMockChannel(pSendr, pRecvr, "localhost:8080", "localhost:9080");
	//pMockChannel->start();

}

//----< this defines processing to frame messages >------------------
HttpMessage ClientHandler::readMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;
	while (true)                                                                          	//1. read message attributes
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

	//2. If client is done, connection breaks and recvString returns empty string
	if (msg.attributes().size() == 0)
	{
		connectionClosed_ = true;
		return msg;
	}
	if (handleNewMessage(socket, msg))
		return msg;

	//4. Check for other commands and process them
	if (msg.attributes()[0].first == "CHECK_IN")                    //Client Starting Checkin
	{
		handleCheckIn(socket, msg);
	}
	if (msg.attributes()[0].first == "CHECK_IN_DONE")               //Client Finished Checking In
	{
		isCheckIn = false;
		std::cout << "\n\tCHECK_IN_DONE received";
		closePackage(msg.findValue("Name"));
	}
	if (msg.attributes()[0].first == "EXTRACT")                     //Client Requesting Files
	{
		handleExtract(socket, msg);
	}
	if (msg.attributes()[0].first == "GET_FILELIST")                //Client Requesting Files
	{
		std::cout << "\n\tGET_FILELIST received";
		handleGetFileList(socket, msg);
	}
	return msg;
}

bool ClientHandler::handleNewMessage(Socket& socket, HttpMessage& msg)
{
	if (msg.attributes()[0].first == "POST_FILE")                         	//3A) Check if 
	{
		std::cout << "\n\tPOST_FILE received";
		if (handlePostFile(socket, msg))
			return true;
	}
	else
	{
		//3B) read message body of non post related stuff
		size_t numBytes = 0;
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

	return false;
}

//Handles Client's POST_FILE command
bool ClientHandler::handlePostFile(Socket& socket, HttpMessage& msg)
{

	std::string pkgName = msg.findValue("Name") + "_PACKAGE";

	//(NON GUI VERSION) check if xml exists and if check in is still open
	//std::string currDir = Directory::getCurrentDirectory();                //generate path
	//std::size_t found = currDir.find_last_of("\\");
	//std::string path = currDir.substr(0, found);
	//std::string pkgPath = path + "\\TestRepository\\";
	//(GUI VERSION)
	std::string path = Directory::getCurrentDirectory();                //generate path
	std::string pkgPath = path + "\\TestRepository\\";

	if (checkIfPackageExist(pkgPath, pkgName))         //if corresponding package does exist
	{
		if (checkIfClosed(msg.findValue("Name")))        //check closed property of xml data
		{
			//B) is this a file message?
			std::string filename = msg.findValue("file");
			if (filename != "")
			{
				size_t contentSize;
				std::string sizeString = msg.findValue("content-length");
				if (sizeString != "")
					contentSize = Converter<size_t>::toValue(sizeString);
				else
					return true;

				//std::cout << "\n readFile() pkgname:" << msg.findValue("Name") << std::endl;         //DEBUGGING
				readFile(filename, msg.findValue("Name"), contentSize, socket);                    //read a binary file from socket and save
				std::string path = pkgPath + pkgName;                                        //Add filename to deps list
				addDependency(filename, path, msg.findValue("Name"));
			}
			if (filename != "")                                                           	//C)get filename
			{
				// construct message body
				msg.removeAttribute("content-length");
				std::string bodyString = "<file>" + filename + "</file>";
				std::string sizeString = Converter<size_t>::toString(bodyString.size());
				msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
				msg.addBody(bodyString);
			}
		}
	}

	return false;
}

//Handles Client's CheckIn request
bool ClientHandler::handleCheckIn(Socket& socket, HttpMessage& msg)
{
	std::cout << "\n\tCHECK_IN received";
	std::string pkgpath = generatePkgPath(msg.findValue("Name"));
	std::cout << "\nCHECK_IN path: " << pkgpath << std::endl;      //DEBUGGING
																																 //pkgpath.append(pkgName);
	Directory::create(pkgpath);                              //Create new package directory
																													 //XmlDocument doc = buildMetaData(msg);                      //Generate XML metadata based on msg
	saveMetaData(buildMetaData(msg), msg.findValue("Name"));
	isCheckIn = true;
	return true;
}

//Handles Client's GetFileList request
bool ClientHandler::handleGetFileList(Socket& socket, HttpMessage& msg)
{
	//Split Package name and path

	std::string path = getPkgDirectory(msg.findValue("Name"));

	//Display Files in dir and store into body for msg
	std::vector<std::string> currfiles;
	std::cout << "\n files residing in " << path << " are:";       //Get files in non-current directory
	currfiles = Directory::getFiles(path, "*.*");        //*.* : get all files, *.snt       //*.snt : only snt files
	std::string body = "";                               //Body for message
	for (size_t i = 0; i < currfiles.size(); ++i)
	{
		std::cout << "\n    " << currfiles[i].c_str();
		body.append(currfiles[i] + "\n");
	}
	std::cout << "\n";

	//Construct HttpMessage for response
	HttpMessage msgResponse;
	HttpMessage::Attribute attrib;
	std::string myEndPoint = "localhost:8080";  // ToDo: make this a member of the sender
																							// given to its constructor
	std::string ep = "localhost::9080";
	msgResponse.clear();
	msgResponse.addAttribute(HttpMessage::attribute("GET_FILELIST_RESULT", "GET_FILELIST_RESULT"));                              //Command (1st field) , (2nd field)???
	msgResponse.addAttribute(HttpMessage::Attribute("mode", "oneway"));                      //Mode
	msgResponse.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));                   //toAddr
	msgResponse.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));         //ToDo: update this to take any 

	msgResponse.addBody(body);
	if (body.size() > 0)
	{
		attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
		msgResponse.addAttribute(attrib);
	}

	//Send message
	std::string msgString = msgResponse.toString();
	//sendSocket->send(msgString.size(), (Socket::byte*)msgString.c_str());
	//sendr->postMessage(msgString);
	pSendr->postMessage(msgResponse);

	return true;
}


//Handles Client's extract request
bool ClientHandler::handleExtract(Socket& socket, HttpMessage& msg)
{
	std::cout << "\n\tEXTRACT received";
	std::string pkgName = msg.findValue("Name");
	//std::string newPkgPath = "../TestFiles/Repository/TEST_PACKAGE";
	//std::cout << "Getting file from package " << newPkgPath;

	//2. SEND FILES in _PACKAGE
	std::string pkgdir = generatePkgPath(msg.findValue("Name"));
	std::cout << "\nEXTRACT pkgdir: " << pkgdir << std::endl;
	std::vector<std::string> files = FileSystem::Directory::getFiles(pkgdir, "*.snt");
	for (size_t i = 0; i < files.size(); ++i)
	{
		Show::write("\n\n Repository sending file " + files[i]);
		//Show::write("\n\n Repository sending file Test.cpp.snt");
		sendFile(files[i], pkgName);
	}

	return true;
}


//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readFile(const std::string& filename, const std::string& pkgname, size_t fileSize, Socket& socket)
{
	//1. Construct appropriate path for writing
	//std::string newPkgPath = pkgPath + "/" + filename + ".snt";             //NON GUI WPF version
	std::string newPkgPath = generatePkgPath(pkgname) + filename + ".snt";       //GUI WPF version

	//std::cout << "\n readFile() newPkgPath: " << newPkgPath << std::endl;                   //DEBUGGING
	//std::string fqname = "../TestFiles/" + filename + ".snt";
	std::string fqname = newPkgPath;
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
//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
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
			Show::write("\n\n  clienthandler thread is terminating");
			break;
		}
		//msgQ_.enQ(msg);         //enq into recever queue
		pRecvr->enQ(msg);
	}
}

//----< build an XmlDocument for testing >-----------------------------------
using sPtr = std::shared_ptr < AbstractXmlElement >;
XmlDocument ClientHandler::buildMetaData(HttpMessage msg)
{
	sPtr pRoot = makeTaggedElement("metadata");
	XmlDocument doc(XmlProcessing::makeDocElement(pRoot));

	//1. Create Package Name Tag
	sPtr tagName = makeTaggedElement("name");                        //Make Package Name tag
	tagName->addChild(makeTextElement(msg.findValue("Name")));

	//2. Create Dependency List Tag
	sPtr tagDeps = makeTaggedElement("deps");

	pRoot->addChild(tagName);                              //add name to metadata
	pRoot->addChild(tagDeps);                              //add dep list to metadata

																												 //3. Create Author Tag
	sPtr tagAuthor = makeTaggedElement("author");
	tagAuthor->addChild(makeTextElement("Young Kyu Kim"));
	pRoot->addChild(tagAuthor);                               //add author to metadata

	char date[9];                                            //add date tag to metadata
	_strdate_s(date);
	sPtr tagDate = makeTaggedElement("date");
	tagDate->addChild(makeTextElement(date));
	pRoot->addChild(tagDate);
																														//4. Create Closed Tag
	sPtr tagClosed = makeTaggedElement("checkInStatus");
	tagClosed->addChild(makeTextElement("open"));
	pRoot->addChild(tagClosed);                           //add closed status to metadata

	return doc;
}

void ClientHandler::saveMetaData(XmlDocument doc, std::string name)
{
	//std::string path = "../TestFiles/Repository/" + name + "_PACKAGE" + "/" + name + "_meta.xml";   //construct correct path to write metadata
	std::string path = generatePkgPath(name) + name + "_meta.xml";
	std::cout << "\nsaveMetaData path: " << path << std::endl;                                                     //Debugging
	std::ofstream out(path);
	out << doc.toString();
	out.close();
}

bool ClientHandler::checkIfPackageExist(std::string pkgpath, std::string pkgName)
{
	std::vector<std::string> currdirs = Directory::getDirectories(pkgpath);
	bool pkgFound = false;
	for (size_t i = 0; i < currdirs.size(); ++i)                               //check if corresponding pkg exists
	{
		if (currdirs[i] == pkgName)
			pkgFound = true;
	}

	if (pkgFound == true)
	{
		pkgpath.append(pkgName);
		std::vector<std::string> currfiles = Directory::getFiles(pkgpath, "*.xml");   //get all xml files
		if (currfiles.size() > 0)                                //check if metadata.xml file exist
		{
			return true;
		}
	}

	return pkgFound;
}

void ClientHandler::addDependency(std::string filename, std::string pkgpath, std::string pkgname)
{
	//std::string metaPath = "../TestFiles/Repository/" + pkgname + "_PACKAGE/" + pkgname + "_meta.xml";

	std::string xmlLocation = pkgpath +"/"+ pkgname + "_meta.xml";
	std::cout << "xmlLocation:" << xmlLocation << std::endl;
	XmlDocument doc(xmlLocation, XmlDocument::file);

	//Make dep tag
	sPtr tagDep = makeTaggedElement("dep");            //
	tagDep->addChild(makeTextElement(filename));       // add dep to list

	//find deps tag
	std::string testTag = "deps";
	std::vector<sPtr> found = doc.element(testTag).select();

	if (found.size() > 0)                                     //if deps tag found, add dep to deps
	{
		//std::cout << "\n  found " << found[0]->tag();
		found[0]->addChild(tagDep);
	}

	saveMetaData(std::move(doc), pkgname);
}

//Close the package so that it can't be modified anymore
void ClientHandler::closePackage(std::string pkgname)
{
	std::string xmlpath = generatePkgPath(pkgname) + pkgname + "_meta.xml";            //make package path
	std::cout << "\nclosePackage path: " << xmlpath << std::endl;      //DEBUGGING
	XmlDocument doc(xmlpath, XmlDocument::file);

  //find closed tag
	std::string searchTag = "checkInStatus";          //checkInStatus
	std::vector<sPtr> found = doc.element(searchTag).select();

	if (found.size() > 0)                                     //if deps tag found, add dep to deps
	{
		//found[0]->removeChild();

		for (std::shared_ptr<AbstractXmlElement> elem : found[0]->children())
		{
			found[0]->removeChild(elem);
			//std::cout << "\n elem tag: " << elem->tag();
			//std::cout << "\n elem value: " << elem->value();
			//elem->value() = "true";
		}
		found[0]->addChild(makeTextElement("closed"));
		//doc.xmlRoot()->addChild(found[0]);
		//std::cout << "\n  found " << found[0]->tag();
		//found[0]->value() = "true";
		saveMetaData(std::move(doc), pkgname);
	}
}

//Check if package is closed for checkin; returns false if open, true if closed
//std::string enQuote(std::string s) { return "\"" + s + "\""; }

bool ClientHandler::checkIfClosed(std::string pkgname)
{
	//generate xml metadata document
	std::string metaPath = generatePkgPath(pkgname) + pkgname + "_meta.xml";
	std::cout << "\n metaPath: " << metaPath << std::endl;
	XmlDocument doc(metaPath, XmlDocument::file);

	//test search for element with specified tag
	std::string searchTag = "checkInStatus";
	std::vector<sPtr> found = doc.element(searchTag).select();
	if (found.size() > 0)
	{
		for (std::shared_ptr<AbstractXmlElement> elem : found[0]->children())
		{
			std::cout << "\n elem tag: " << elem->tag();
			std::cout << "\n elem value: " << elem->value();

			//trim \n and ' '
			std::string temp2 = elem->value();
			int count = 0;
			for (size_t i = 0; i < elem->value().size(); i++)
			{
				if (temp2[i] != '\n' && temp2[i] != ' ')
				{
					//temp.append(&temp2[i]);
					count++;
				}
				else
					break;
			}
			std::string temp = temp2.substr(0, count);
			//std::cout << "temp: " << temp << "\n";
			if (temp.compare("open") == 0)
			{
				return true;
			}
			if (temp.compare("open") != 0)
			{
				return false;
			}
		}
	}
	else                                               //couldn't find tag
	{
		return false;
	}

	return false;
}

//Allows Repository to send files to client
//----< send file using socket >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool ClientHandler::sendFile(const std::string& filename, std::string pkgname)
{
	// assumes that socket is connected
	//1. Setup File path
	std::string fqname = generatePkgPath(pkgname) + filename;

	//2. Get file size
	FileSystem::FileInfo fi(fqname);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);

	//3. Setup FileSystem for Read
	FileSystem::File file(fqname);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;

	//4. Send Confimation Message which allows client to prepare for file receiving
	HttpMessage msg = makeMessage(1, "EXTRACT_RESULTS", "", "localhost::9080");     //NOTE!! must keep body field empty using ""
	msg.addAttribute(HttpMessage::Attribute("file", filename));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
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

//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - To do that the first argument is 1, e.g., index for the type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
HttpMessage ClientHandler::makeMessage(size_t n, const std::string& cmd, const std::string& body, const std::string& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	std::string myEndPoint = "localhost::8080";  // ToDo: make this a member of the sender
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

//generates and returns a path where repository can store code and metadata
std::string ClientHandler::generatePkgPath(std::string pkgName)
{
	//std::string path = "../TestRepository/" + pkgName + "_PACKAGE/";         //NON GUI VERSION
	std::string path = "./TestRepository/" + pkgName + "_PACKAGE/";          //GUI VERSION
	return path;
}


std::string ClientHandler::getPkgDirectory(std::string pkgName)
{
	//Split Package name and path (NON GUI VERSION)
	/*
	std::string currDir = Directory::getCurrentDirectory();
	std::cout << "Splitting: " << currDir << '\n';
	std::size_t found = currDir.find_last_of("\\");
	std::string path = currDir.substr(0, found);
	std::cout << " path: " << path << '\n';                           //get path name of cur dir
	std::cout << " currdir: " << currDir.substr(found + 1) << '\n';   //get folder name of current directory
																																		//Construct directory
	std::string fs = "\\TestRepository\\" + pkgName + "_PACKAGE";
	path.append(fs);
	*/

	//GUI VERSION
	std::string path = Directory::getCurrentDirectory();
	std::cout << " path: " << path << '\n';                           //get path name of cur dir
	std::string fs = "\\TestRepository\\" + pkgName + "_PACKAGE";
	path.append(fs);

	return path;
}

#endif