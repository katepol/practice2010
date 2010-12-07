#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <fstream>
#include <istream>
#include <ostream>
#include <iostream>

using std::string;
using std::ifstream;
using std::ofstream;
using std::cerr;

string parseFile (string const & fileName);

/*class Parser
{
private:
    Parser (Parser const & parser);
    Parser & operator= (Parser const & parser);

    bool tagOpened_;
public:
    Parser(): tagOpened_(false) {}
    string parseLine (string const & s);
    void parseFile (string const & fileName) const;

};
*/
#endif // PARSER_H
