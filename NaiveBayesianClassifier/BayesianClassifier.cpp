#include "BayesianClassifier.h"

mapsi BayesianClassifier::makePreCounter (lststr const & list) const
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

mapSmapSI BayesianClassifier::makeSimpleCounter (mapsi const & preCounter, string const & cat) const
{
    mapSmapSI cnt;
    for (mapsi::const_iterator i = preCounter.begin(); i != preCounter.end(); ++i)
    {
        cnt.insert(std::pair<string, mapsi>(i->first, mapsi())); // вставили признак
        for (mapsi::const_iterator j = ndocs_.begin(); j != ndocs_.end(); ++j)
        {
            int k = (j->first == cat) ? i->second : 0;
            cnt[i->first].insert(std::pair<string, int>(j->first, k));
        }
    }
    return cnt;
}

void BayesianClassifier::mergeCounters (mapSmapSI & cnt, mapsi const & map, string const & cat) const
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

void BayesianClassifier::mergeProbs (mapSmapSD const & prob)
{
    for (mapSmapSD::const_iterator i = prob.begin(); i != prob.end(); ++i)
    {
        probability_.insert(std::pair<string, mapsd>(i->first, i->second));
    }
}

mapSmapSD BayesianClassifier::makeProb(mapSmapSI & cnt, int weight, double aprioryProbability)
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

        for (mapsi::const_iterator j = ndocs_.begin(); j != ndocs_.end(); ++j)
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

void BayesianClassifier::stemWord (string & s) const
{
    // TODO: remowe punctuation

    // to lower case
    for (unsigned int i=0; i != s.length(); ++i)
        if (isupper((unsigned char)s[i]))
            s[i] = (unsigned char)tolower(s[i]);

    //stemming
    char * stemmed = (char *)sb_stemmer_stem(stemmer_, (unsigned char *)s.c_str(), s.length());
    if (stemmed == 0)
    {
        cerr << "Stemming failed!\n";
    }
    else
    {
        s = stemmed;
    }
}

mapsi BayesianClassifier::parseDocument (string const &docFileName, string const &language, string const &encoding)
{
    string fName = parseFile(docFileName);
    ifstream doc(fName.c_str(), ifstream::in);
    //ifstream doc(docFileName.c_str(), ifstream::in);
    if (!doc.is_open())
    {
        cerr << "Cannot open document " << docFileName << " Check the path.\nParsing aborted.\n";
        return mapsi();
    }


    // cout << "Parsing document " << docFileName << std::endl;

    stemmer_ = sb_stemmer_new(language.c_str(),  encoding.c_str());
    if (stemmer_ == 0) {
        if (encoding == "")
            cerr << "language " << language << " not available for stemming\n";
        else
            cerr << "language " << language << " not available for stemming in encoding " << encoding << "\n";
    }

    lststr word;
    while (!doc.eof())
    {
      string s;
      doc >> s;
      if (s.length() > importance_)
      {
          stemWord(s);
          word.push_back(s);
      }
    }
    if (stemmer_ != 0) sb_stemmer_delete(stemmer_);
    doc.close();
    word.sort();
    return makePreCounter(word);
}

int BayesianClassifier::trainOnFile (string const & fileName, string const & cat, mapSmapSI & cnt, string const &language, string const &encoding)
{
    mapsi m = parseDocument(fileName, language, encoding);

    // cout << "  training on " << fileName << "...\n";
    mergeCounters(cnt, m, cat);

    mapsi::iterator i = ndocs_.find(cat);
    if (i != ndocs_.end())
    { // увеличиваем счетчик документов по категории
        ++ndocs_[cat];
    }
    else
    {
        ndocs_.insert (std::pair<string, int>(cat, 1));
    }
    return 0;
}

int BayesianClassifier::train (string dataFileName, unsigned int examplesQty, string resultFileName, string const &language, string const &encoding)
{
    ifstream data(dataFileName.c_str(), ifstream::in);
    if (!data.is_open())
    {
        cerr << "Cannot open data file " << dataFileName << " Check the path.\nTraining aborted.\n";
        return 1;
    }
    cout << "Training...\n";

    vecstr files;

    while (!data.eof())
    {
        char tmp[100];
        data.getline(tmp, 100);
        string line(tmp);
        files.push_back(line);
    }
    data.close();

    if (files.size() < examplesQty)
    {
        cerr << "Not enough data for training! Training aborted.\n";
        return 2;
    }

    ofstream out(resultFileName.c_str(), ofstream::trunc);
    if (!out.is_open())
    {
        cerr << "Cannot open output file " << resultFileName << " Check the path.\nTraining logging is off.\n";
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

        if (trainOnFile(tgDir+fname, cat, counter, language, encoding) == 0)
        {
            ++nTrained;
            if (out.good())
                out << fname << " " << cat << " : training\n";
        }
    }
    if (out.is_open())
        out.close();

    // printMM (cnt);
    // cout << "ndocs\n"; printM (ndocs_);
    cout << "vocabulary size = " << counter.size() <<"\n";

    // устанавливаем вероятности
    probability_ = makeProb(counter, 1, 0.5);
    //printMM (probability);

    return 0;
}

mapsd BayesianClassifier::documentProbability (string const & fileName, mapsi & unknown /*список незнакомых признаков*/, string const &language, string const &encoding)
{
    mapsi m = parseDocument(fileName, language, encoding);
    mapsd result;

    for (mapsi::const_iterator i = ndocs_.begin(); i != ndocs_.end(); ++i)
        result.insert(std::pair<string, double>(i->first, 1));

    for(mapsi::const_iterator i = m.begin(); i != m.end(); ++i)
    {
        mapSmapSD::const_iterator j = probability_.find(i->first);
        if (j != probability_.end())
        { // такой признак видели, классифицируем
            for (mapsi::const_iterator j = ndocs_.begin(); j != ndocs_.end(); ++j)
                result[j->first] *= probability_[i->first][j->first];
        }
        else
        { // А ЕСЛИ НЕТ
            unknown.insert(*i);
        }
    }
    return result;
}



string BayesianClassifier::classify (string const & fileName, string const &language, string const &encoding)
{
    if (fileName=="/home/kate/APTU/Practice/data2/10")
    {
        cout << "TEEEEEEEEEEEST.\n";
        ofstream doc("/home/kate/APTU/Practice/data2/voc.txt", ofstream::trunc);
        if (!doc.is_open())
        {
            cout << "Cannot open file for writing data. Abort.\n";
            return 0;
        }

        printMM(probability_, doc);

        doc.close();
    }

    mapsi newWords;
    mapsd pDocCat = documentProbability(fileName, newWords, language, encoding);
    // общее число документов
    int total = 0;
    for (mapsi::const_iterator i = ndocs_.begin(); i != ndocs_.end(); ++i)
        total += i->second;

    mapsd pCatDoc;
    for (mapsi::const_iterator i = ndocs_.begin(); i != ndocs_.end(); ++i)
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
    mapSmapSI tmpCnt = makeSimpleCounter (newWords, cat);
    mapSmapSD tmpProb = makeProb (tmpCnt, 1, 0.25);
    mergeProbs(tmpProb);

    cout << "vocabulary size = " << probability_.size() << "\n";

    return cat;
}
