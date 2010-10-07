#include <cstring>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <sys/time.h>

#include "NotEpubException.h"

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
        //delete tmp;  //SIGABRT
        delete ifEpub;
        throw NotEpubException();
    }
    //delete tmp; //SIGABRT
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

char* MakeStr (const char* a, int id, const char* b)
{
    char* charId = new char();
    sprintf(charId, "%d", id);

    char* s = new char [strlen(a)+strlen(charId)+strlen(b)];
    strcpy(s, a);
    strcat(s, charId);
    strcat(s, b);

    delete charId;

    return s;
}

int DownloadBook (CURL *curl, int id, FILE* f, const char* srcUrl, FILE *log)
{
    char* url = MakeStr(srcUrl, id, ".epub");
    fprintf(log, "%s\n", url);

    char errorBuffer[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    CURLcode result;
    try
    {
        result = curl_easy_perform(curl);
    }
    catch (NotEpubException& exc)
    {
        fprintf(log, "   '%s' error: %s\n", url, exc.what());
        delete url;
        return 0;
    }
    if (result == CURLE_OK)
    {
        delete url;
        return 1;
    }
    fprintf(log, "   BookId %d error: [%d] - %s\n", id, result, errorBuffer);
    delete url;
    return 0;
}

void printTime (const char* p, FILE* log)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    tm *tm1 = localtime(&tv.tv_sec);
    fprintf(log, "%s %d:%02d:%02d\n", p, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);

   // delete tm1; //SEGABRT
}

int main(int argc, char *argv[])
{
    // init curl
    CURL* curl;
    curl = curl_easy_init();
    if (!curl)
    {
        fprintf(stderr, "CURL error! Downloading aborted.\n");
        return 3;
    }

    // init parameters from arguments
    const char* srcUrl;
    const char* tgDir;
    int cnt;

    if (argc != 3)
    {
        fprintf(stdout, "You should have entered 3 arguments:\nSource url (f.e.  www.feedbooks.com/book)\nTarget directory (f.e. /home)\nQuantity of books to download.\n");
        fprintf(stdout, "The defaults are: www.feedbooks.com/book/, /home/kate/Downloads/books/, 10.\n");
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
    const char* ln = "downloadLog.txt\0";
    char *logName = new char[strlen(tgDir)+strlen(ln)];
    strcpy(logName, tgDir);
    strcat(logName, ln);
    FILE* log = fopen(logName, "w");
    if (log == 0)
    {
        fprintf(stderr, "Cannot create log file '%s'. Check the path.\nDownloading aborted.\n", logName);
        curl_easy_cleanup(curl);
        return 1;
    }
    fprintf(stdout, "Downloading log is %s.\n", logName);
    //delete ln; //SEGFAULT
    delete logName;

    // set init values
    fprintf(stdout, "Downloading ...\n");
    int downloaded = 0;
    int bookId = 0;

    printTime("Started at", log);

    while (downloaded < cnt && (bookId - downloaded) < 101)
    {
        ++bookId;
        char* fname = MakeStr(tgDir, bookId, ".epub");
        FILE* f = fopen(fname, "w");
        if (f == 0)
        {
            fprintf(stderr, "Cannot create file '%s'. Check the path.\nDownloading aborted.\n", fname);

            //dispose mem
            printTime("Finished at", log);
            fclose(log);
            //delete log; //SEGABRT
            //delete f; //SEGABRT
            delete fname;
            //delete srcUrl; //SEGFAULT
            //delete tgDir; //SEGFAULT
            curl_easy_cleanup(curl);
            //delete curl; //SEGABRT

            return 2;
        }
        fprintf(log,"Written file is <%s>\n",fname);
        if (DownloadBook(curl, bookId, f, srcUrl, log))
        {
            ++downloaded;
            fclose(f);
        }
        else
        { //если файл не загружен, удаляем созданный f
            fclose(f);
            if (remove(fname) == -1)
                fprintf(log, "       Failed to delete %s.\n", fname);
            else
                fprintf(log, "       Deleted file '%s'.\n", fname);
        }

        //delete f; //SEGABRT
        delete fname;
    }

    if ((bookId - downloaded) > 100)
        fprintf(stderr, "There were 100 SEGFAULTs in books' links. Check the source url. You have specified '%s'.\n", srcUrl);
    else
        fprintf(stdout, "Download finished!\n");

    //dispose mem
    printTime("Finished at", log);
    fclose(log);
    //delete log; //SEGABRT
    //delete srcUrl; //SEGFAULT
    //delete tgDir; //SEGFAULT
    curl_easy_cleanup(curl);
    //delete curl; //SIGABRT

    return 0;
}
