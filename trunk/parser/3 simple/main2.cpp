#include <stdio.h>
#include <expat.h>

#include <iostream>

#define BUFFSIZE 1024
char Buff[BUFFSIZE];
int Depth;
XML_Parser first;

static void XMLCALL start(void *data, const char *tag, const char **attr)
{
    int i;
    for (i = 0; i < Depth; i++) printf(" |->");
    printf("tag=%s userdata=%s\n", tag, (char*)data);

    for (i = 0; attr[i]; i += 2)
    {
	printf("\tattrib %s='%s'\n", attr[i], attr[i + 1]);
    }
    Depth++;
}

static void XMLCALL end(void *userdata, const char *tag)
{
    Depth--;
}

static void XMLCALL characterDataHandler(void *userdata, char const *d, int len)
    {
        printf ("DATA LENGTH=%d, DATA=%s\nEND_DATA\n", len, (d==0) ? "(null)" : d);
    }

int main (void)
{
    int done, len;
    first = XML_ParserCreate(NULL);
    XML_SetElementHandler(first, start, end);
    //XML_SetCharacterDataHandler(first, characterDataHandler);
    FILE* inp = fopen("/home/kate/APTU/Practice/ExpatTest/easy.xml", "r");
    if (inp==0)
    {
    	std::cerr << "Cannot open file to parse. Abort.\n";
        return 1;
    }

    do
    {
        len = fread(Buff, 1, BUFFSIZE, inp);
        done = feof(inp);
	XML_Parse(first, Buff, len, done);
    } while (!done);

    fclose(inp);
    XML_ParserFree(first);
    return 0;
}
