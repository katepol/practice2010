#include "WrongFileException.h"

using std::string;
using std::exception;

WrongFileException::WrongFileException(string msg)
{
    message = msg;
}

const  char* WrongFileException:: what() const throw()
{
    return message.c_str();
}

