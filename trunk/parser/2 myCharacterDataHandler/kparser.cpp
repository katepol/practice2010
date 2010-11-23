#include <stdio.h>
#include <expat.h>
#include <iostream>
#include <fstream>

#ifndef KPARSER_CPP
#define KPARSER_CPP

#define BUFFSIZE 500
char Buff[BUFFSIZE];
int Depth=0;


static void XMLCALL start(void *data, const char *tag, const char **attr)
{
    int i;
        for (i = 0; i < Depth; i++)
            std::cout << " |->";
        //std::cout << "tag=" << tag << " userdata=" << (char*)data << "\n";
        printf("tag=%s userdata=%s\n", tag, (char*)data);

        for (i = 0; attr[i]; i += 2)
        {
            //printf("\tattrib %s='%s'\n", attr[i], attr[i + 1]);
            std::cout << "\tattr " << attr[i] << "='" << attr[i+1] << "'\n";
        }
        Depth++;
    }

    static void XMLCALL end(void *userdata, const char *tag)
    {
        Depth--;
        for (int i = 0; i < Depth; i++)
            std::cout << " |->";
        printf("\\tag=%s userdata=%s\n", tag, (char*)userdata);
        //std::cout << "\\" << tag << "; userdata=" << (char*)userdata <<"\n";
    }

    static void XMLCALL characterDataHandler(void *userdata, char const *d, int len)
    {
        printf ("DATA LENGTH=%d, DATA=%s\nEND_DATA\n", len, (d==0) ? "(null)" : d);
    }

    int simpleParse (char const * fname) // "/home/kate/APTU/Practice/ExpatTest/lab8.html"
    {
        //int done, len;
        XML_Parser first = XML_ParserCreate(NULL);

        XML_SetElementHandler(first, start, end);

        //XML_SetCharacterDataHandler(first, XML_CharacterDataHandler());
        XML_SetCharacterDataHandler(first, characterDataHandler);

        FILE* inp = fopen(fname, "r");
        if (inp==0)
        {
            std::cerr << "Cannot open file to parse. Abort.\n";
            return 0;
        }

        int done = 0;
        do
        {
            int len = fread(Buff, 1, BUFFSIZE, inp);
            done = feof(inp);
            XML_Parse(first, Buff, len, done);
        } while (!done);

        fclose(inp);
        XML_ParserFree(first);
        return 1;
    }


#endif // KPARSER_CPP
