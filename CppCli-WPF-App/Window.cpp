/////////////////////////////////////////////////////////////////////////////
//  Window.h - C++\CLI implementation of WPF Application. It allows        //
//             MsgClient to receive inputs from users to perform           //
//             repository operations                                       //
//  ver 3.1                                                                //
//  Application:   CSE687 Project 4, Sp2016                                //
//  Author:        Young Kyu Kim, ykim127@syr.edu                          //
//  Source:        Jim Fawcett, CST 4-187, Syracuse University             //
//                 (315) 443-3948, jfawcett@twcny.rr.com                   //
/////////////////////////////////////////////////////////////////////////////
/*
*  To run as a Windows Application:
*  - Set Project Properties > Linker > System > Subsystem to Windows
*  - Comment out int main(...) at bottom
*  - Uncomment int _stdcall WinMain() at bottom
*  To run as a Console Application:
*  - Set Project Properties > Linker > System > Subsytem to Console
*  - Uncomment int main(...) at bottom
*  - Comment out int _stdcall WinMain() at bottom
*/
#include "Window.h"
#include "../HttpMessage/HttpMessage.h"
#include "../Utilities/Utilities.h"

using namespace Utilities;

using namespace CppCliWindows;

WPFCppCliDemo::WPFCppCliDemo(String^ clientPort, String^ serverPort)
{
  ObjectFactory* pObjFact = new ObjectFactory;
	size_t result1;                                        //clients port
	std::stringstream ss1(toStdString(clientPort));
	ss1 >> result1;                                        //convert string portnumber to size_t port number
	ss1.str("");
	ss1.clear();//Client's sender
	size_t result2;                                        //server's port
	std::stringstream ss2(toStdString(serverPort));
	ss2 >> result2;                                        //convert string portnumber to size_t port number
	ss2.str("");
	ss2.clear();//Client's sender
	pSendr_ = pObjFact->createSendr(result2);
  pRecvr_ = pObjFact->createRecvr(result1);                                    //Client's reciever (Clients addr: 9080)
  pChann_ = pObjFact->createMockChannel(pSendr_, pRecvr_);
  pChann_->start();
  delete pObjFact;

  // client's receive thread
  recvThread = gcnew Thread(gcnew ThreadStart(this, &WPFCppCliDemo::getMessage));
  recvThread->Start();

  // set event handlers
  this->Loaded += 
    gcnew System::Windows::RoutedEventHandler(this, &WPFCppCliDemo::OnLoaded);
  this->Closing += 
    gcnew CancelEventHandler(this, &WPFCppCliDemo::Unloading);
  hSendButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::sendMessage);                 //Send Message
  hClearButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::clear);
  hFolderBrowseButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::browseForFolder);
  hShowItemsButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::getItemsFromList);
	hConnectButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::connectToServer);          //Q's Event Handling to Q's Connect button
  // set Window properties
  this->Title = "WPF C++/CLI Demo";
  this->Width = 1000;
  this->Height = 700;
  // attach dock panel to Window
  this->Content = hDockPanel;
  hDockPanel->Children->Add(hStatusBar);
  hDockPanel->SetDock(hStatusBar, Dock::Bottom);
  hDockPanel->Children->Add(hGrid);
  // setup Window controls and views
  setUpTabControl();
  setUpStatusBar();
  setUpSendMessageView();
  setUpFileListView();
  setUpConnectionView();
}

WPFCppCliDemo::~WPFCppCliDemo()
{
  delete pChann_;
  delete pSendr_;
  delete pRecvr_;
}

void WPFCppCliDemo::setUpStatusBar()
{
  hStatusBar->Items->Add(hStatusBarItem);
  hStatus->Text = "very important messages will appear here";
  //status->FontWeight = FontWeights::Bold;
  hStatusBarItem->Content = hStatus;
  hStatusBar->Padding = Thickness(10, 2, 10, 2);
}

void WPFCppCliDemo::setUpTabControl()
{
  hGrid->Children->Add(hTabControl);
  hSendMessageTab->Header = "Send Message";
  hFileListTab->Header = "File List";
  hConnectTab->Header = "Connect";
  hTabControl->Items->Add(hSendMessageTab);
  hTabControl->Items->Add(hFileListTab);
  hTabControl->Items->Add(hConnectTab);
}

//TextBlock setup for sendPage
void WPFCppCliDemo::setTextBlockProperties()
{
  RowDefinition^ hRow1Def = gcnew RowDefinition();
  hSendMessageGrid->RowDefinitions->Add(hRow1Def);
  Border^ hBorder1 = gcnew Border();
  hBorder1->BorderThickness = Thickness(1);
  hBorder1->BorderBrush = Brushes::Black;
  hBorder1->Child = hTextBlock1;
  hTextBlock1->Padding = Thickness(15);
  hTextBlock1->Text = "";
  hTextBlock1->FontFamily = gcnew Windows::Media::FontFamily("Tahoma");
  hTextBlock1->FontWeight = FontWeights::Bold;
  hTextBlock1->FontSize = 16;
  hScrollViewer1->VerticalScrollBarVisibility = ScrollBarVisibility::Auto;
  hScrollViewer1->Content = hBorder1;
  hSendMessageGrid->SetRow(hScrollViewer1, 0);
  hSendMessageGrid->Children->Add(hScrollViewer1);

	RowDefinition^ hRow2Def = gcnew RowDefinition();            	//Add Cmd TextBox
	hRow2Def->Height = GridLength(40);
	hSendMessageGrid->RowDefinitions->Add(hRow2Def);
	hTextBoxCmd->FontSize = 16;
	hTextBoxCmd->Text = "COMMAND";
	hTextBoxCmd->Width = 200;
	hTextBoxCmd->Height = 30;
	hSendMessageGrid->SetRow(hTextBoxCmd, 1);
	hSendMessageGrid->Children->Add(hTextBoxCmd);
	
	RowDefinition^ hRow3Def = gcnew RowDefinition();              	//Add Name TextBox
	hRow3Def->Height = GridLength(40);
	hSendMessageGrid->RowDefinitions->Add(hRow3Def);
	hTextBoxName->Text = "PACKAGE NAME";
	hTextBoxName->FontSize = 16;
	hTextBoxName->Width = 200;
	hTextBoxName->Height = 30;
	hSendMessageGrid->SetRow(hTextBoxName, 2);
	hSendMessageGrid->Children->Add(hTextBoxName);

	RowDefinition^ hRow4Def = gcnew RowDefinition();                 	//Add body TextBox
	hRow4Def->Height = GridLength(40);
	hSendMessageGrid->RowDefinitions->Add(hRow4Def);
	hTextBoxBody->Text = "BODY (optional)";
	hTextBoxBody->FontSize = 16;
	hTextBoxBody->Width = 200;
	hTextBoxBody->Height = 30;
	hSendMessageGrid->SetRow(hTextBoxBody, 3);
	hSendMessageGrid->Children->Add(hTextBoxBody);

	setFileTextBlockProperties();
}

void WPFCppCliDemo::setFileTextBlockProperties()
{
	//AddPath Directory TextBox
	ColumnDefinition^ hColumn0Def = gcnew ColumnDefinition();                  //Add 2 columns to grid view
	hColumn0Def->Width = GridLength(400);
	hSendMessageGrid->ColumnDefinitions->Add(hColumn0Def);

	ColumnDefinition^ hColumn1Def = gcnew ColumnDefinition();
	hColumn1Def->Width = GridLength(400);
	hSendMessageGrid->ColumnDefinitions->Add(hColumn1Def);

	//Add File Path TextBox
	hTextBlockPath->Text = "File Path:";                                   //Add Path Title
	hTextBlockPath->FontFamily = gcnew Windows::Media::FontFamily("Tahoma");
	hTextBlockPath->FontWeight = FontWeights::Bold;
	hTextBlockPath->FontSize = 16;
	hSendMessageGrid->SetColumn(hTextBlockPath, 1);
	hSendMessageGrid->SetRow(hTextBlockPath, 0);
	hSendMessageGrid->Children->Add(hTextBlockPath);

	hTextBoxPath->Text = "./TestFilesClient";
	hTextBoxPath->FontSize = 16;
	hTextBoxPath->Width = 200;
	hTextBoxPath->Height = 30;
	hSendMessageGrid->SetColumn(hTextBoxPath, 1);
	hSendMessageGrid->SetRow(hTextBoxPath, 1);
	hSendMessageGrid->Children->Add(hTextBoxPath);

	//Add Filename TextBox
	hTextBlockFile->Text = "File Name:";                                   //Add File Title
	hTextBlockFile->FontFamily = gcnew Windows::Media::FontFamily("Tahoma");
	hTextBlockFile->FontWeight = FontWeights::Bold;
	hTextBlockFile->FontSize = 16;
	hSendMessageGrid->SetColumn(hTextBlockFile, 1);
	hSendMessageGrid->SetRow(hTextBlockFile, 2);
	hSendMessageGrid->Children->Add(hTextBlockFile);

	hTextBoxFileName->Text = "FILE NAME";
	hTextBoxFileName->FontSize = 16;
	hTextBoxFileName->Width = 200;
	hTextBoxFileName->Height = 30;
	hSendMessageGrid->SetColumn(hTextBoxFileName, 1);
	hSendMessageGrid->SetRow(hTextBoxFileName, 3);
	hSendMessageGrid->Children->Add(hTextBoxFileName);
}

void WPFCppCliDemo::setButtonsProperties()
{
  RowDefinition^ hRow5Def = gcnew RowDefinition();
  hRow5Def->Height = GridLength(75);
  hSendMessageGrid->RowDefinitions->Add(hRow5Def);
  hSendButton->Content = "Send Message";
  Border^ hBorder2 = gcnew Border();
  hBorder2->Width = 120;
  hBorder2->Height = 30;
  hBorder2->BorderThickness = Thickness(1);
  hBorder2->BorderBrush = Brushes::Black;
  hClearButton->Content = "Clear";
  hBorder2->Child = hSendButton;
  Border^ hBorder3 = gcnew Border();
  hBorder3->Width = 120;
  hBorder3->Height = 30;
  hBorder3->BorderThickness = Thickness(1);
  hBorder3->BorderBrush = Brushes::Black;
  hBorder3->Child = hClearButton;
  hStackPanel1->Children->Add(hBorder2);
  TextBlock^ hSpacer = gcnew TextBlock();
  hSpacer->Width = 10;
  hStackPanel1->Children->Add(hSpacer);
  hStackPanel1->Children->Add(hBorder3);
  hStackPanel1->Orientation = Orientation::Horizontal;
  hStackPanel1->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;
  hSendMessageGrid->SetRow(hStackPanel1, 4);
  hSendMessageGrid->Children->Add(hStackPanel1);
}

//TAB: send message view
void WPFCppCliDemo::setUpSendMessageView()
{
  Console::Write("\n  setting up sendMessage view");
  hSendMessageGrid->Margin = Thickness(20);
  hSendMessageTab->Content = hSendMessageGrid;

  setTextBlockProperties();
  setButtonsProperties();
}

std::string WPFCppCliDemo::toStdString(String^ pStr)
{
  std::string dst;
  for (int i = 0; i < pStr->Length; ++i)
    dst += (char)pStr[i];
  return dst;
}



//Q's String to HttpMessage generator : NEEDS WORK
/*
HttpMessage toHttpMsg(std::string str)
{
	HttpMessage httpmsg;

	return httpmsg;
}
*/

//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - To do that the first argument is 1, e.g., index for the type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
HttpMessage makeMessage(size_t n, const std::string& cmd, const std::string& body, const std::string& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	std::string myEndPoint = "localhost:9080";  // ToDo: make this a member of the sender
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
			attrib = HttpMessage::attribute("content-length", Utilities::Converter<size_t>::toString(body.size()));
			msg.addAttribute(attrib);
		}
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}

//Q: modified to retrieve contents of textboxes for http message generation
void WPFCppCliDemo::sendMessage(Object^ obj, RoutedEventArgs^ args)
{
	//Q's Code

	//1. Fetch texts from TextBoxes?
	String^ cmd = hTextBoxCmd->Text;            //get command
	String^ name = hTextBoxName->Text;          //get name
	String^ body = hTextBoxBody->Text;          //get body
	//String^ filepath = hTextBoxPath->Text;      //get file path
	//String^ filename = hTextBoxFileName->Text;  //get file name
	hStatus->Text = "cmd: " + cmd + "\tname: " + name + "\tbody: " + body;


	//2. Construct HttpMessage httpmsg(toHttpMsg(toStdString(msgText)));
	//HttpMessage httpmsg = makeMessage(1, "TEST", "Hello, Testing", "localhost:8080");
	HttpMessage httpmsg = makeMessage(1, toStdString(cmd), toStdString(body), "localhost:8080");
	if (cmd == "POST_FILE")
	{
		String^ filepath = hTextBoxPath->Text;      //get file path
		String^ filename = hTextBoxFileName->Text;  //get file name
		httpmsg.addAttribute(HttpMessage::Attribute("Name", toStdString(name)));           //Also add package name

		std::cout << "\nname:" << toStdString(name) << std::endl;
		std::cout << "\npath:" << toStdString(filepath) << std::endl;
		pChann_->updateFilePath(toStdString(filepath));         //update Channel's testfile directory 
		//pSendr_->postMessage(httpmsg);
		pChann_->sendFile(toStdString(filename), toStdString(name));               //Send File
	}
	if (cmd == "CHECK_IN" || cmd == "CHECK_IN_DONE" || cmd == "GET_FILELIST" || cmd == "EXTRACT")       //add package name attribute
	{
		httpmsg.addAttribute(HttpMessage::Attribute("Name", toStdString(name)));
		Console::Write("\n pkgName : "+ name);
		pSendr_->postMessage(httpmsg);
	}

  //pSendr_->postMessage(httpmsg);
  Console::Write("\n  sent message");
  hStatus->Text = "Sent message";

	String^ ip = hTextBoxIp->Text;
	Console::Write("\n Sending msg to Server");
	hStatus->Text = "Connected to " + ip;

	addText(cmd + "executed on package" + name);

}

String^ WPFCppCliDemo::toSystemString(std::string& str)
{
  StringBuilder^ pStr = gcnew StringBuilder();
  for (size_t i = 0; i < str.size(); ++i)
    pStr->Append((Char)str[i]);
  return pStr->ToString();
}

void WPFCppCliDemo::addText(String^ msg)
{
  hTextBlock1->Text += msg + "\n\n";
}


//Receive Thread
void WPFCppCliDemo::getMessage()
{
  // recvThread runs this function

  while (true)
  {
    std::cout << "\n  receive thread calling getMessage()";
    HttpMessage httpmsg = pRecvr_->getMessage();                    //Q's Code
		std::string msg = httpmsg.toString();                           //Q's Code

    String^ sMsg = toSystemString(msg);
    array<String^>^ args = gcnew array<String^>(1);
    args[0] = sMsg;

    Action<String^>^ act = gcnew Action<String^>(this, &WPFCppCliDemo::addText);
    Dispatcher->Invoke(act, args);  // must call addText on main UI thread
  }
}

void WPFCppCliDemo::clear(Object^ sender, RoutedEventArgs^ args)
{
  Console::Write("\n  cleared message text");
  hStatus->Text = "Cleared message";
  hTextBlock1->Text = "";
}


void WPFCppCliDemo::getItemsFromList(Object^ sender, RoutedEventArgs^ args)
{
	int index = 0;
	int count = hListBox->SelectedItems->Count;
	hStatus->Text = "Show Selected Items";
	array<System::String^>^ items = gcnew array<String^>(count);
	if (count > 0) {
		for each (String^ item in hListBox->SelectedItems)
		{
			items[index++] = item;
		}
	}

	hListBox->Items->Clear();
	if (count > 0) {
		for each (String^ item in items)
		{
			hListBox->Items->Add(item);
		}
	}
}



void WPFCppCliDemo::setUpFileListView()
{
  Console::Write("\n  setting up FileList view");
  hFileListGrid->Margin = Thickness(20);
  hFileListTab->Content = hFileListGrid;            //will add the grid to the tab
  RowDefinition^ hRow1Def = gcnew RowDefinition();  //will create a row
  //hRow1Def->Height = GridLength(75);
  hFileListGrid->RowDefinitions->Add(hRow1Def);     //will add the row to the grid
  Border^ hBorder1 = gcnew Border();                //Create Black Border ** 
  hBorder1->BorderThickness = Thickness(1);
  hBorder1->BorderBrush = Brushes::Black;
  hListBox->SelectionMode = SelectionMode::Multiple;
  hBorder1->Child = hListBox;
  hFileListGrid->SetRow(hBorder1, 0);               // ** End of Black border
  hFileListGrid->Children->Add(hBorder1);           //will ad the listbox to the grid

  RowDefinition^ hRow2Def = gcnew RowDefinition();
  hRow2Def->Height = GridLength(75);
  RowDefinition^ hRow2Def2 = gcnew RowDefinition();
  hRow2Def2->Height = GridLength(75);
  hFileListGrid->RowDefinitions->Add(hRow2Def);
  hFileListGrid->RowDefinitions->Add(hRow2Def2);
  hFolderBrowseButton->Content = "Select Directory";
  hFolderBrowseButton->Height = 30;
  hFolderBrowseButton->Width = 120;
  hFolderBrowseButton->BorderThickness = Thickness(2);
  hFolderBrowseButton->BorderBrush = Brushes::Black;
  hFileListGrid->SetRow(hFolderBrowseButton, 1);
  hFileListGrid->Children->Add(hFolderBrowseButton);

  // Show selected items button.
  hShowItemsButton->Content = "Show Selected Items";
  hShowItemsButton->Height = 30;
  hShowItemsButton->Width = 120;
  hShowItemsButton->BorderThickness = Thickness(2);
  hShowItemsButton->BorderBrush = Brushes::Black;
  hFileListGrid->SetRow(hShowItemsButton, 2);
  hFileListGrid->Children->Add(hShowItemsButton);

  hFolderBrowserDialog->ShowNewFolderButton = false;
  hFolderBrowserDialog->SelectedPath = System::IO::Directory::GetCurrentDirectory();
}

void WPFCppCliDemo::browseForFolder(Object^ sender, RoutedEventArgs^ args)
{
  std::cout << "\n  Browsing for folder";
  hListBox->Items->Clear();
  System::Windows::Forms::DialogResult result;
  result = hFolderBrowserDialog->ShowDialog();
  if (result == System::Windows::Forms::DialogResult::OK)
  {
    String^ path = hFolderBrowserDialog->SelectedPath;
    std::cout << "\n  opening folder \"" << toStdString(path) << "\"";
    array<String^>^ files = System::IO::Directory::GetFiles(path, L"*.*");
    for (int i = 0; i < files->Length; ++i)
      hListBox->Items->Add(files[i]);
    array<String^>^ dirs = System::IO::Directory::GetDirectories(path);
    for (int i = 0; i < dirs->Length; ++i)
      hListBox->Items->Add(L"<> " + dirs[i]);
  }
}

//TAB: setup Connect Tab
void WPFCppCliDemo::setUpConnectionView()
{
  Console::Write("\n  setting up Connection view");
	//1. Setup grid layout
	hConnectGrid->Margin = Thickness(20);            //thickness of gridlayout
	hConnectTab->Content = hConnectGrid;             //set Connect GridLayout as Connect Tab's content 

	//2. Setup IP TextBox
	RowDefinition^ hRow1Def = gcnew RowDefinition();       //make 1st row
	hRow1Def->Height = GridLength(50);
	hConnectGrid->RowDefinitions->Add(hRow1Def);                  //add 2nd row to grid
	hTextBoxIp->Text = "IP";
	hTextBoxIp->Width = 200;
	hTextBoxIp->Height = 25;
	hConnectGrid->SetRow(hTextBoxIp, 0);
	hConnectGrid->Children->Add(hTextBoxIp);

	//3. Setup Port TextBox 
	RowDefinition^ hRow2Def = gcnew RowDefinition();
	hRow2Def->Height = GridLength(50);
	hConnectGrid->RowDefinitions->Add(hRow2Def);
	hTextBoxPort->Text = "PORT";
	hTextBoxPort->Width = 200;
	hTextBoxPort->Height = 25;
	hConnectGrid->SetRow(hTextBoxPort, 1);
	hConnectGrid->Children->Add(hTextBoxPort);

	//4. Setup Buttons
	RowDefinition^ hRow3Def = gcnew RowDefinition();       //make 1st row
	hRow3Def->Height = GridLength(50);
	hConnectGrid->RowDefinitions->Add(hRow3Def);                  //add 2nd row to grid
	hConnectButton->Content = "Connect";
	hConnectButton->Width = 120;
	hConnectButton->Height = 30;
	hConnectGrid->SetRow(hConnectButton, 3);
	hConnectGrid->Children->Add(hConnectButton);
}

//Q's Set Event Handler for Connect Button
void WPFCppCliDemo::connectToServer(Object^ obj, RoutedEventArgs^ args)
{
	String^ ip = hTextBoxIp->Text;
	Console::Write("\n Connecting To Server");
	hStatus->Text = "Connected to " + ip;
}

void WPFCppCliDemo::OnLoaded(Object^ sender, RoutedEventArgs^ args)
{
  Console::Write("\n  Window loaded");
}
void WPFCppCliDemo::Unloading(Object^ sender, System::ComponentModel::CancelEventArgs^ args)
{
  Console::Write("\n  Window closing");
}

[STAThread]
//int _stdcall WinMain()
int main(array<System::String^>^ args)
{

	//for (int i = 0; i < args->Length; i++)
	//{
		//do something
		Console::WriteLine(L"\n Starting WPFCppCliDemo");

		Application^ app = gcnew Application();
		app->Run(gcnew WPFCppCliDemo(args[0], args[1]));
		Console::WriteLine(L"\n\n");
	//}

}