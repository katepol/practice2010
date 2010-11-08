#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <curl/curl.h>
#include <curl/types.h>

#ifndef _MyDownloader
#define _MyDownloader

class Downloader {
private:
    std::string extention;
    std::string srcUrl;
    std::ofstream log;

    size_t WriteData(void *ptr, size_t size, size_t nmemb, FILE *myStream);
    int DownloadFile (CURL *curl, int id, FILE* f);
    void printTime (std::string const & p);

public:
    Downloader () {}
    int Download(std::ofstream * res, std::string const & category, std::string const & ext, std::string const & su, std::string const & td, std::string const & lfn, int startNum, int cnt, int faultLimit);
};

#endif
