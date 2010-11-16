#include <iostream>
#include <fstream>
#include <string>
#include "BayesianClassifier.h"
#include "Downloader.h"

using std::string;
using std::ofstream;
using std::ifstream;

int DownloadFiles (string const & dir, string const & dataFileName)
{
    ofstream doc(dataFileName.c_str(), ofstream::trunc);
    if (!doc.is_open())
    {
        std::cout << "Cannot open file for writing data. Abort.\n";
        return 0;
    }
    Downloader d (20000, 1000);
    //REM: in extention must be . : f.e. ".txt"
    d.Download(&doc, "forum", "", "http://forumishka.net/showthread.php?t=", dir, "log_forum.txt", 1, 100);
    d.Download(&doc, "science article", "", "http://swsys.ru/index.php?page=article&id=", dir, "log_science_article.txt", 2463, 100);
    d.Download(&doc, "livejournal", "", "http://www.livejournal.ru/themes/id/", dir, "log_livejournal.txt", 22201, 100);
    doc.close();
    return 1;
}

int main(int argc, char* argv[])
{
    string dir("/home/kate/APTU/Practice/data2/");
    if (argc != 2)
    {
        std::cout << "Default working directory: " << dir << "\n";
    }
    else
    {
        dir = argv[1];
        std::cout << "Custom working directory: " << dir << "\n";
    }
    string dataFileName(dir + "description.txt");

    if (!DownloadFiles (dir, dataFileName))
        return 1;

    BayesianClassifier bc(2);

    string resultFileName(dir + "train_output.txt");
    bc.Train(dataFileName, 100, resultFileName);

    resultFileName = dir + "classify_output.txt";

    ifstream data(dataFileName.c_str(), ifstream::in);
    if (!data.is_open())
    {
        std::cerr << "Error reading data from " << dataFileName << "\n";
        return 2;
    }
    ofstream out(resultFileName.c_str(), ofstream::trunc);
    if (!out.is_open())
    {
        std::cerr << "Error writing data to " << resultFileName << "\n";
        return 3;
    }
    while (data.good())
    {
        char* c = new char;
        data.getline(c, 50);
        if (c == 0)
        {
            std::cerr << "Extracting line from " << dataFileName << " ERROR\n";
            continue;
        }
        string fname(c);
        if (fname == "")
            continue;
        fname = fname.substr(0, fname.find(' '));
        out << fname << " " << bc.Classify(dir+fname) << "\n";
        delete c;
    }
    data.close();
    out.close();

    return 0;
}
