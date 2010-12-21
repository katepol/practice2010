#include <list>
#include <vector>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <regex.h>
#include "PrintTemplates.h"
#include "Parser.h"
#include "HtmlToXml.h"
#include <externallibs/include/libstemmer.h>

#ifndef _MyBayesianClassifier
#define _MyBayesianClassifier

using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::cerr;
using std::cout;

typedef std::list<std::string>              lststr;
typedef std::vector<std::string>            vecstr;
typedef std::map<std::string, int>          mapsi;
typedef std::map<std::string, double>       mapsd;
typedef std::map<std::string, std::map<std::string, int> >     mapSmapSI;
typedef std::map<std::string, std::map<std::string, double> >  mapSmapSD;

// singletone
class BayesianClassifier
{
    private:

        static BayesianClassifier * instance_;

        sb_stemmer * stemmer_ru_;
        sb_stemmer * stemmer_en_;

        //regex_t * smileRegex;
        //string * smilePattern;

        // пары (категория:счетчик документов)
        // note: заполнен после Train
        mapsi ndocs_;

        // множество вероятностей принадлежности уникальных признаков категориям {слово: {(категория:вероятность)} }
        mapSmapSD probability_;

        // по cnt заполняем результат взвешенными вероятностями
        mapSmapSD makeProb (mapSmapSI & cnt, int weight, double aprioryProbability);

        // слияние набора prob с набором probability
        void mergeProbs (mapSmapSD const & prob);

        // из отсортированного списка признаков делает map: (уникальный признак : кол-во вхождений в документ)
        mapsi makePreCounter (lststr const & list) const;

        mapSmapSI makeSimpleCounter (mapsi const & preCounter, string const & cat) const;

        // слияние набора map (принадлежит категории cat) с набором cnt
        void mergeCounters (mapSmapSI & cnt, mapsi const & map, string const & cat) const;

        // обучение на файле
        int trainOnFile (string const & fileName, string const & cat, mapSmapSI & cnt, unsigned int importance);

        // вероятности (категория|документ) по всем категориям для данного файла
        mapsd documentProbability (string const & fileName, mapsi & unknown, unsigned int importance);

        // find & remove smiles
        void findSmiles (string & s, mapsi & smileCounter) const;

        //remowe punctuation from the end & from the beginning
        void removePunctuation (string & s) const;

        // returns language
        char const * toLowerCase (string & s) const;

        void stem (string & s, char const * language) const;

        // stemming, includes collecting smiles
        void processWord (string & s, mapsi & smileCounter) const;
        
        // подсчет, сколько раз встречается pattern в строке stringToCheck и удаление
        int regexp (string & stringToCheck, string const & pattern) const;

        // составление словаря по документу:
        // возвращает отсортированный список отстемленных уникальных признаков в нижнем регистре (длины > importance)
        mapsi parseDocument (string const &docFileName, unsigned int importance);

        // запрещаем конструктор копирования и опреатор присваивания
        BayesianClassifier (BayesianClassifier const & b);
        BayesianClassifier& operator= (BayesianClassifier const & b);

        BayesianClassifier();

        ~BayesianClassifier();

    public:
        static BayesianClassifier * getInstance ()
        {
            if (instance_ == 0)
                instance_ = new BayesianClassifier();
            return instance_;
        }

        // принимает имя файла со списком документов по категориям и количество примеров на которых будет обучаться
        int train (string dataFileName, unsigned int examplesQty, unsigned int importance, string const & resultFileName);

        // возвращает категорию документа
        string classify (string const & fileName, unsigned int importance);

        mapsi getNdocs () { return ndocs_; }
};

//BayesianClassifier * BayesianClassifier::instance_ ;

#endif
