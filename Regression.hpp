#ifndef _REGRESSION_HPP_
#define _REGRESSION_HPP_

#include <string>

#include "General.hpp"
#include "TaskData.hpp"

using std::string;

class DecisionTree;
class DecisionTreeNode;

class IRegression
{
public:
    IRegression() {}
    IRegression(string name) : _name(name) {}

    void virtual Learn(const MetaParameters& params,
             SerpIterator itTrainBegin, SerpIterator itTrainEnd,
             SerpIterator itTestBegin, SerpIterator itTestEnd,
             TestResults *results)
        { };
    double virtual Predict(const Point& point) const = 0;
    string GetName() { return _name; }
    string Get_C_code() { return ""; } 

private:
    string _name;
};

class Regression : IRegression
{
public:
    void Learn(const MetaParameters& params,
                 SerpIterator itTrainBegin, SerpIterator itTrainEnd,
                 SerpIterator itTestBegin, SerpIterator itTestEnd,
                 TestResults *results);
    void GetFeaturesImportance(vector<double> *importance) const;
    double Predict(const Point& point) const;
    string Get_C_code() const;

private:
    string Get_C_code(double shinkage, DecisionTreeNode *node, int level) const;
    string Get_C_code(const pair<double, DecisionTree>& term) const;

    TaskData _taskData;
    TreeSeries _series;
};

class AvgRegression : IRegression
{
public:
    AvgRegression(string name) : IRegression(name) {}
    void Learn(const MetaParameters& params,
                 SerpIterator itTrainBegin, SerpIterator itTrainEnd,
                 SerpIterator itTestBegin, SerpIterator itTestEnd,
                 TestResults *results)
    {
        _value = GetAvgResponce(itTrainBegin, itTrainEnd);
    }

    double Predict(const Point& point) const
    {
        return _value;
    }

private:
    double _value;
};

class ConstantRegression : IRegression
{
public:
    ConstantRegression(string name, double value) : _value(value), IRegression(name) {}

    double Predict(const Point& point) const
    {
        return _value;
    }

private:
    double _value;
};


class OneFeatureRegression : IRegression
{
public:
    OneFeatureRegression(string name, int featureId, double coefficient) : _featureId(featureId), _coefficient(coefficient), IRegression(name)
    {}

    double Predict(const Point& point) const
    {
        return point.x[_featureId] * _coefficient;
    }

private:
    int _featureId;
    double _coefficient;
};

class LinearRegression : IRegression
{
public:
    LinearRegression(string name, int firstFeatureId, double constant, vector<double> coefficients) 
        : _firstFeatureId(firstFeatureId), _constant(constant), _coefficients(coefficients), IRegression(name)
    {}

    double Predict(const Point& point) const
    {
        double product = 0;

        for(int i = 0; i < (int)_coefficients.size(); ++i)
            product += point.x[i + _firstFeatureId] * _coefficients[i];

        return _constant + product;
    }

private:
    int _firstFeatureId;
    vector<double> _coefficients;
    double _constant;
};

#define FIRST_ARINA_FEATURE 2       // Первый признак Арины
#define ARINA_FEATURES_COUNT 13     // Количество признаков Арины без константы

#define PSHOWS_FEATURE 49
#define QTAIL_WORD_COUNT_FEATURE 50
#define SEARCH_NOT_CLICKS_FEATURE 58
#define WEEKDAY_FEATURE 65
#define PCTR_PAGE_NO_FEATURE 66
#define PAGE_NO_FEATURE 67

class Arina : IRegression
{
public:
    Arina();
    double Predict(const Point& point) const;
    ~Arina();

private:
    LinearRegression* GetArinaPageNo1();
    LinearRegression* GetArinaPageNo2();

    LinearRegression* _PageNo1Regression;
    LinearRegression* _PageNo2Regression;
};

class Karina : IRegression
{
public:
    Karina() : IRegression("KARINA") { }
    double Predict(const Point& point) const;

private:
    double ScalarProduct(const vector<double> array1, double array2[]) const;
    vector<double> GetKarinaFeatures(const Point& point) const;

    static double _workdays1[];
    static double _workdays2[];
    static double _weekend1[];
    static double _weekend2[];
};

#endif /* _REGRESSION_HPP_ */