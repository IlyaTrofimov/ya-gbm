#ifndef _LOG_FUNCTIONAL_HPP_
#define _LOG_FUNCTIONAL_HPP_

#include "General.hpp"
#include "Regression.hpp"
#include "DataSetIterator.hpp"
#include "DecisionTree.hpp"

class QualityChecker
{
public:
    QualityChecker() : _name("") {}
    QualityChecker(const string& name) : _name(name) {}

    virtual double GetQuality(IRegression* regression, ConstSerpIterator begin, ConstSerpIterator end) {return 0;}
    virtual double GetQuality(ConstSerpIterator begin, ConstSerpIterator end) {return 0;}
    virtual void CalculateGradient(SerpIterator begin, SerpIterator end) = 0;
    virtual void SetLeafValues(SerpIterator begin, SerpIterator end, int classesCount, DecisionTree *tree) {}
    virtual void CalculateTarget(SerpIterator begin, SerpIterator end) = 0;
    virtual void CalculateRegression(SerpIterator begin, SerpIterator end) = 0;

    string GetName() {return _name; }

    virtual double GetQuality(const TreeSeries& series, ConstSerpIterator begin, ConstSerpIterator end) {return 0;}

private:
    string _name;
};

enum WeightType
{
    WeightType_Constant = 1,
    WeightType_Linear = 2,
    WeightType_Square = 3
};

class MSE : public QualityChecker
{
public:
    MSE(WeightType weightType);
    double GetQuality(const vector< pair<double, DecisionTree> >& series, vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd);
    double GetQuality(ConstSerpIterator begin, ConstSerpIterator end);
    double GetQuality(IRegression* regression, ConstSerpIterator begin, ConstSerpIterator end);
    double GetPointWeight(const Point& point);
    void CalculateGradient(SerpIterator itBegin, SerpIterator itEnd);
    void CalculateTarget(SerpIterator begin, SerpIterator end);
    void CalculateRegression(SerpIterator begin, SerpIterator end);

private:
    WeightType _weightType;
};

class LogFunctional : public QualityChecker
{
public:
    LogFunctional(bool useRankWeight);
    double GetQuality(const vector< pair<double, DecisionTree> >& series, vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd);
    void CalculateGradient(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd);
    void CalculateTarget(SerpIterator begin, SerpIterator end) {}
    void CalculateRegression(SerpIterator begin, SerpIterator end) {}

private:
    double GetSerpQuality(const vector< pair<double, DecisionTree> >& series, const vector<Point>& points);	
    double GetWeight(const Point& point1, const Point& point2);
    bool _useRankWeight;
};

class LogLikelihood : public QualityChecker
{
public:
    LogLikelihood() : QualityChecker("LogLikelihood") {}
    double GetQuality(IRegression* regression, ConstSerpIterator begin, ConstSerpIterator end);
    double GetQuality(ConstSerpIterator begin, ConstSerpIterator end);
    void CalculateGradient(SerpIterator begin, SerpIterator end);
    void SetLeafValues(SerpIterator begin, SerpIterator end, int classesCount, DecisionTree *tree);
    void CalculateTarget(SerpIterator begin, SerpIterator end);
    void CalculateRegression(SerpIterator begin, SerpIterator end);
};

#endif /* LogFunctional.hpp */