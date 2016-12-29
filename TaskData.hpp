#ifndef _TASK_DATA_HPP
#define _TASK_DATA_HPP

#include "General.hpp"

class TaskData
{
public:
    void Calculate(ConstSerpIterator itBegin, ConstSerpIterator itEnd, int thresholdsMax);
    const vector<double>* GetThresholds(int featureIndex) const;
    const vector< vector<double> >& GetAllThresholds() const;
    int GetFeaturesCount() const;
    int GetClassesCount() const;

private:
    int GetClassesCount(ConstSerpIterator itBegin, ConstSerpIterator itEnd);

    vector< vector<double> > _thresholds;
    int _classesCount;
};

void FillThresholdsData(const vector< vector<double> >& thresholds, SerpIterator begin, SerpIterator end);

#endif /* TaskData.hpp */
