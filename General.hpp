#ifndef _GENERAL_HPP
#define _GENERAL_HPP

#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <ctime>

class IRegression;
class QualityChecker;

using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::endl;
using std::pair;
using std::cout;
using std::make_pair;

struct Point
{
    double y;
    double regression;
    double target;
    double gradient;
    vector<double> x;
    vector<bool> present;
    vector<char> thresholds;
    bool disabled;
    bool totalDisabled;
};

typedef vector<Point> Serp;

#define SQUARE(X) ((X) * (X))
#define CUBE(X) ((X) * (X) * (X))
#define ARRAY_LEN(X) (sizeof(X) / sizeof(X[0]))
#define TO_MINUS_PLUS(y) (2 * (y) - 1)
#define EXP_LIMITED(X) ((X) < 9.21 ? exp(X) : 10000)
#define LOG_LIMITED(X) ((X) < 0.0001 ? -9.21 : log(X))

enum EnumTreeBuilder 
{
    TreeBuilder_MaxHeight,
    TreeBuilder_FirstBest,
    TreeBuilder_Matrix
};

struct MetaParameters
{
    int TreeHeight;
    int TreesTotal;
    int BoostingDelay;
    double GradientShrinkage;
    double BaggingPart;
    double RSMPart;
    int ThresholdsCount;
    int Features;
    double NoiseDeletePart;
    bool StopByTestError;
    double TestPart;
    int FilterErrors;
    double WeightTrimming;
    int MinLeafSize;
    int LeafsCountMax;
    EnumTreeBuilder TreeBuilder;
    IRegression *InitialRegression;
    vector<IRegression*> BaselineRegression;
    vector<QualityChecker*> Metrics;
    vector<bool> AllowedFeatures;
};

struct ThresholdStat
{
    double Threshold;
    int PointsCount;
    double AvgResponce;
    double AvgResponceDelta;
    double Dispersion;
};

struct TestResults
{
    double LearnError;
    double TestError;
    double LearnErrorRelStd;
    double TestErrorRelStd;
    double LearnQuality;
    double TestQuality;

    int Length;
    double Time;
    vector<double> Importance;
    vector< vector<ThresholdStat> > Stat;

    vector<double> Metrics;
    vector< vector<double> > BaseLineMetrics;
};

struct MetaResults
{
    int SeriesLength;
    double Error;
    double Quality;
    double Time;
};

class DecisionTree;

typedef vector< pair<double, DecisionTree> > TreeSeries;
typedef vector<Serp>::iterator SerpIterator;
typedef vector<Serp>::const_iterator ConstSerpIterator;
typedef vector<Serp> DataSet;
typedef double (*MetricsFunction)(ConstSerpIterator begin, ConstSerpIterator end);
typedef double (*MetricsFunction2)(SerpIterator begin, SerpIterator end);
typedef bool (*DataSetFilter)(const Point* point);

struct DataSetParams
{
    string LearnFile;
    string TaskFile;
    string MarkedTaskFile;
    string ParamsFile;
    string ResultsFile;
    QualityChecker *checker;
    MetricsFunction Metrics;
    int LearnSize;
    int ExamSize;
    double SerpShrinkage;
};

struct Log
{
    string ResultsFile;
    string FeauresFile;
    int ExperimentId;
    vector<string> FeaturesName;
};

bool PointComparer(const Point& point1, const Point& point2);
bool PairSecondCompare(const pair<double, double>& pair1, const pair<double, double>& pair2);
bool PairFirstCompare(const pair<double, double>& pair1, const pair<double, double>& pair2);
bool PairSecondCompare2(const pair<Point*, double>& pair1, const pair<Point*, double>& pair2);
double GetAvg(const vector<Point*>& points);
double GetAvgResponce(DataSet* points);
double GetAvgResponce(SerpIterator begin, SerpIterator end);
string ToString(int value);
string ToString(unsigned int value);
string ToString(double value);
string PadRight(string s, int width);

#endif  /* _GENERAL_HPP */
 
