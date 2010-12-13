#include "Parser.h"

/*void XMLCALL Parser::characterDataHandler(void * userdata, XML_Char const *d, int len)
{
    for (int i=0; i<len; ++i)
        out << d[i];
}
*/
int Parser::parseFile (char const * toParseFileName, char const * outputFileName)
{
    out.open(outputFileName, ofstream::trunc);
    if (!out.is_open())
    {
        std::cerr << "Cannot create output file " << outputFileName << ". Abort.\n";
        return 1;
    }
    XML_Parser parser = XML_ParserCreate(NULL);
    //XML_SetCharacterDataHandler(parser, characterDataHandler);
    FILE * inp = fopen(toParseFileName, "r");
    if (inp == 0)
    {
        std::cerr << "Cannot open file to parse "<< toParseFileName <<". Abort.\n";
        return 2;
    }
    int done, len;
    char Buff[BUFFSIZE];
    do
    {
        len = fread(Buff, 1, BUFFSIZE, inp);
        done = feof(inp);
        XML_Parse(parser, Buff, len, done);
    } while (!done);
    fclose(inp);
    out.close();
    XML_ParserFree(parser);
    return 0;
}

