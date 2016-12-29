#ifndef _MULTICLASS_CLASSIFIER_HPP_
#define _MULTICLASS_CLASSIFIER_HPP_

#include "General.hpp"
#include "DecisionTree.hpp"

class TaskData;

class MulticlassClassifier
{
public:
    void Learn(const TaskData& taskData, const MetaParameters& params,
                SerpIterator learnBegin, SerpIterator learnEnd,
                SerpIterator examBegin, SerpIterator examEnd,
                bool stopAtNoisePart,
                TestResults *results);
    int GetClass(const Point& point) const;
    double GetCondidence(const Point& point, int classNumber) const;
    void GetFeaturesStat(vector<int> *stat) const;

private:
   vector<TreeSeries> _classifiers;
   double GetError(SerpIterator begin, SerpIterator end);
   
};

#endif /* MulticlassClassifier.hpp */