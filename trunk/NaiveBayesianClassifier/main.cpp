#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

#include "BayesianClassifier.h"
#include "Downloader.h"

using std::string;
using std::ofstream;
using std::ifstream;
using std::cout;
using std::cerr;

int DownloadFiles (string const & dir, string const & dataFileName)
{
    ofstream doc(dataFileName.c_str(), ofstream::trunc);
    if (!doc.is_open())
    {
        cout << "Cannot open file for writing data. Abort.\n";
        return 0;
    }
    //REM: in extention must be . : f.e. ".txt"
    int minFileSize = 20000;
    int faultLimit = 1000;
    //Downloader::download(&doc, "forum", "", "http://forum.nic.ru/showthread.php?t=", dir, "log_forum.txt", 1, 100, 20000, 2000);
    Downloader d;
    d.download(&doc, "forum", "", "http://forum.ubuntu.ru/index.php?topic=", dir, "log_forum.txt", 400, 100, minFileSize, faultLimit);
    d.download(&doc, "science article", "", "http://swsys.ru/index.php?page=article&id=", dir, "log_science_article.txt", 2463, 100, minFileSize, faultLimit);
    d.download(&doc, "livejournal", "", "http://www.livejournal.ru/themes/id/", dir, "log_livejournal.txt", 22201, 100, minFileSize, faultLimit);
    d.download(&doc, "news", "", "http://russiaregionpress.ru/archives/", dir, "log_news.txt", 97000, 100, 46300, 3*faultLimit);

    doc.close();
    return 1;
}

int Classify (string const & dir, string const & dataFileName)
{
    unsigned int importance = 2;

    string resultFileName(dir + "train_output.txt");

    BayesianClassifier::getInstance()->train(dataFileName, 100, importance, resultFileName);

    resultFileName = dir + "classify_output.txt";

    ifstream data(dataFileName.c_str(), ifstream::in);
    if (!data.is_open())
    {
        cerr << "Error reading data from " << dataFileName << "\n";
        return 2;
    }
    ofstream out(resultFileName.c_str(), ofstream::trunc);
    if (!out.is_open())
    {
        cerr << "Error writing data to " << resultFileName << "\n";
        return 3;
    }
    while (data.good())
    {
        char* c = new char;
        data.getline(c, 50);
        if (c == 0)
        {
            cerr << "Extracting line from " << dataFileName << " ERROR\n";
            continue;
        }
        string fname(c);
        if (fname == "")
            continue;
        fname = fname.substr(0, fname.find(' '));
        out << fname << " " << BayesianClassifier::getInstance()->classify(dir+fname, importance) << "\n";
        delete c;
    }
    data.close();
    out.close();
    return 0;
}

int checkClassify (string const & trueInfo, string const & classifyOutput, string const & outp)
{
    ifstream in(trueInfo.c_str(), ifstream::in);
    if (!in.is_open())
    {
        cerr << "Error reading data from " << trueInfo << "\n";
        return 1;
    }
    ifstream totest(classifyOutput.c_str(), ifstream::in);
    if (!totest.is_open())
    {
        cerr << "Error reading data from " << classifyOutput << "\n";
        return 2;
    }

    mapSmapSI catcnt;

    mapsi nd = BayesianClassifier::getInstance()->getNdocs();
    for (mapsi::const_iterator i = nd.begin(); i != nd.end(); ++i)
    {
        catcnt.insert(std::pair<string, mapsi>(i->first, mapsi()));
        for (mapsi::const_iterator j = nd.begin(); j != nd.end(); ++j)
            catcnt[i->first].insert(std::pair<string, int>(j->first, 0));
    }


    while (in.good() && totest.good())
    {
        char * s1 = new char;
        char * s2 = new char;
        in.getline(s1, 50);
        totest.getline(s2, 50);
        if (s1 == 0 || s2 == 0)
        {
            cerr << "Extracting line ERROR\n";
            continue;
        }
        string c1(s1);
        string c2(s2);
        delete s1;
        delete s2;
        if (c1 == "" || c2 == "")
        {
            continue;
        }
        if (c1.substr(0, c1.find(' ')) != c2.substr(0, c2.find(' ')))
        {
            cerr << "filenames dont match\n";
            continue;
        }

        string t1(c1);
        string t2(c2);
        c1 = c1.substr(c1.find(' ')+1);
        c2 = c2.substr(c2.find(' ')+1);
        if (c1[1] == ' ' || c2[1]==' ')
        {
            cerr << "fuck '" << t1 <<"' ; '" << t2 << "'\n";
        }

        catcnt[c1][c2] += 1;
    }
    in.close();
    totest.close();

    ofstream out(outp.c_str(), ofstream::trunc);
    if (!out.is_open())
    {
        cerr << "Error writing data\n";
        return 3;
    }

    out << "true        ";
    for (mapSmapSI::const_iterator i = catcnt.begin(); i != catcnt.end(); ++i)
        out << i->first << "        ";
    out << "\n";

    for (mapSmapSI::const_iterator i = catcnt.begin(); i != catcnt.end(); ++i)
    {
        out << i->first << "       ";
        for (mapsi::const_iterator j = (i->second).begin(); j != (i->second).end(); ++j)
        {
            out << j->second << "       ";
        }
        out << "\n";
    }

    out.close();
}

int main(int argc, char* argv[])
{
    string dir("/home/kate/APTU/Practice/data2/");
    string dataFileName("description.txt");
    if (argc != 2)
    {
        cout << "Default working directory: " << dir << "\n";
    }
    else
    {
        dir = argv[1];
        cout << "Custom working directory: " << dir << "\n";
    }
    dataFileName = dir + dataFileName;

//    if (!DownloadFiles (dir, dataFileName))        return 1;

    Classify(dir, dataFileName);

    checkClassify(dataFileName, dir+"classify_output.txt", dir+"analyisis.txt");

    cout << "finish\n";

    return 0;

}
