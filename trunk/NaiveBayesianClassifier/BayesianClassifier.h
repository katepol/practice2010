#include <list>
#include <vector>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "PrintTemplates.cpp"
#include "Parser.h"
#include <libs/libstemmer.h>

#ifndef _MyBayesianClassifier
#define _MyBayesianClassifier

using std::string;
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

class BayesianClassifier
{
    private:

        sb_stemmer * stemmer_;

        // min word length to proceed
        unsigned int importance_;

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
        int trainOnFile (string const & fileName, string const & cat, mapSmapSI & cnt, string const &language, string const &encoding);

        // вероятности (категория|документ) по всем категориям для данного файла
        mapsd documentProbability (string const & fileName, mapsi & unknown, string const &language, string const &encoding);

        void stemWord (string & s) const;

        // составление словаря по документу:
        // возвращает отсортированный список отстемленных уникальных признаков в нижнем регистре (длины > importance)
        mapsi parseDocument (string const &docFileName, string const &language, string const &encoding);

        // запрещаем конструктор копирования и опреатор присваивания
        BayesianClassifier (BayesianClassifier const & b);
        BayesianClassifier& operator= (BayesianClassifier const & b);

    public:
        BayesianClassifier (unsigned int i) : importance_(i) {}

        // принимает имя файла со списком документов по категориям и количество примеров на которых будет обучаться
        int train (string dataFileName, unsigned int examplesQty, string resultFileName, string const &language, string const &encoding);

        // возвращает категорию документа
        string classify (string const & fileName, string const &language, string const &encoding);
};

#endif
