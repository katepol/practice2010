#ifndef PRINTTEMPLATES_H
#define PRINTTEMPLATES_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <ostream>

template <class T>
        void printL (std::list<T> const & list);

template <class T>
        void printV (std::vector<T> const & vector);

template <class K, class V>
        void printM (std::map<K, V> const & map);

template <class K1, class K2, class V>
        void printMM (std::map<K1, std::map<K2, V> > const & map);

template <class K1, class K2, class V>
        void printMM (std::map<K1, std::map<K2, V> > const & map, std::ofstream & out);

#endif // PRINTTEMPLATES_H
