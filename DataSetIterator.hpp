#ifndef _DATASET_ITERATOR_HPP
#define _DATASET_ITERATOR_HPP

#include "General.hpp"

#include <vector>

template<class TSerpIterator, class TPoint>
class DataSetIteratorBase
{
public:
    DataSetIteratorBase(vector<Serp> *dataSet);
    DataSetIteratorBase(TSerpIterator begin, TSerpIterator end);
    int GetGroupsCount() const;

    void MoveFirst();
    void MoveNext();
    bool IsDone() const;

    TPoint Current();
    int Index() const;

private:
    TSerpIterator _begin;
    TSerpIterator _end;
    TSerpIterator _serpIterator;
    int _position;
    int _index;
};

class DataSetIterator
{
public:
    DataSetIterator(vector<Serp> *dataSet);
    DataSetIterator(SerpIterator begin, SerpIterator end);
    int GetGroupsCount() const;

    void MoveFirst();
    void MoveNext();
    bool IsDone() const;

    Point* Current();
    int Index() const;

private:
    SerpIterator _begin;
    SerpIterator _end;
    SerpIterator _serpIterator;
    int _position;
    int _index;
};

//typedef DataSetIterator DataSetIterator;
//typedef DataSetIteratorBase<ConstSerpIterator, const Point*> ConstDataSetIterator;

#endif /* _DATASET_ITERATOR_HPP */