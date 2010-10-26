#include <iostream>
#include <fstream>
#include <string>
#include "BayesianClassifier.h"

typedef std::string string;

void makeDataFile(string const & s)
{
    std::ofstream doc(s.c_str(), std::ofstream::trunc);
    if (!doc.is_open())
        return;
    for (int i=1; i!=101; ++i)
        doc << i << " forum\n";
    for (int i=2463; i!=2563; ++i)
        doc << i << " science article\n";
    for (int i=22201; i!=22301; ++i)
        doc << i << " livejournal\n";
    doc.close();
}

int main(int argc, char* argv[])
{
    string dir("/home/kate/APTU/Practice/data/");
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
    makeDataFile(dataFileName);

    BayesianClassifier bc;

    string resultFileName(dir + "train_output.txt");
    bc.Train(dataFileName, 100, 2, resultFileName);

    resultFileName = dir + "classify_output.txt";

    std::ifstream data(dataFileName.c_str(), std::ifstream::in);
    if (!data.is_open())
    {
        std::cerr << "Error reading data from " << dataFileName << "\n";
        return 1;
    }
    std::ofstream out(resultFileName.c_str(), std::ofstream::trunc);
    if (!out.is_open())
    {
        std::cerr << "Error writing data to " << resultFileName << "\n";
        return 2;
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
