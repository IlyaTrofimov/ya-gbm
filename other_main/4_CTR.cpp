#include "stdio.h"
#include <fstream>
#include <algorithm>
#include <numeric>
#include <limits>
#include <map>

#include "General.hpp"
#include "FilesIO.hpp"
#include "MulticlassClassifier.hpp"
#include "DataSetIterator.hpp"
#include "TaskData.hpp"
#include "AlgorithmTester.hpp"
#include "QualityChecker.hpp"
#include "Regression.hpp"

using std::map;

MetaParameters GetMetaParametersFixed();
double GetBaselineError(DataSet *dataSet);
double GetArinaError(SerpIterator begin, SerpIterator end);
double GetOlesyaError(SerpIterator begin, SerpIterator end);
double GetCOlesyaError(SerpIterator begin, SerpIterator end);
vector<string> GetFeaturesNames();
LinearRegression* GetArina();
vector<MetaParameters> GetMetaParameters_CTR();

void TestMetaParameters(const vector<MetaParameters>& paramsSet, DataSet* dataSet, Log *log);
MetaParameters DisableFeatures(const string& disabledFeatures, const MetaParameters& params);
void TestLearnCurve(const MetaParameters& params, DataSet* dataSet, Log *log);

bool ExactMatch(const Point* point)
{
    return (point->x[PAGE_NO_FEATURE] == 1 && point->x[QTAIL_WORD_COUNT_FEATURE] == 0);
}

bool NotExactMatch(const Point* point)
{
    return !ExactMatch(point);
}

bool LongStatistics(const Point* point)
{
    return (point->x[PSHOWS_FEATURE] > 1000);
}

bool ShortStatistics(const Point* point)
{
    return (point->x[PSHOWS_FEATURE] < 50);
}

const string g_WideFeatures = "58, 59, 64, 65";

int main444()
{
    string dataFileName = "C:\\CTR\\week";

    Log log;
    log.ExperimentId = 0;
    log.FeauresFile = dataFileName + ".features";
    log.ResultsFile = dataFileName + ".results";
    log.FeaturesName = GetFeaturesNames();

    DataSet dataSet;
    GetDataFactorLog(dataFileName, 1, 1000, ShortStatistics, &dataSet);

    printf("\n");
    printf("Points total = %d\n", dataSet.size());
    printf("CTR_avg = %f\n", GetAvgResponce(&dataSet));
    printf("\n");

    WriteLine("Запуск программы", &log);
    WriteLine("Файл " + dataFileName, &log);
    WriteLine("Количество точек " + ToString(dataSet.size()), &log);

    srand(123);
    std::random_shuffle(dataSet.begin(), dataSet.end());

    vector<MetaParameters> paramsSet;
    MetaParameters defaultParams = GetMetaParametersFixed();

    //defaultParams = DisableFeatures(g_WideFeatures, defaultParams);

    paramsSet.push_back(defaultParams);

    //defaultParams.AllowedFeatures[1] = true;

    //paramsSet.push_back(defaultParams);

    /*std::swap(defaultParams.Metrics[0], defaultParams.Metrics[1]);

    defaultParams.GradientShrinkage -= 0.005;
    paramsSet.push_back(defaultParams);
    defaultParams.GradientShrinkage -= 0.005;
    paramsSet.push_back(defaultParams);
    defaultParams.GradientShrinkage -= 0.005;
    paramsSet.push_back(defaultParams);*/

    //paramsSet.push_back(DisableFeatures("6, 9, 13, 18, 19, 20, 21, 22, 28, 35, 36, 39, 40, 42, 44, 46, 53, 62, 7, 15, 38, 10, 51", defaultParams));
    //paramsSet.push_back(DisableFeatures("58, 59", defaultParams));

    //paramsSet = GetMetaParameters_CTR();

    //DelStrategy(DisableFeatures("6, 9, 13, 18, 19, 20, 21, 22, 28, 35, 36, 39, 40, 42, 44, 46, 53, 62", defaultParams), 5, &dataSet, &log);

    //MetaParameters params = GetMetaParametersFixed();
    TestMetaParameters(paramsSet, &dataSet, &log);

    //TestLearnCurve(params, &dataSet, &log);

    //params.ThresholdsCount = 90;
    //TestLearnCurve(params, &dataSet, &log);

    return 0;
}

void TestMetaParameters(const vector<MetaParameters>& paramsSet, DataSet* dataSet, Log *log)
{
    WriteHeaders(log);

    for(int i = 0; i < (int)paramsSet.size(); ++i)
    {
        MetaParameters params = paramsSet[i];
        TestResults results = TestRegression(dataSet, params, 1, 1);

        WriteExperimentInfo(params, results, log);
    }
}

void TestLearnCurve(const MetaParameters& params, DataSet* dataSet, Log *log)
{
    WriteHeaders(log);

    MetaParameters testParams = params;

    for(double learnPart = 0.8; learnPart < 0.85; learnPart += 0.3)
        for(testParams.TreesTotal = 400; testParams.TreesTotal < 1000; testParams.TreesTotal += 200)
        {
            double testPart = 0.2;
            TestResults results = TestRegression2(dataSet, testParams, learnPart, testPart);

            WriteExperimentInfo(params, results, log);
        }
}

double GetCOlesyaError(SerpIterator begin, SerpIterator end)
{
    DataSetIterator it(begin, end);
    double errorMin = std::numeric_limits<double>::max();

    for(double coeff = 1; coeff > 0.8; coeff -= 0.01)
    {
        double error = 0;

        for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
        {
            double bk_pctr = it.Current()->x[FIRST_ARINA_FEATURE];
            error += SQUARE(coeff * bk_pctr - it.Current()->y);
        }

        if(error < errorMin)
        {
            errorMin = error;
        }
    }

    return errorMin;
}

MetaParameters DisableFeatures(const string& disabledFeatures, const MetaParameters& params)
{
    vector<string> parts = Split(disabledFeatures, ',');
    MetaParameters params_modified = params;

    for(int i = 0; i < (int)parts.size(); ++i)
    {
        int featureId = atoi(parts[i].c_str());
        params_modified.AllowedFeatures[featureId] = false;
    }

    return params_modified;
}

MetaParameters GetMetaParametersFixed()
{  
    //
    // Значения по умолчанию
    //
    MetaParameters params;
    params.BoostingDelay = 25;
    params.GradientShrinkage = 0.02;
    params.TreeHeight = 5;
    params.TreesTotal = 200;
    params.BaggingPart = 0.5;
    params.ThresholdsCount = 30;
    params.RSMPart = 1;
    params.Features = 0;
    params.StopByTestError = true;
    params.TestPart = 0.5;
    params.FilterErrors = 0;
    params.WeightTrimming = 0;
    params.MinLeafSize = 1;

    //params.InitialRegression = (IRegression*)(new Arina());
    params.InitialRegression = (IRegression*)(new Karina());

    params.BaselineRegression.resize(4);
    params.BaselineRegression[0] = (IRegression*)(new AvgRegression("AVG"));
    params.BaselineRegression[1] = (IRegression*)(new OneFeatureRegression("OLESYA", FIRST_ARINA_FEATURE, 1));
    params.BaselineRegression[2] = (IRegression*)(new Arina());
    params.BaselineRegression[3] = (IRegression*)(new Karina());

    params.Metrics.resize(2);
    params.Metrics[0] = (QualityChecker*)(new MSE(WeightType_Constant));
    params.Metrics[1] = (QualityChecker*)(new LogLikelihood());

    params.AllowedFeatures.resize(68, true);
    params.AllowedFeatures[0] = false;
    params.AllowedFeatures[1] = false;

    return params;
}

vector<MetaParameters> GetMetaParameters_CTR()
{
    vector<MetaParameters> paramsSet;
    MetaParameters params = GetMetaParametersFixed();

    for(int i = 1; i < 2; ++i)
    {
        if(i == 1)
            std::swap(params.Metrics[0], params.Metrics[1]);

        for(params.TreeHeight = 3; params.TreeHeight <= 5; ++params.TreeHeight)
            for(params.GradientShrinkage = 0.01; params.GradientShrinkage < 0.2; params.GradientShrinkage *= 2)
                paramsSet.push_back(params);
    }

    return paramsSet;
}

vector<string> GetFeaturesNames()
{
    vector<string> names;

    names.push_back("");
    names.push_back("");
    names.push_back("PremCTR               ");
    names.push_back("PremCTR*tc            ");
    names.push_back("PremDomenCtr          ");
    names.push_back("RightCtr              ");
    names.push_back("RightDomenCtr         ");
    names.push_back("PremDomenCtr*PremCTR  ");
    names.push_back("RightCtr*PremCTR      ");
    names.push_back("RightDomenCtr*PremCTR ");
    names.push_back("QTailPrem             ");
    names.push_back("QTailRight            ");
    names.push_back("QTailPrem*PremCTR     ");
    names.push_back("QTailRight*PremCTR    ");

    names.push_back("TitleLen");
    names.push_back("UpperTitleShare");
    names.push_back("BannerLen");
    names.push_back("NonStopwordBanner");
    names.push_back("WordShareText");
    names.push_back("WordShareText2");
    names.push_back("WordShareBanner1");
    names.push_back("WordShareBanner2");
    names.push_back("QueryLen");
    names.push_back("NonStopwordQuery");
    names.push_back("TitleRelevance");
    names.push_back("TextRelevance");
    names.push_back("BannerRelevance");
    names.push_back("QueryTitleIntersect");
    names.push_back("QueryTextIntersect");
    names.push_back("QueryBannerIntersect");
    names.push_back("TextQtailIntersect");
    names.push_back("BannerQtailIntersect");
    names.push_back("SubsetTitle");
    names.push_back("SubsetText");
    names.push_back("SubsetBanner");
    names.push_back("CTR");
    names.push_back("PCTR");
    names.push_back("RClicks");
    names.push_back("RShows");
    names.push_back("TextShareCategory1");
    names.push_back("TextShareCategory2");
    names.push_back("BannerShareCategory1");
    names.push_back("BannerShareCategory2");
    names.push_back("TitleLemmCount");
    names.push_back("TextLemmCount");
    names.push_back("BannerLemmCount");
    names.push_back("QueryLemmCount");
    names.push_back("QTailLemmCount");
    names.push_back("PClicks");
    names.push_back("PShows");
    names.push_back("QTailWordCount");
    names.push_back("WordCount");
    names.push_back("RDomainCTR");
    names.push_back("PDomainCTR");
    names.push_back("RDomainShows");
    names.push_back("RDomainClicks");
    names.push_back("PDomainShows");
    names.push_back("PDomainClicks");
    names.push_back("UserSearchNotClicks");
    names.push_back("UserNetworkNotClicks");
    names.push_back("PQTailShows");
    names.push_back("PQTailClicks");
    names.push_back("RQTailShows");
    names.push_back("RQTailClicks");
    names.push_back("Hour");
    names.push_back("DayOfWeek");
    names.push_back("PCTR * PageNoFactor");
    names.push_back("PageNo");

    for(int i = 0; i < (int)names.size(); ++i)
        names[i].append(25 - names[i].size(), ' ');

    return names;
}
