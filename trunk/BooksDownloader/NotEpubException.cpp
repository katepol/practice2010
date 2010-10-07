#include "NotEpubException.h"

NotEpubException::NotEpubException() {}

const  char* NotEpubException:: what() const throw()
{
    return "File format != Epub.";
}

