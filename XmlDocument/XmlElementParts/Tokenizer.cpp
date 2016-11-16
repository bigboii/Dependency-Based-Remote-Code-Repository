/////////////////////////////////////////////////////////////////////
//  Tokenizer.cpp - Reads words from a file                        //
//  ver 1.9                                                        //
//                                                                 //
//  Language:      Visual C++ 2008, SP1                            //
//  Platform:      Dell Precision T7400, Vista Ultimate SP1        //
//  Application:   Prototype for CSE687 Pr1, Sp09                  //
//  Author:        Jim Fawcett, CST 4-187, Syracuse University     //
//                 (315) 443-3948, jfawcett@twcny.rr.com           //
/////////////////////////////////////////////////////////////////////

#include <cctype>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include "Tokenizer.h"

//----< constructor may be called with no argument >-----------

Toker::Toker(const std::string& src, bool isFile) 
     : prevprevChar(0), prevChar(0), currChar(0), nextChar(0),
       doReturnComments(false), numLines(0), braceCount(0), pIn(0),
       _state(default_state)
{
  if(src.length() > 0)
  {
    if(!attach(src, isFile))
    {
      std::string temp = std::string("can't open ") + src;
      throw std::exception(temp.c_str());
    }
  }
  scTok = "()[]{};.\n";
  if(_mode == xml)
    scTok = "<>!" + scTok;
}
//----< destructor >-------------------------------------------

Toker::~Toker()
{
  if(pIn)
  {
    pIn->clear();
    std::ifstream* pFs = dynamic_cast<std::ifstream*>(pIn);
    if(pFs)
    {
      pFs->close();
    }
    delete pIn;
  }
}
//----< set mode of tokenizing, e.g., code or xml >------------

void Toker::setMode(Toker::mode md)
{ 
  _mode = md; 
  scTok = "()[]{};.\n";
  if(_mode == xml)
    scTok = "<>!" + scTok;
}
//----< set mode of tokenizing, e.g., code or xml >------------

void Toker::setSingleCharTokens(std::string tokChars)
{ 
  _mode = custom; 
  scTok = tokChars;
}
//----< attach tokenizer to a source file or string >----------

bool Toker::attach(const std::string& name, bool srcIsFile)
{
  if(pIn && srcIsFile)
  {
    pIn->clear();
    std::ifstream* pFs = dynamic_cast<std::ifstream*>(pIn);
    if(pFs)
    {
      pFs->close();
    }
    delete pIn;
  }
  if(srcIsFile)
    pIn = new std::ifstream(name.c_str());
  else
    pIn = new std::istringstream(name.c_str());
  return pIn->good();
}
//----< peek function that works with multiple putbacks >------

int Toker::peek()
{
  if(putbacks.size() > 0)
    return putbacks[putbacks.size()-1];
  else
    return pIn->peek();

}
//----< multiple putBack that won't break peek >---------------

void Toker::putback(int ch)
{
  putbacks.push_back(ch);
  nextChar = ch;
  currChar = prevChar;
  prevChar = prevprevChar;
}
//----< get consistent with peek and putback >-----------------

int Toker::get()
{
  if(putbacks.size() > 0)
  {
    char ch = putbacks.front();
    putbacks.pop_back();
    return ch;
  }
  return pIn->get();
}
//
//----< extract character from attached stream >---------------

bool Toker::getChar()
{
  char oldNext = nextChar;
  prevprevChar = prevChar;
  prevChar = currChar;
  currChar = this->get();
  nextChar = this->peek();
  _ASSERT(currChar == oldNext || oldNext == 0);
  if(currChar == '\n')
    ++numLines;
  if(currChar == '{' && _state == default_state)
    ++braceCount;
  if(currChar == '}' && _state == default_state)
    --braceCount;
  return !pIn->eof();
}
//----< is this char a single char token? >--------------------

bool Toker::isSingleCharTok(char ch)
{
  if(scTok.find(ch) < scTok.length())
    return true;
  return false;
}
//----< remove contiguous white space except for newline >-----

void Toker::stripWhiteSpace()
{
  if(nextChar == '\n')
    return;
  while(isspace(nextChar) && nextChar != '\n')
  {
    getChar();
  }
}
//----< is this an indentifier character? >--------------------

bool Toker::isIdentifierChar(char ch)
{
  if(isalpha(ch) || ch == '_' || isdigit(ch))
    return true;
  return false;
}
//----< is this the end of a token? >--------------------------

bool Toker::isTokEnd()
{
  if(isspace(nextChar))
    return true;
  if(isSingleCharTok(nextChar) || isSingleCharTok(currChar))
    return true;
  if(isIdentifierChar(currChar) && !isIdentifierChar(nextChar))
    return true;
  if(!isIdentifierChar(currChar) && isIdentifierChar(nextChar))
    return true;
  if(isFileEnd())
    return true;
  return false;
}
//----< is this the beginning of a comment? >------------------

bool Toker::isBeginComment()
{
  if(prevChar != '\\' && currChar == '/' && nextChar == '*')
  {
    aCppComment = false;
    return true;
  }
  if(prevChar != '\\' && currChar == '/' && nextChar == '/')
  {
    aCppComment = true;
    return true;
  }
  return false;
}
//----< is this the end of a comment? >------------------------

bool Toker::isEndComment()
{
  if(aCppComment && currChar != '\\' && nextChar == '\n')
      return true;
  if(!aCppComment && prevChar != '\\' && currChar == '*' && nextChar == '/')
    return true;
  return false;
}
//----< return comment as a token >----------------------------

std::string Toker::eatComment()
{
  _state = comment_state;
  std::string tok(1,currChar);
  while(!isEndComment() && pIn->good())
  {
    getChar();
    tok.append(1,currChar);
  }

  if(!aCppComment)
  {
    getChar();
    tok.append(1,currChar);
  }
  _state = default_state;
  return tok;
}
//----< is this the beginning of a quote? >--------------------

bool Toker::isBeginQuote()
{
  if(prevChar != '\\' && currChar == '\'')
  {
    aSingleQuote = true;
    return true;
  }
  if(prevChar != '\\' && currChar == '\"')
  {
    aSingleQuote = false;
    return true;
  }
  return false;
}
//----< is this the end of quote? >----------------------------

bool Toker::isEndQuote()
{
  if(prevChar =='\\' || currChar != '\\')
  {
    if(aSingleQuote && nextChar == '\'')
      return true;
    if(!aSingleQuote && nextChar == '\"')
      return true;
  }
  return false;
}
//----< return single or double quote as token >---------------

std::string Toker::eatQuote()
{
  _state = quote_state;
  std::string tok(1,currChar);
  while(!isEndQuote())
  {
    if(!pIn->good())
      throw std::exception("missing end of quote");
    getChar();
    tok.append(1,currChar);
  }
  getChar();
  tok.append(1,currChar);
  _state = default_state;
  return tok;
}
//----< read token from attached file >------------------------

std::string Toker::getTok()
{
  std::string tok = "";
  stripWhiteSpace();
  if(isSingleCharTok(nextChar))
  {
    getChar();
    tok.append(1,currChar);
    return tok;
  }
  do
  {
    if(isFileEnd())
      return tok;

    getChar();
    if(isBeginComment())
    {
      if(tok.length() > 0)
      {
        this->putback(currChar);
        return tok;
      }
      tok = eatComment();
      if(doReturnComments)
        return tok;
      else
      {
        tok = "";
        continue;
      }
    }
		if (checkBeginQuote(tok, tok.length()))
			return tok;

    if(!isspace(currChar))
      tok.append(1,currChar);
  }  while(!isTokEnd() || tok.length() == 0);
  return tok;
}

bool Toker::checkBeginQuote(std::string& tok, size_t toklength)
{
	if (isBeginQuote())
	{
		if (toklength > 0)
		{
			this->putback(currChar);
			return true;
		}
		tok = eatQuote();
		return true;
	}

	return false;
}
//----< test stub >--------------------------------------------

#ifdef TEST_TOKENIZER

int main(int argc, char* argv[])
{
  std::cout << "\n  Testing Tokenizer class\n "
            << std::string(25,'=') << std::endl;
  std::cout
    << "\n  Note that comments and quotes are returned as single tokens\n\n";

  // collecting tokens from a string

  Toker t_fromStr("tokens from a string: -> int x; /* a comment */", false);
  std::string tok;
  do
  {
    tok = t_fromStr.getTok();
    std::cout << "  " << tok;
  } while(tok != "");
  std::cout << "\n\n";
}

#endif
