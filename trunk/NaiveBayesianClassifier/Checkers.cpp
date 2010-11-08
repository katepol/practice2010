#include <cstdio>
#include <cstring>

#include "WrongFileException.h"
#include "Checkers.h"

using std::string;

void testIsCorrect (FILE* f, int shift, char const * str, string const & msg)
{
    char* tmp = f->_IO_read_ptr + shift;
    char ifCorrect[strlen(str)];
    strncpy(ifCorrect, tmp, strlen(str));
    if (strcmp(ifCorrect, str)) //==1 when !equal
    {
        throw WrongFileException(msg);
    }
}

void Check (FILE* f, std::string const & ext)
{
    if (ext == "forum")
        testIsCorrect(f, 5000, "error", "Bad forum page.");
    else
        if (ext == "science_article")
            testIsCorrect(f, 5000, "", "Bad science article page.");
        else
            if (ext == "livejournal")
                testIsCorrect(f, 5000, "", "Bad livejournal page.");
            else
                throw WrongFileException("Unexpected format '" + ext + "'.");
}
