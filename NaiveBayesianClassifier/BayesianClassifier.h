#include <list>
#include <vector>
#include <map>
#include <iostream>

#ifndef _MyBayesianClassifier
#define _MyBayesianClassifier

class BayesianClassifier
{
    private:
        // min word length to proceed
        unsigned int importance;

        // пары (категория:счетчик документов)
        // note: заполнен после Train
        std::map<std::string, int> ndocs;

        // множество счетчиков уникальных признаков по категориям {слово: {(категория:счетчик)} }
        // std::map<std::string, std::map<std::string, int> > counter;

        // множество вероятностей принадлежности уникальных признаков категориям {слово: {(категория:вероятность)} }
        std::map<std::string, std::map<std::string, double> > probability;

        // по cnt заполняем результат взвешенными вероятностями
        std::map<std::string, std::map<std::string, double> >
                MakeProb (std::map<std::string, std::map<std::string, int> > & cnt, int weight, double aprioryProbability);

        // слияние набора prob с набором probability
        void
                MergeProbs (std::map<std::string, std::map<std::string, double> > const & prob);


        // из отсортированного списка признаков делает map: (уникальный признак : кол-во вхождений в документ)
        std::map<std::string, int>
                MakePreCounter (std::list<std::string> const & list);

        std::map<std::string, std::map<std::string, int> >
                MakeSimpleCounter (std::map<std::string, int> const & preCounter, std::string const & cat);

        // слияние набора map (принадлежит категории cat) с набором cnt
        void
                MergeCounters (std::map<std::string, std::map<std::string, int> > & cnt, std::map<std::string, int> const & map, std::string const & cat);

        // обучение на файле
        int
                TrainOnFile (std::string const & fileName, std::string const & cat, std::map<std::string, std::map<std::string, int> > & cnt);

        // вероятности (категория|документ) по всем категориям для данного файла
        std::map<std::string, double>
                DocumentProbability (std::string const & fileName, std::map<std::string, int> & unknown);

        // составление словаря по документу:
        // возвращает отсортированный список уникальных признаков (длины > importance)
        // TODO: нижнем регистре
        std::map<std::string, int>
                ParseDocument (std::string const &docFileName);

        // запрещаем конструктор копирования и опреатор присваивания
        BayesianClassifier (BayesianClassifier const & b);
        BayesianClassifier& operator= (BayesianClassifier const & b);

    public:
        BayesianClassifier (unsigned int i): importance(i) {}

        // принимает имя файла со списком документов по категориям и количество примеров на которых будет обучаться
        int
                Train (std::string dataFileName, unsigned int examplesQty, std::string resultFileName);

        // возвращает категорию документа
        std::string
                Classify (std::string const & fileName);
};

#endif
