#include "BayesianClassifier.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "PrintTemplates.cpp"

typedef std::string                         string;
typedef std::list<std::string>              lststr;
typedef std::vector<std::string>            vecstr;
typedef std::map<std::string, int>          mapsi;
typedef std::map<std::string, double>       mapsd;
typedef std::map<std::string, std::map<std::string, int> >     mapSmapSI;
typedef std::map<std::string, std::map<std::string, double> >  mapSmapSD;
typedef std::ifstream ifstream;
typedef std::ofstream ofstream;

mapsi BayesianClassifier::MakeMap (lststr const & list)
{
    if (list.empty())
        return mapsi();
    mapsi result;
    for (lststr::const_iterator i = list.begin(); i != list.end(); ++i)
    {
        mapsi::const_iterator j = result.find(*i);
        if (j != result.end())
            ++result[*i];
        else
            result.insert (std::pair<string, int>(*i, 1));
    }
    return result;
}

void BayesianClassifier::MergeMaps (mapsi map, string const & cat)
{
    for (mapsi::iterator i = map.begin(); i != map.end(); ++i)
    {
        if (counter.find(i->first) != counter.end())
        {// такой признак уже есть, увеличиваем счетчик категории (если она есть)
            if (counter[i->first].find(cat) != counter[i->first].end())
            {// такая категория есть, увеличиваем счетчик
                counter[i->first][cat] += i->second;
            }
            else
            {// добавляем категорию
                counter[i->first].insert(std::pair<string, int>(cat, i->second));
            }
        }
        else
        {// добавляем признак и категорию в него
            counter.insert(std::pair<string, mapsi>(i->first, mapsi()));
            counter[i->first].insert(std::pair<string, int>(cat, i->second));
        }
    }
}

void BayesianClassifier::MakeProb(int weight, double aprioryProbability)
{
    for (mapSmapSI::const_iterator i = counter.begin(); i != counter.end(); ++i)
    {
        probability.insert(std::pair<string, mapsd>(i->first, mapsd()));
        // подсчет суммы вхождений признака во все документы
        int sum = 0;
        for (mapsi::const_iterator j = (i->second).begin(); j != (i->second).end(); ++j)
        {
            sum += j->second;
        }
        for (mapsi::const_iterator j = (i->second).begin(); j != (i->second).end(); ++j)
        {
            double p = (double)j->second/sum;
            double wp = ((weight*aprioryProbability) + (sum*p))/(weight + sum);
            probability[i->first].insert(std::pair<string, double>(j->first, wp));
        }
    }
}

lststr BayesianClassifier::ParseDocument (string const &docFileName, unsigned int importance)
{
    ifstream doc(docFileName.c_str(), ifstream::in);
    if (!doc.is_open())
    {
        std::cerr << "Cannot open document " << docFileName << " Check the path.\nParsing aborted.\n";
        return lststr();
    }
    // std::cout << "Parsing document " << docFileName << std::endl;

    lststr word;
    while (!doc.eof())
    { // to lower case
      // punctuation
      string s;
      doc >> s;
      if (s.length() > importance)
        word.push_back(s);
    }
    doc.close();
    word.sort();
    return word;
}

int BayesianClassifier::TrainOnFile (string const & fileName, string const & cat, unsigned int importance)
{
    lststr v = ParseDocument(fileName, importance);
    mapsi m = MakeMap(v);
    // std::cout << "  training on " << fileName << "...\n";
    MergeMaps(m, cat);

    mapsi::iterator i = ndocs.find(cat);
    if (i != ndocs.end())
    { // увеличиваем счетчик документов по категории
        ++ndocs[cat];
    }
    else
    {
        ndocs.insert (std::pair<string, int>(cat, 1));
    }
    return 0;
}

int BayesianClassifier::Train (string dataFileName, unsigned int examplesQty, unsigned int importance, string resultFileName)
{
    ifstream data(dataFileName.c_str(), ifstream::in);
    if (!data.is_open())
    {
        std::cerr << "Cannot open data file " << dataFileName << " Check the path.\nTraining aborted.\n";
        return 1;
    }
    std::cout << "Training...\n";

    vecstr files;

    while (!data.eof())
    {
        char* s = new char;
        data.getline(s, 100);
        if (s == 0)
        {
            std::cerr << "Extracting line from " << dataFileName << " ERROR\n";
            continue;
        }
        string line(s);
        files.push_back(line);
        delete s;
    }
    data.close();

    if (files.size() < examplesQty)
    {
        std::cerr << "Not enough data for training! Training aborted.\n";
        return 2;
    }

    ofstream out(resultFileName.c_str(), ofstream::trunc);
    if (!out.is_open())
    {
        std::cerr << "Cannot open output file " << resultFileName << " Check the path.\nTraining not logging.\n";
    }

    // выбор random examplesQty образцов для обучения
    string tgDir = dataFileName.substr(0, dataFileName.rfind('/')+1);
    unsigned int nTrained = 0;
    int nTotal = files.size()-1;
    srand (time(NULL));

    std::list<int> tested;

    while (nTrained < examplesQty)
    {
        // чтобы k не повторялись
        int k = rand() % nTotal;
        while (std::find(tested.begin(), tested.end(), k) != tested.end())
            k = rand() % nTotal;
        tested.push_back(k);

        int q = files[k].find(' ');
        string fname = files[k].substr(0, q);
        string cat   = files[k].substr(q+1);

        if (TrainOnFile(tgDir+fname, cat, importance) == 0)
        {
            ++nTrained;
            if (out.good())
                out << fname << " " << cat << " : training\n";
        }
    }
    if (out.is_open())
        out.close();
   // printMM (counter);
    std::cout << "ndocs\n";
    printM (ndocs);
    std::cout << "vocabulary size = " << counter.size() <<"\n";

    // устанавливаем вероятности
    MakeProb(1, 0.5);
    //printMM (probability);

    return 0;
}

mapsd BayesianClassifier::DocumentProbability (string const & fileName)
{// ~ метод не очень-то :(
    lststr list = ParseDocument(fileName, 2);
    list.unique();

    mapsd result;
    for (mapsi::const_iterator i = ndocs.begin(); i != ndocs.end(); ++i)
        result.insert(std::pair<string, double>(i->first, 1));

    for(lststr::const_iterator i = list.begin(); i != list.end(); ++i)
    {
        mapSmapSD::const_iterator j = probability.find(*i);
        if (j != probability.end())
        { // такой признак видели, классифицируем  ---> А ЕСЛИ НЕТ?
            for (mapsi::const_iterator j = ndocs.begin(); j != ndocs.end(); ++j)
                result[j->first] *= probability[*i][j->first];
        }
    }
    return result;
}

string BayesianClassifier::Classify (string const & fileName)
{
    mapsd pDocCat = DocumentProbability(fileName);
    // общее число документов
    int total = 0;
    for (mapsi::const_iterator i = ndocs.begin(); i != ndocs.end(); ++i)
        total += i->second;

    mapsd pCatDoc;
    for (mapsi::const_iterator i = ndocs.begin(); i != ndocs.end(); ++i)
    {
        double p = pDocCat[i->first] * (double)i->second / total;
        pCatDoc.insert(std::pair<string, double>(i->first, p));
    }

    // ищем максимальное pCatDoc
    mapsd::iterator max = pCatDoc.begin();
    for (mapsd::iterator i = pCatDoc.begin(); i != pCatDoc.end(); ++i)
        if (i->second > max->second)
            max = i;

    return max->first;
}
