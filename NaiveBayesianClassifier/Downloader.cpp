#include "Downloader.h"

size_t Downloader::writeData(void *ptr, size_t size, size_t nmemb, FILE *myStream)
{
    return fwrite(ptr, size, nmemb, myStream);
}

int Downloader::downloadFile (CURL *curl, int id, FILE* f)
{
    string url = srcUrl_ + toString<int>(id) + extention_;
    log_ << url << std::endl;
    char errorBuffer[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Downloader::writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    CURLcode result = curl_easy_perform(curl);
    if (result == CURLE_OK)
    {
        // check for file size
        if (ftell(f) < minFileSize_)
        {
            log_ << "    file id " << id << " error: FILE SEEMS TO BE EMPTY. SKIP.\n";
            return 0;
        }
        return 1;
    }
    log_ << "    file id " << id << " error " << result << " - " << errorBuffer << std::endl;
    return 0;
}

void Downloader::printTime (string const & p)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    tm *tm1 = localtime(&tv.tv_sec);
    log_ << p << " " << tm1->tm_hour << ":" << tm1->tm_min << ":" << tm1->tm_sec << std::endl;
}

int Downloader::download(ofstream * res, string const & category, string const & ext, string const & su, string const & tgDir, string const & logFileName, int startNum, int cnt, int minFileSize, int faultLimit)
{
    minFileSize_ = minFileSize;
    faultLimit_ = faultLimit;
    extention_ = ext;
    srcUrl_ = su;

    // init curl
    CURL* curl;
    curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "CURL error! Downloading aborted.\n";
        return 1;
    }

    // initialize log file
    if (log_.is_open())
        log_.close();
    log_.open((tgDir+logFileName).c_str(), ofstream::trunc);
    if (!log_.is_open())
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

    while (downloaded < cnt && faultNum < faultLimit_)
    {
        ++fileId;
        string fname = tgDir + toString<int>(fileId) + extention_;
        FILE* f = fopen(fname.c_str(), "w");
        if (f == 0)
        {
            std::cerr << "Cannot create file " << fname << ". Check the path.\nDownloading aborted.\n";
            printTime("Finished at");
            log_.close();
            curl_easy_cleanup(curl);
            return 3;
        }

        log_ << "Written file is "<< fname << std::endl;
        if (downloadFile(curl, fileId, f))
        {
            *res << fileId << extention_ << " " << category << std::endl;
            ++downloaded;
            fclose(f);
        }
        else
        { //если файл не загружен, удаляем созданный f
            ++faultNum;
            //std::cerr << faultNum << "\n";
            fclose(f);
            if (remove(fname.c_str()) == -1)
                log_ << "       Failed to delete " << fname << std::endl;
            else
                log_ << "       Deleted file " << fname << std::endl;
        }
    }
    if (faultNum >= faultLimit_)
        std::cerr << "There were "<< faultNum << " faults in files' links. Check the source url. You have specified " << srcUrl_ << std::endl;
    else
        std::cout << "Download finished!\n";

    printTime("Finished at");
    log_.close();
    curl_easy_cleanup(curl);
    return 0;
}
