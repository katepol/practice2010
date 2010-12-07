#include "Parser.h"
/*
string Parser::parseLine (string const & s)
{
    string r("");
    size_t k = (tagOpened_) ? s.find_first_of('>') : s.find_first_of('<');
    if (k != string::npos)
    {

        tagOpened_ = !tagOpened_;

    }
}

void Parser::parseFile (string const & fileName) const*/

string parseFile (string const & fileName)
{
    ifstream in(fileName.c_str(), ifstream::in);
    if (!in.is_open())
    {
        cerr << "Error opening input file " << fileName << " for parsing. Aborted.\n";
        return "";
    }
    size_t k = fileName.find_last_of('/');
    string outFileName = (k == string::npos) ? "" : fileName.substr(0, k+1);
    outFileName += "tmp";
    ofstream out(outFileName.c_str(), ofstream::trunc);
    if (!out.is_open())
    {
        cerr << "Error opening output file " << fileName << " for parsing. Aborted.\n";
        return "";
    }

    bool tagOpened = false;
    while (!in.eof())
    {
        unsigned char c = in.get();
        if (tagOpened)
        {
            if (c == '>')
                tagOpened = false;
        }
        else
        {
            if (c == '<')
                tagOpened = true;
            else {
                //if (c != '\r')
                    out << c;
            }
        }
    }
    in.close();
    out.close();
    return outFileName;
}
