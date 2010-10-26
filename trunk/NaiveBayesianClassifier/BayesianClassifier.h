#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>

#ifndef _MyBayesianClassifier
#define _MyBayesianClassifier

class BayesianClassifier
{
    private:
        // пары (категория:счетчик документов)
        std::map<std::string, int> ndocs;

        // множество счетчиков уникальных признаков по категориям {слово: {(категория:счетчик)} }
        std::map<std::string, std::map<std::string, int> > counter;

        // множество вероятностей принадлежности уникальных признаков категориям {слово: {(категория:вероятность)} }
        std::map<std::string, std::map<std::string, double> > probability;

        // по counter заполняем probability (взвешенными вероятностями)
        // note: в probability есть вероятность по всем категориям! (в counter могут отсутствовать - там, где 0)
        void MakeProb (int weight, double aprioryProbability);

        // из отсортированного списка признаков делает map: (уникальный признак : кол-во вхождений в документ)
        std::map<std::string, int> MakeMap (std::list<std::string> const & list);

        // слияние набора map (принадлежит категории cat) с набором counter
        void MergeMaps (std::map<std::string, int> map, std::string const & cat);

        // обучение на файле
        int TrainOnFile (std::string const & fileName, std::string const & cat, unsigned int importance);

        // вероятности (категория|документ) по всем категориям для данного файла
        std::map<std::string, double> DocumentProbability (std::string const & fileName);

        // составление словаря по документу:
        // возвращает отсортированный список уникальных признаков (длины > importance)
        // TODO: нижнем регистре
        std::list<std::string> ParseDocument (std::string const &docFileName, unsigned int importance);

        // запрещаем конструктор копирования и опреатор присваивания
        BayesianClassifier (BayesianClassifier const & b);
        BayesianClassifier& operator= (BayesianClassifier const & b);

    public:
        BayesianClassifier () { }
        // принимает имя файла со списком документов по категориям и количество примеров на которых будет обучаться
        int Train (std::string dataFileName, unsigned int examplesQty, unsigned int importance, std::string resultFileName);

        // возвращает категорию документа
        std::string Classify (std::string const & fileName);


};

#endif
