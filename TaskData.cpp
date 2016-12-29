#include "TaskData.hpp"
#include "DataSetIterator.hpp"

#include <algorithm>
#include <cmath>
#include <set>

using std::set;

const vector<double>* TaskData::GetThresholds(int featureId) const
{
    return &_thresholds[featureId];
}

const vector< vector<double> >& TaskData::GetAllThresholds() const
{
    return _thresholds;
}

bool Equals(double x, double y, double precision)
{
    return fabs(x - y) < precision;
}

int TaskData::GetFeaturesCount() const
{
    return _thresholds.size();
}

int TaskData::GetClassesCount() const
{
    return _classesCount;
}

int TaskData::GetClassesCount(ConstSerpIterator itBegin, ConstSerpIterator itEnd)
{
    set<int> classes;

    for(ConstSerpIterator it = itBegin; it != itEnd; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
            classes.insert((int)it->at(i).y);

    return classes.size();
}

void FillThresholdsData(const vector< vector<double> >& thresholds, SerpIterator begin, SerpIterator end)
{
    DataSetIterator it(begin, end);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
    {
        Point* point = it.Current();
        int featuresCount = thresholds.size();
        point->thresholds.resize(featuresCount, 0);

        for(int featureId = 0; featureId < featuresCount; ++featureId)
        {
            double feature = point->x[featureId];
            double value = point->y;

            for(int j = 0; j < (int)thresholds[featureId].size(); ++j)
            {           
                if(j == thresholds[featureId].size() - 1)
                {
                    point->thresholds[featureId] = j;
                    break;
                }
                else if(thresholds[featureId][j] <= feature && feature < thresholds[featureId][j + 1])
                {
                    point->thresholds[featureId] = j;
                    break;
                }
            }
        }
    }
}

void TaskData::Calculate(ConstSerpIterator itBegin, ConstSerpIterator itEnd, int thresholdsMax)
{
    _classesCount = GetClassesCount(itBegin, itEnd);
    _thresholds.clear();

    // FIXME
    int featuresCount = (int)itBegin->at(0).x.size();

    for(int featureId = 0; featureId < featuresCount; ++featureId)
    {
        vector<double> allValues;

        for(ConstSerpIterator it = itBegin; it != itEnd; ++it)
            for(int i = 0; i < (int)it->size(); ++i)
                allValues.push_back(it->at(i).x[featureId]);

        std::sort(allValues.begin(), allValues.end());

        vector<double> new_thresholds;
        new_thresholds.push_back(allValues[0]);

        int step = allValues.size() / thresholdsMax + 1;

        for(int i = 0; i < (int)allValues.size(); i += step)
        {
            double prevFeatureValue = new_thresholds.back();
            double curFeatureValue = allValues[i];

            if(!Equals(prevFeatureValue, curFeatureValue, 0.001))
            {
                new_thresholds.push_back(curFeatureValue);
            }
        }

        //if(new_thresholds.back() != allValues.back())
        //    new_thresholds.push_back(allValues.back());

        _thresholds.push_back(new_thresholds);
    }
}
