#ifndef _CPC_OPTIMIZER_HPP_
#define _CPC_OPTIMIZER_HPP_

#include "General.hpp"
#include "QualityChecker.hpp"

class DecisionTree;

class CPC_Optimizer : public QualityChecker
{
public:    
	double GetQuality(const vector< pair<double, DecisionTree> >& series, vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd);
	void CalculateGradient(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd);
    void CalculateTarget(SerpIterator begin, SerpIterator end) {}
    void CalculateRegression(SerpIterator begin, SerpIterator end) {}
private:
    WeightType _weightType;
};

double GetMoneyFraction(ConstSerpIterator begin, ConstSerpIterator end);
double GetHits(ConstSerpIterator itBegin, ConstSerpIterator itEnd);
void CalculateHitsGradient(SerpIterator itBegin, SerpIterator itEnd);

#endif /* CPC_Optimizer.hpp */