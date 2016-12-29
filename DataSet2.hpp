#ifndef _DATASET2_HPP
#define _DATASET2_HPP

#include "General.hpp"

class DataSetIterator;

class DataSet2
{
public:
    DataSet2(SerpIterator begin, SerpIterator end);
    int GetClassesCount() const;
    int GetFeaturesCount() const;
    DataSetIterator GetIterator();

private:
    int GetClassesCount(ConstSerpIterator begin, ConstSerpIterator end);

    int _classesCount;
    int _featuresCount;

    SerpIterator _begin;
    SerpIterator _end;
};

#endif /* _DATASET2_HPP */