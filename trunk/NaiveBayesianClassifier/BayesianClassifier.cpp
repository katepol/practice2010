#include "BayesianClassifier.h"

BayesianClassifier * BayesianClassifier::instance_ = new BayesianClassifier();

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
    {// TODO: why not simply i->first, i->second ?
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

int BayesianClassifier::regexp (string & stringToCheck, char const * pattern) const
{     
	int num = 0;           
	regex_t rgT;
	regmatch_t match;
	regcomp(&rgT, pattern, REG_EXTENDED);
	while (regexec(&rgT, stringToCheck.c_str(), 1, &match, 0) == 0) 
	{
		int begin = (int)match.rm_so;
		int end = (int)match.rm_eo;
		stringToCheck = stringToCheck.substr(0, begin) +  stringToCheck.substr(end);
		++num;
	}
	regfree(&rgT);
	return num;
}

mapsi BayesianClassifier::findSmiles (string & s, mapsi & smileCounter) const
{// TODO: check
	string re[] = { ":\\)+", ":-\\)+", ";\\)+", ";-\\)+", ":\\(+", ":-\\(+", ":-\\*+" }; 	
	string smiles[] = { ":)", ":-)", ";)", ";-)", ":(", ":-(", ":-*" };
	int n = 7;
	for (int i=0; i != 7; ++i)
	{
		int k = regexp(s, smiles[i]);
		if (k > 0)
		{
			mapsi::iterator it = smileCounter.find(smiles[i]);
			if (it != map::end)
			{
				it->second += k;
			}
			else
				smileCounter.insert(smiles[i], k);
		}
	}
}
	
void BayesianClassifier::stemWord (string & s, mapsi & smileCounter) const
{
	// find and remove smiles
	smiles = findSmiles(s, smileCounter);
	
    //remowe punctuation from the end & from the beginning
    string p ( "!?:;.,\"()"  );
    int index = s.length();
    for (int i = s.length()-1; i!=0; --i)
    {
        if (p.find_first_of(s[i]) >= p.length())
        {
            index = i + 1;
            break;
        }
    }
    s = s.substr(0, index);
    index = 0;
    for (int i = 0; i!=s.length(); ++i)
    {
        if (p.find_first_of(s[i]) >= p.length())
        {
            index = i;
            break;
        }
    }
    s = s.substr(index);

    // to lower case
    // TODO: russian!
    // TODO: define language
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

mapsi BayesianClassifier::parseDocument (string const &docFileName, string const &language, string const &encoding, unsigned int importance)
{
    // NOTE: забиваем, если конвертируется с ошибками
    convertToXml(docFileName.c_str(), (docFileName+".xml").c_str());

    string fName(docFileName+"_p.xml");

    if (Parser::parseFile((docFileName+".xml").c_str(), (docFileName+"_p.xml").c_str()) > 0) //parsing failed
    {
        fName = docFileName+".xml";
    }

    ifstream doc(fName.c_str(), ifstream::in);

    if (!doc.is_open())
    {
        cerr << "Cannot open document " << fName << " Check the path.\nParsing aborted.\n";
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
    mapsi smiles;
    strig previousWord("");
    while (!doc.eof())
    {
      string s;
      doc >> s;
      if (s.length() > importance)
      {
          stemWord(s, smiles);
          word.push_back(s);
          
          word.push_back(previousWord+" "+s);
          previousWord = s;
      }
    }
    if (stemmer_ != 0) sb_stemmer_delete(stemmer_);
    doc.close();

    // delete *.xml and *_p.xml
    remove((docFileName+".xml").c_str());
    remove((docFileName+"_p.xml").c_str());

    word.sort();
    mapsi m1 = makePreCounter(word);
    // merge smiles and m1
    // TODO: check
    for (mapsi::const_iterator it = smiles.begin(); it != smiles.end(); ++it)
    	m1.insert (it->first, it->second);
    
    
	return m1;
}

int BayesianClassifier::trainOnFile (string const & fileName, string const & cat, mapSmapSI & cnt, string const &language, string const &encoding, unsigned int importance)
{
    mapsi m = parseDocument(fileName, language, encoding, importance);

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

int BayesianClassifier::train (string dataFileName, unsigned int examplesQty, unsigned int importance, string resultFileName, string const &language, string const &encoding)
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

        if (instance_->trainOnFile(tgDir+fname, cat, counter, language, encoding, importance) == 0)
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
    instance_->probability_ = instance_->makeProb(counter, 1, 0.5);
    //printMM (probability);

    return 0;
}

mapsd BayesianClassifier::documentProbability (string const & fileName, mapsi & unknown /*список незнакомых признаков*/, string const &language, string const &encoding, unsigned int importance)
{
    mapsi m = parseDocument(fileName, language, encoding, importance);
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



string BayesianClassifier::classify (string const & fileName, string const &language, unsigned int importance, string const &encoding)
{
    if (fileName=="/home/kate/APTU/Practice/data2/22312l")
        {
            cout << "TEEEEEEEEEEEST.\n";
            ofstream doc("/home/kate/APTU/Practice/data2/voc.txt", ofstream::trunc);
            if (!doc.is_open())
            {
                cout << "Cannot open file for writing data. Abort.\n";
                return 0;
            }
            printMM(instance_->probability_, doc);
            doc.close();
        }

    mapsi newWords;
    mapsd pDocCat = instance_->documentProbability(fileName, newWords, language, encoding, importance);
    // общее число документов
    int total = 0;
    for (mapsi::const_iterator i = instance_->ndocs_.begin(); i != instance_->ndocs_.end(); ++i)
        total += i->second;

    mapsd pCatDoc;
    for (mapsi::const_iterator i = instance_->ndocs_.begin(); i != instance_->ndocs_.end(); ++i)
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
    mapSmapSI tmpCnt = instance_->makeSimpleCounter (newWords, cat);
    mapSmapSD tmpProb = instance_->makeProb (tmpCnt, 1, 0.25);
    instance_->mergeProbs(tmpProb);

    //cout << "vocabulary size = " << instance_->probability_.size() << "\n";
    //cout << "classify " << fileName << "\n";

    return cat;
}
