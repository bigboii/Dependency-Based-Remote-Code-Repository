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

//#include "MsgClient.h"
#include "../FileSystem/FileSystem.h"

#include <string>
#include <iostream>

//using Show = StaticLogger<1>;
//using namespace Utilities;

//#include "../MockChannel/RepositoryHandler.h"




//----< miscellaneous stuff - setup file reading>------





/*
		TEST STUB, MAIN()
*/
//----< entry point - runs two clients each on its own thread >------
#define CLIENT_TEST
#ifdef CLIENT_TEST

int main()
{
  ::SetConsoleTitle(L"Clients Running on Threads");

	/*
  Show::title("Demonstrating two HttpMessage Clients each running on a child thread");
	SocketSystem ss;
  MsgClient c1;
	c1.setConnectionInfo(9080, Socket::IP6);               // Client's address: 9080, initialize ISendr + IRecvr         
  std::thread t1(
    [&]() { c1.execute(100, 1); } // 20 messages 100 millisec apart
  );
	*/

  /*
  MsgClient c2;
  std::thread t2(
    [&]() { c2.execute(120, 20); } // 20 messages 120 millisec apart
  );
  */
  //t1.join();
  //t2.join();
}
#endif