#include "General.hpp"
#include "DataSet2.hpp"
#include "DataSetIterator.hpp"

#include <set>

using std::set;

int DataSet2::GetClassesCount() const
{
    return _classesCount;
}
int DataSet2::GetFeaturesCount() const
{
    return _featuresCount;
}

int DataSet2::GetClassesCount(ConstSerpIterator itBegin, ConstSerpIterator itEnd)
{
    int featuresCount = (int)itBegin->at(0).x.size();

    set<int> classes;

    for(ConstSerpIterator it = itBegin; it != itEnd; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
            classes.insert((int)it->at(i).y);

    return classes.size();
}

DataSet2::DataSet2(SerpIterator begin, SerpIterator end)
{
    _begin = begin;
    _end = end;

    _classesCount = GetClassesCount(begin, end);
    _featuresCount = begin->at(0).x.size();
}

DataSetIterator DataSet2::GetIterator()
{
    return DataSetIterator(_begin, _end);
}
