#include <cstring>
#include <cstdlib> //for atoi()
#include <curl/curl.h>
#include <curl/types.h>
#include <sys/time.h>
#include <fstream>
#include <iostream>

#include "StringConvert.h"
#include "NotEpubException.h"

typedef std::string string;
typedef std::ofstream ofstream;

void testIsEpub (FILE* f)
// as I've noticed every *.epub file contains string "epub"
// which is on 50-53 positions. Here I check whether "epub" is contained.
{
    char* tmp = f->_IO_read_ptr + 50;
    char* ifEpub = new char[5];
    strncpy(ifEpub, tmp, 4); //save symbols[50-53] to ifEpub
    ifEpub[4] = '\0';
    if (strcmp(ifEpub, "epub")) //==1 when !equal
    {
        delete ifEpub;
        throw NotEpubException();
    }
    delete ifEpub;
}

size_t WriteData(void *ptr, size_t size, size_t nmemb, FILE *myStream)
{
    bool firstTime = false;
    if (myStream->_IO_read_ptr == 0)
        firstTime = true;

    size_t written = fwrite(ptr, size, nmemb, myStream);

    if (firstTime)
    { //at first time: confirm it is *.epub
        testIsEpub (myStream);
    }

    return written;
}

int DownloadBook (CURL *curl, int id, FILE* f, string srcUrl, ofstream * log)
{
    string url = srcUrl + toString<int>(id) + ".epub";
    *log << url << std::endl;

    char errorBuffer[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    CURLcode result;
    try
    {
        result = curl_easy_perform(curl);
    }
    catch (NotEpubException& exc)
    {
        *log << "    " << url <<" error: " << exc.what() << std::endl;
        return 0;
    }
    if (result == CURLE_OK)
    {
        return 1;
    }
    *log << "    file id " << id << "error " << result << " - " << errorBuffer << std::endl;
    return 0;
}

void printTime (string p, ofstream *log)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    tm *tm1 = localtime(&tv.tv_sec);
    *log << p << " " << tm1->tm_hour << ":" << tm1->tm_min << ":" << tm1->tm_sec << std::endl;
}

int main(int argc, char *argv[])
{
    // init curl
    CURL* curl;
    curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "CURL error! Downloading aborted.\n";
        return 1;
    }

    // init parameters from arguments
    string srcUrl;
    string tgDir;
    int cnt;

    if (argc != 3)
    {
        std::cout <<"You should have entered 3 arguments:\nSource url (f.e.  www.feedbooks.com/book)\nTarget directory (f.e. /home)\nQuantity of books to download.\nThe defaults are: www.feedbooks.com/book/, /home/kate/Downloads/books/, 10.\n";
        srcUrl = "www.feedbooks.com/book/";
        tgDir = "/home/kate/Downloads/books/";
        cnt = 10;
    }
    else
    {
        srcUrl = argv[0];
        tgDir = argv[1] ;
        cnt = atoi(argv[2]);
    }

    // initialize log file
    string logName = tgDir + "downloadLog.txt";
    ofstream log(logName.c_str(), ofstream::trunc);
    if (!log.is_open())
    {
        std::cerr << "Cannot create log file " << logName << " Check the path.\nDownloading aborted.\n";
        curl_easy_cleanup(curl);
        return 2;
    }
    std::cout << "Downloading log is " << logName << std::endl;

    // set init values
    std::cout << "Downloading ...\n";
    int downloaded = 0;
    int bookId = 0;

    printTime("Started at", &log);

    while (downloaded < cnt && (bookId - downloaded) < 101)
    {
        ++bookId;
        string fname = tgDir + toString<int>(bookId) + ".epub";
        FILE* f = fopen(fname.c_str(), "w");
        if (f == 0)
        {
            std::cerr << "Cannot create file " << fname << ". Check the path.\nDownloading aborted.\n";
            printTime("Finished at", &log);
            log.close();
            curl_easy_cleanup(curl);
            return 3;
        }

        log << "Written file is "<< fname << std::endl;
        if (DownloadBook(curl, bookId, f, srcUrl, &log))
        {
            ++downloaded;
            fclose(f);
        }
        else
        { //если файл не загружен, удаляем созданный f
            fclose(f);
            if (remove(fname.c_str()) == -1)
                log << "       Failed to delete " << fname << std::endl;
            else
                log << "       Deleted file " << fname << std::endl;
        }
    }
    if ((bookId - downloaded) > 100)
        std::cerr << "There were 100 SEGFAULTs in books' links. Check the source url. You have specified " << srcUrl << std::endl;
    else
        std::cout << "Download finished!\n";

    printTime("Finished at", &log);
    log.close();
    curl_easy_cleanup(curl);
    return 0;
}
