#include "General.hpp"
#include "DataSetIterator.hpp"

template<class TSerpIterator, class TPoint>
DataSetIteratorBase<TSerpIterator, TPoint>::DataSetIteratorBase(vector<Serp> *dataSet)
{
    _begin = dataSet->begin();
    _end = dataSet->end();
}

template<class TSerpIterator, class TPoint>
DataSetIteratorBase<TSerpIterator, TPoint>::DataSetIteratorBase(TSerpIterator begin, TSerpIterator end)
{
    _begin = begin;
    _end = end;
}

template<class TSerpIterator, class TPoint>
int DataSetIteratorBase<TSerpIterator, TPoint>::GetGroupsCount() const
{
    return _end - _begin;
}

template<class TSerpIterator, class TPoint>
void DataSetIteratorBase<TSerpIterator, TPoint>::MoveFirst()
{
    _serpIterator = _begin;
    _position = 0;
    _index = 0;
}

template<class TSerpIterator, class TPoint>
void DataSetIteratorBase<TSerpIterator, TPoint>::MoveNext()
{
    int steps = 0;

    while(_serpIterator != _end)
    {
        while(_position < (int)_serpIterator->size())
        {
            if(steps == 1)
                return;

            ++_position;
            ++steps;
            ++_index;
        }

        ++_serpIterator;
        _position = 0;
    }
}

template<class TSerpIterator, class TPoint>
bool DataSetIteratorBase<TSerpIterator, TPoint>::IsDone() const
{
    return (_serpIterator == _end);
}

template<class TSerpIterator, class TPoint>
TPoint DataSetIteratorBase<TSerpIterator, TPoint>::Current()
{
    return &(_serpIterator->at(_position));
}

template<class TSerpIterator, class TPoint>
int DataSetIteratorBase<TSerpIterator, TPoint>::Index() const
{
    return _index;
}




DataSetIterator::DataSetIterator(vector<Serp> *dataSet)
{
    _begin = dataSet->begin();
    _end = dataSet->end();
}

DataSetIterator::DataSetIterator(SerpIterator begin, SerpIterator end)
{
    _begin = begin;
    _end = end;
}

int DataSetIterator::GetGroupsCount() const
{
    return _end - _begin;
}

void DataSetIterator::MoveFirst()
{
    _serpIterator = _begin;
    _position = 0;
    _index = 0;
}

void DataSetIterator::MoveNext()
{
    int steps = 0;

    while(_serpIterator != _end)
    {
        while(_position < (int)_serpIterator->size())
        {
            if(steps == 1)
                return;

            ++_position;
            ++steps;
            ++_index;
        }

        ++_serpIterator;
        _position = 0;
    }
}

bool DataSetIterator::IsDone() const
{
    return (_serpIterator == _end);
}

Point* DataSetIterator::Current()
{
    return &(_serpIterator->at(_position));
}

int DataSetIterator::Index() const
{
    return _index;
}