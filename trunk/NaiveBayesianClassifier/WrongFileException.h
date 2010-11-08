#include <exception>
#include <string>

#ifndef _WrongFileException
#define _WrongFileException

class WrongFileException: public std::exception
{
    private:
        std::string message;
    public:
        virtual ~WrongFileException() throw() {}
        WrongFileException(std::string msg);
        virtual const char* what() const throw();
};

#endif
