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

// singletone
class Parser {
private:
	// static Parser * instance_;
    static const int BUFFSIZE = 1024;
    static ofstream out;

    Parser() {}
    ~Parser();
    Parser (Parser const & p);
    Parser& operator= (Parser const & p);
    friend void XMLCALL characterDataHandler(void *userdata, char const *d, int len);
public:
    static int parseFile (char const * toParseFileName, char const * outputFileName);
    //Parser & getInstance();
};

#endif // PARSER_H
