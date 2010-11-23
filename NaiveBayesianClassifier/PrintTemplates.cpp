#include <string>
#include <list>
#include <vector>
#include <map>

#include <iostream>

template <class T>
void printL (std::list<T> const & list)
{
    for (typename std::list<T>::const_iterator i = list.begin(); i != list.end(); ++i)
        std::cout << *i << " | ";
    std::cout << std::endl;
}

template <class T>
void printV (std::vector<T> const & vector)
{
    for (typename std::vector<T>::const_iterator i = vector.begin(); i != vector.end(); ++i)
        std::cout << *i << " | ";
    std::cout << std::endl;
}

template <class K, class V>
void printM (std::map<K, V> const & map)
{
    for (typename std::map<K, V>::const_iterator i = map.begin(); i != map.end(); ++i)
        std::cout << (*i).first << ":" << (*i).second << "\n";
    std::cout << std::endl;
}

template <class K1, class K2, class V>
void printMM (std::map<K1, std::map<K2, V> > const & map)
{
    for (typename std::map<K1, std::map<K2, V> >::const_iterator i = map.begin(); i != map.end(); ++i)
    {
        std::cout << i->first << ": { ";
        for (typename std::map<K2, V>::const_iterator j = (i->second).begin(); j != (i->second).end(); ++j)
            std::cout << "(" << j->first << ":" << j->second << "); ";
        std::cout << "}" << std::endl;
    }
    std::cout << std::endl;
}