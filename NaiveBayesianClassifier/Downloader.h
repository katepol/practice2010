#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>

#include "externallibs/include/curl/curl.h"
#include "externallibs/include/curl/types.h"

#include <cstdlib> //for atoi()
#include <sys/time.h>
#include "StringConvert.h"

#ifndef _MyDownloader
#define _MyDownloader

using std::string;
using std::ofstream;

class Downloader
{
private:
    int minFileSize_;
    int faultLimit_;
    string extention_;
    string srcUrl_;
    ofstream log_;

    static size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *myStream);
    int downloadFile (CURL *curl, int id, FILE* f);
    void printTime (string const & p);

public:
    Downloader () {}
    int download(ofstream * res,
                 string const & category,
                 string const & ext,
                 string const & su,
                 string const & td,
                 string const & lfn,
                 int startNum,
                 int cnt,
                 int minFileSize,
                 int faultLimit);
};

#endif
