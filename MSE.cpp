#include <cmath>
#include <algorithm>

#include "Regression.hpp"
#include "QualityChecker.hpp"
#include "DecisionTree.hpp"

MSE::MSE(WeightType weightType) : QualityChecker("MSE")
{
    _weightType = weightType;
}

double MSE::GetPointWeight(const Point& point)
{
     switch(_weightType)
     {
     case WeightType_Constant:
         return 1;
         break;

     case WeightType_Linear:
         return (point.y + 1);
         break;

     case WeightType_Square:
         return SQUARE(point.y + 1);
         break;
     }     

     return 1;
}

double MSE::GetQuality(const vector< pair<double, DecisionTree> >& series,
                         vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd)
{
    double error = 0;

    for(vector<Serp>::const_iterator it = itBegin; it != itEnd; ++it)	
        for(int i = 0; i < (int)it->size(); ++i)
        {
            double value = it->at(i).target;
            double weight = GetPointWeight(it->at(i));

            error += weight * SQUARE(it->at(i).y - value);
        }

	return error;
}

double MSE::GetQuality(ConstSerpIterator begin, ConstSerpIterator end)
{
    double error = 0;

    for(vector<Serp>::const_iterator it = begin; it != end; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            double value = it->at(i).target;
            double weight = GetPointWeight(it->at(i));

            error += weight * SQUARE(it->at(i).y - value);
        }

    return error;
}

double MSE::GetQuality(IRegression* regression, ConstSerpIterator begin, ConstSerpIterator end)
{
    double error = 0;

    for(vector<Serp>::const_iterator it = begin; it != end; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            double value = regression->Predict(it->at(i));
            double weight = GetPointWeight(it->at(i));

            error += weight * SQUARE(it->at(i).y - value);
        }

    return error;
}

void MSE::CalculateGradient(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd)
{
    for(vector<Serp>::iterator it = itBegin; it != itEnd; ++it)
        for(int i = 0; i < (int)it->size(); ++i)	
        {
            double weight = GetPointWeight(it->at(i));
            it->at(i).gradient = weight *(it->at(i).y - it->at(i).target);
        }
}

void MSE::CalculateTarget(SerpIterator begin, SerpIterator end)
{
    DataSetIterator it(begin, end);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
    {
        it.Current()->target = it.Current()->regression;
    }
}

void MSE::CalculateRegression(SerpIterator begin, SerpIterator end)
{
    DataSetIterator it(begin, end);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
    {
        it.Current()->regression = it.Current()->target;
    }
}
