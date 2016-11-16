#ifndef MOCKCHANNEL_H
#define MOCKCHANNEL_H

/////////////////////////////////////////////////////////////////////////////
//  MockChannel.h - Reads words from a file                                //
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

#ifdef IN_DLL
#define DLL_DECL __declspec(dllexport)
#else
#define DLL_DECL __declspec(dllimport)
#endif

#include <string>
#include "../HttpMessage/HttpMessage.h"

/////////////////////////////////////////////////////////////////////////////
// Sendr class
// - accepts messages from client for consumption by MockChannel
//

struct ISendr
{
  virtual void postMessage(const HttpMessage& msg) = 0;

	//Q's implementation
	virtual void sendFile(size_t bytes, char* buffer) = 0;
	virtual void startConnecting() = 0;   
	//virtual void updateFile

	virtual std::string getFilePath() = 0;             //for wpf
};



/////////////////////////////////////////////////////////////////////////////
// Recvr class
// - accepts messages from MockChanel for consumption by client
//
struct IRecvr
{
  virtual HttpMessage getMessage() = 0;
	virtual void enQ(HttpMessage msg) = 0;

	//virtual void startListening(std::function<void(ISendr* isendr, IRecvr* irecvr)> func) = 0;         //Q's implementation
	virtual void startListening(ISendr* sendr, IRecvr* recvr, std::string handlerName) = 0;         //Q's implementation
	virtual int getQSize() = 0;
};



/////////////////////////////////////////////////////////////////////////////
// MockChannel class
// - reads messages from Sendr and writes messages to Recvr
//
struct IMockChannel
{
public:
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void updateFilePath(std::string path) = 0;
	virtual void sendFile(std::string filename, std::string pkgname) = 0;
};


#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H
/////////////////////////////////////////////////////////////////////////////
// ObjectFactory struct
// - reads messages from Sendr and writes messages to Recvr
//
extern "C" {
	struct ObjectFactory
	{
		DLL_DECL ISendr* createSendr(size_t portNum);
		//DLL_DECL IRecvr* createRecvr();
		DLL_DECL IRecvr* createRecvr(size_t portNum/*, Socket::IpVer ip*/);
		DLL_DECL IMockChannel* createMockChannel(ISendr* pISendr, IRecvr* pIRecvr/*, size_t sendrAddr, size_t recvrAddr*/);
	};
}
#endif


#endif


