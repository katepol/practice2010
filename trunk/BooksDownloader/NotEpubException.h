#include <exception>

#ifndef _NotEpubException
#define _NotEpubException

class NotEpubException: public std::exception
{
    public:
        NotEpubException();
        virtual const char* what() const throw();
};

#endif
