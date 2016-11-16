#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#define DLL_DECL __declspec(dllexport)
#include "MockChannel.h"

/////////////////////////////////////////////////////////////////////////////
// ObjectFactory struct
// - reads messages from Sendr and writes messages to Recvr
//
extern "C" {
	struct ObjectFactory
	{
		DLL_DECL ISendr* createSendr();
		DLL_DECL IRecvr* createRecvr();
		DLL_DECL IRecvr* createRecvr(size_t portNum, Socket::IpVer ip);
		DLL_DECL IMockChannel* createMockChannel(ISendr* pISendr, IRecvr* pIRecvr, std::string sendrAddr, std::string recvrAddr);
	};
}


#endif