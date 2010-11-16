#include "BayesianClassifier.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "PrintTemplates.cpp"

using std::string;
using std::ifstream;
using std::ofstream;
typedef std::list<std::string>              lststr;
typedef std::vector<std::string>            vecstr;
typedef std::map<std::string, int>          mapsi;
typedef std::map<std::string, double>       mapsd;
typedef std::map<std::string, std::map<std::string, int> >     mapSmapSI;
typedef std::map<std::string, std::map<std::string, double> >  mapSmapSD;

mapsi BayesianClassifier::MakePreCounter (lststr const & list)
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

mapSmapSI BayesianClassifier::MakeSimpleCounter (mapsi const & preCounter, string const & cat)
{
    mapSmapSI cnt;
    for (mapsi::const_iterator i = preCounter.begin(); i != preCounter.end(); ++i)
    {
        cnt.insert(std::pair<string, mapsi>(i->first, mapsi())); // вставили признак
        for (mapsi::const_iterator j = ndocs.begin(); j != ndocs.end(); ++j)
        {
            int k = (j->first == cat) ? i->second : 0;
            cnt[i->first].insert(std::pair<string, int>(j->first, k));
        }
    }
    return cnt;
}

void BayesianClassifier::MergeCounters (mapSmapSI & cnt, mapsi const & map, string const & cat)
{
    for (mapsi::const_iterator i = map.begin(); i != map.end(); ++i)
    {
        if (cnt.find(i->first) != cnt.end())
        {// такой признак уже есть, увеличиваем счетчик категории (если она есть)
            if (cnt[i->first].find(cat) != cnt[i->first].end())
            {// такая категория есть, увеличиваем счетчик
                cnt[i->first][cat] += i->second;
            }
            else
            {// добавляем категорию
                cnt[i->first].insert(std::pair<string, int>(cat, i->second));
            }
        }
        else
        {// добавляем признак и все категории в него
            cnt.insert(std::pair<string, mapsi>(i->first, mapsi()));
            cnt[i->first].insert(std::pair<string, int>(cat, i->second));
        }
    }
}

void BayesianClassifier::MergeProbs (mapSmapSD const & prob)
{
    for (mapSmapSD::const_iterator i = prob.begin(); i != prob.end(); ++i)
    {
        probability.insert(std::pair<string, mapsd>(i->first, i->second));
    }
}

mapSmapSD BayesianClassifier::MakeProb(mapSmapSI & cnt, int weight, double aprioryProbability)
{
    mapSmapSD res;

    for (mapSmapSI::const_iterator i = cnt.begin(); i != cnt.end(); ++i)
    {
        res.insert(std::pair<string, mapsd>(i->first, mapsd()));
        // подсчет суммы вхождений признака во все документы
        int sum = 0;
        for (mapsi::const_iterator j = (i->second).begin(); j != (i->second).end(); ++j)
        {
            sum += j->second;
        }

        for (mapsi::const_iterator j = ndocs.begin(); j != ndocs.end(); ++j)
        {
            double p = 0;
            if (cnt[i->first].find(j->first) != cnt[i->first].end())
                p = (double)cnt[i->first][j->first]/sum;
            double wp = ((weight*aprioryProbability) + (sum*p))/(weight + sum);

            res[i->first].insert(std::pair<string, double>(j->first, wp));
        }
    }
    return res;
}

mapsi BayesianClassifier::ParseDocument (string const &docFileName)
{
    ifstream doc(docFileName.c_str(), ifstream::in);
    if (!doc.is_open())
    {
        std::cerr << "Cannot open document " << docFileName << " Check the path.\nParsing aborted.\n";
        return mapsi();
    }
    // std::cout << "Parsing document " << docFileName << std::endl;

    lststr word;
    while (!doc.eof())
    { // to lower case
      // punctuation
      // stemming
      string s;
      doc >> s;
      //int tolower(int);
      if (s.length() > importance)
        word.push_back(s);
    }
    doc.close();
    word.sort();
    return MakePreCounter(word);
}

int BayesianClassifier::TrainOnFile (string const & fileName, string const & cat, mapSmapSI & cnt)
{
    mapsi m = ParseDocument(fileName);

    // std::cout << "  training on " << fileName << "...\n";   
    MergeCounters(cnt, m, cat);

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

int BayesianClassifier::Train (string dataFileName, unsigned int examplesQty, string resultFileName)
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
        std::cerr << "Cannot open output file " << resultFileName << " Check the path.\nTraining logging is off.\n";
    }

    // выбор random examplesQty образцов для обучения
    string tgDir = dataFileName.substr(0, dataFileName.rfind('/')+1);
    unsigned int nTrained = 0;
    int nTotal = files.size()-1;
    srand (time(NULL));

    std::list<int> tested;
    // множество счетчиков уникальных признаков по категориям {слово: {(категория:счетчик)} }
    mapSmapSI counter;

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

        if (TrainOnFile(tgDir+fname, cat, counter) == 0)
        {
            ++nTrained;
            if (out.good())
                out << fname << " " << cat << " : training\n";
        }
    }
    if (out.is_open())
        out.close();
   // printMM (cnt);
    std::cout << "ndocs\n";
    printM (ndocs);

    std::cout << "vocabulary size = " << counter.size() <<"\n";

    // устанавливаем вероятности
    probability = MakeProb(counter, 1, 0.5);
    //printMM (probability);

    return 0;
}

mapsd BayesianClassifier::DocumentProbability (string const & fileName, mapsi & unknown /*список незнакомых признаков*/)
{
    mapsi m = ParseDocument(fileName);
    mapsd result;

    for (mapsi::const_iterator i = ndocs.begin(); i != ndocs.end(); ++i)
        result.insert(std::pair<string, double>(i->first, 1));

    for(mapsi::const_iterator i = m.begin(); i != m.end(); ++i)
    {
        mapSmapSD::const_iterator j = probability.find(i->first);
        if (j != probability.end())
        { // такой признак видели, классифицируем
            for (mapsi::const_iterator j = ndocs.begin(); j != ndocs.end(); ++j)
                result[j->first] *= probability[i->first][j->first];
        }
        else
        { // А ЕСЛИ НЕТ
            unknown.insert(*i);
        }
    }
    return result;
}

string BayesianClassifier::Classify (string const & fileName)
{
    mapsi newWords;
    mapsd pDocCat = DocumentProbability(fileName, newWords);
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

    string cat = max->first;

    // добавляем в общий probability новые слова и вероятности
    mapSmapSI tmpCnt = MakeSimpleCounter (newWords, cat);
    mapSmapSD tmpProb = MakeProb (tmpCnt, 1, 0.25);
    MergeProbs(tmpProb);

    std::cout << "vocabulary size = " << probability.size() << "\n";

    return cat;
}
