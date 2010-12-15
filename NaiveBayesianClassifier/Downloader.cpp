#include "Downloader.h"

Downloader * Downloader::instance_ = new Downloader();

size_t Downloader::writeData(void *ptr, size_t size, size_t nmemb, FILE *myStream)
{
    return fwrite(ptr, size, nmemb, myStream);
}

int Downloader::downloadFile (CURL *curl, int id, FILE* f, int minFileSize)
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
        if (ftell(f) < minFileSize)
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
    instance_->extention_ = ext;
    instance_->srcUrl_ = su;

    // init curl
    CURL* curl;
    curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "CURL error! Downloading aborted.\n";
        return 1;
    }

    // initialize log file
    if (instance_->log_.is_open())
        instance_->log_.close();
    instance_->log_.open((tgDir+logFileName).c_str(), ofstream::trunc);
    if (!instance_->log_.is_open())
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

    instance_->printTime("Started at");
    int faultNum = 0;

    while (downloaded < cnt && faultNum < faultLimit)
    {
        ++fileId;
        string shortFileName = toString<int>(fileId) + category[0] + instance_->extention_;
        string fullFileName = tgDir + shortFileName;
        FILE* f = fopen(fullFileName.c_str(), "w");
        if (f == 0)
        {
            std::cerr << "Cannot create file " << fullFileName << ". Check the path.\nDownloading aborted.\n";
            instance_->printTime("Finished at");
            instance_->log_.close();
            curl_easy_cleanup(curl);
            return 3;
        }

        instance_->log_ << "Written file is "<< fullFileName << std::endl;
        if (instance_->downloadFile(curl, fileId, f, minFileSize))
        {
            *res << shortFileName << " " << category << std::endl;
            ++downloaded;
            fclose(f);
        }
        else
        { //если файл не загружен, удаляем созданный f
            ++faultNum;
            //std::cerr << faultNum << "\n";
            fclose(f);
            if (remove(fullFileName.c_str()) == -1)
                instance_->log_ << "       Failed to delete " << fullFileName << std::endl;
            else
                instance_->log_ << "       Deleted file " << fullFileName << std::endl;
        }
    }
    if (faultNum >= faultLimit)
        std::cerr << "There were "<< faultNum << " faults in files' links. Check the source url. You have specified " << instance_->srcUrl_ << std::endl;
    else
        std::cout << "Download finished!\n";

    instance_->printTime("Finished at");
    instance_->log_.close();
    curl_easy_cleanup(curl);
    return 0;
}
