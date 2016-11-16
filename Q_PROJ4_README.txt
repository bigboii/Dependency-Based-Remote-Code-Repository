Young Kyu Kim's Project 4 README

/////////////////////////////////
///    Quick Overview        ///
///////////////////////////////

** I have a working code repository with WPF GUI that supports package checkin, file list retrieval, and  package extraction.
Modules were not implemented.

** Client and Server uses Http Message style system to communicate

** Each package contains a xml Metadata and it supports open/closed properties. 
Once the project is closed, files cannot be posted as the package becomes immutable.

** Multiple Clients are not supported.

** Port Numbers are not hard coded; you can define them flexibly in "run.bat", explained later.

** None of the extra credits were implemented, due to deadline.

//////////////////////////////////
///   HOW TO USE RUN.bat      ///
////////////////////////////////
You can set different port numbers for client and server.
By default, Client is 9085 and
            Server is 8080.


start Debug/CppCli-WPF-App.exe [CLIENT_NUM] [SERVER_NUM]
start Debug/MsgServer.exe [SERVER_NUM] [CLIENT_NUM]


////////////////////////////////////////////////////////
///    Repository and Client Test Files location    ///
//////////////////////////////////////////////////////
** Packages are stored in Repository folder

Repository is located in: 
	./CommPrototype/TestRepository

** Test Files for sending are in TestFilesClient folder

Client Test Files is located in:
	./CommPrototype/TestFilesClient


////////////////////////////////////////////////////////
///                  5 Commands                     ///
//////////////////////////////////////////////////////

You will notice 5 TextBlocks in the WPF:
	- Command  TextBlock
	- PkgName  TextBlock
	- Body     TextBlock       (optional)
	- FileDir  TextBlock
	- FileName TextBlock


There are 5 possible commands (case sensitive):
	CHECK_IN
	POST_FILE
	CHECK_IN_DONE
	GET_FILELIST
	EXTRACT            (require pkgname and filename)



/////////////////////////////////
///  Case "Package Check In"  ///
/////////////////////////////////

To perform a Package Check In, fill out the following TextBlocks as show below
 in following order then press send:

1.
	CommandTextBlock  : "CHECK_IN"
	PkgnameTextBlock  : "DUMMY"      (will create DUMMY_PACKAGE)

2.
	CommandTextBlock  : "POST_FILE"
	PkgnameTextBlock  : "DUMMY"      (will create DUMMY_PACKAGE)
	FiledirTextBlock  : "./TestFilesClient"
	FilenameTextBlock : "Test.h"
3.
	CommandTextBlock  : "POST_FILE"
	PkgnameTextBlock  : "DUMMY"      (will create DUMMY_PACKAGE)
	FiledirTextBlock  : "./TestFilesClient"
	FilenameTextBlock : "Test.cpp"
4.
	CommandTextBlock  : "CHECK_IN_DONE"
	PkgnameTextBlock  : "DUMMY"      (will create DUMMY_PACKAGE)



 After the above steps have been performed, please check "./CommPrototype/TestRepository" to verify
newly created DUMMY_PACKAGE with files. They have ".snt" placed on them

/////////////////////////////
///  Case "EXTRACT"       ///
/////////////////////////////
To extract file simply do the following:

CommandTextBlock: EXTRACT

1.
	CommandTextBlock  : "EXTRACT"
	PkgnameTextBlock  : "DUMMY"      (will extract files from DUMMY_PACKAGE)

 After extraction, please check "./CommPrototype/TestFilesClient" to verify extracted files.
They have ".extracted" idenfifer placed on them

/////////////////////////////////
///  Case "Get File List"    ///
///////////////////////////////

To get list of files in a package simply do the following:

CommandTextBlock: GET_FILE_LIST

1.
	CommandTextBlock  : "GET_FILELIST"
	PkgnameTextBlock  : "DUMMY"      (will get a list of files from DUMMY_PACKAGE)

The filelist will be displayed on WPF GUI.






