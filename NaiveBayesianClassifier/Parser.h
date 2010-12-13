#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <fstream>
//#include <istream>
//#include <ostream>
#include <iostream>
#include "externallibs/include/expat/expat.h"

using std::string;
using std::ifstream;
using std::ofstream;
using std::cerr;

class Parser {
private:
    const int BUFFSIZE;
    ofstream out;
    void XMLCALL characterDataHandler(void *userdata, char const *d, int len);

    Parser (Parser const & p);
    Parser& operator= (Parser const & p);
public:
    Parser() : BUFFSIZE(1024) {}
    int parseFile (char const * toParseFileName, char const * outputFileName);
};

#endif // PARSER_H
