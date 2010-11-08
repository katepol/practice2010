#include <cstdlib> //for atoi()
#include <curl/curl.h>
#include <curl/types.h>
#include <sys/time.h>

#include "StringConvert.h"
#include "WrongFileException.h"
#include "Downloader.h"
#include "Checkers.h"

using std::string;
using std::ofstream;

size_t Downloader::WriteData(void *ptr, size_t size, size_t nmemb, FILE *myStream)
{
    bool firstTime = false;

    if (myStream->_IO_read_ptr == 0)
        firstTime = true;

    size_t written = fwrite(ptr, size, nmemb, myStream);

    if (firstTime)
    {
        // Check(myStream, extention);
    }

    return written;
}

size_t wd(void *ptr, size_t size, size_t nmemb, FILE *myStream)
{
    bool firstTime = false;

    if (myStream->_IO_read_ptr == 0)
        firstTime = true;

    size_t written = fwrite(ptr, size, nmemb, myStream);

    if (firstTime)
    {
        // Check(myStream, extention);
    }

    return written;
}

int Downloader::DownloadFile (CURL *curl, int id, FILE* f)
{
    string url = srcUrl + toString<int>(id) + extention;
    log << url << std::endl;

    char errorBuffer[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wd); //Downloader::WriteData
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    CURLcode result;
    try
    {
        result = curl_easy_perform(curl);
    }
    catch (WrongFileException& exc)
    {
        log << "    " << url <<" error: " << exc.what() << std::endl;
        return 0;
    }
    if (result == CURLE_OK)
    {
        return 1;
    }
    log << "    file id " << id << " error " << result << " - " << errorBuffer << std::endl;
    return 0;
}

void Downloader::printTime (string const & p)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    tm *tm1 = localtime(&tv.tv_sec);
    log << p << " " << tm1->tm_hour << ":" << tm1->tm_min << ":" << tm1->tm_sec << std::endl;
}

int Downloader::Download(ofstream * res, string const & category, string const & ext, string const & su, string const & tgDir, string const & logFileName, int startNum, int cnt, int faultLimit)
{
    extention = ext;
    srcUrl = su;

    // init curl
    CURL* curl;
    curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "CURL error! Downloading aborted.\n";
        return 1;
    }

    // initialize log file
    if (log.is_open())
        log.close();
    log.open((tgDir+logFileName).c_str(), ofstream::trunc);
    if (!log.is_open())
    {
        std::cerr << "Cannot create log file " << logFileName << " Check the path.\nDownloading aborted.\n";
        curl_easy_cleanup(curl);
        return 2;
    }
    std::cout << "Downloading log is " << logFileName << std::endl;

    // set init values
    std::cout << "Downloading ...\n";
    int downloaded = 0;
    int fileId = startNum-1;

    printTime("Started at");
    int faultNum = 0;

    while (downloaded < cnt && faultNum < faultLimit)
    {
        ++fileId;
        string fname = tgDir + toString<int>(fileId) + extention;
        FILE* f = fopen(fname.c_str(), "w");
        if (f == 0)
        {
            std::cerr << "Cannot create file " << fname << ". Check the path.\nDownloading aborted.\n";
            printTime("Finished at");
            log.close();
            curl_easy_cleanup(curl);
            return 3;
        }

        log << "Written file is "<< fname << std::endl;
        if (DownloadFile(curl, fileId, f))
        {
            *res << fileId << extention << " " << category << std::endl;
            ++downloaded;
            fclose(f);
        }
        else
        { //если файл не загружен, удаляем созданный f
            ++faultNum;
            std::cerr << faultNum << "\n";
            fclose(f);
            if (remove(fname.c_str()) == -1)
                log << "       Failed to delete " << fname << std::endl;
            else
                log << "       Deleted file " << fname << std::endl;
        }
    }
    if (faultNum >= faultLimit)
        std::cerr << "There were "<< faultNum << " faults in files' links. Check the source url. You have specified " << srcUrl << std::endl;
    else
        std::cout << "Download finished!\n";

    printTime("Finished at");
    log.close();
    curl_easy_cleanup(curl);
    return 0;
}
