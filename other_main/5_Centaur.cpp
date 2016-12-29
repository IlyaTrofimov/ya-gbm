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
vector<string> GetFeaturesNamesC();

void TestMetaParametersC(const vector<MetaParameters>& paramsSet, DataSet* dataSet, Log *log);
MetaParameters GetMetaParametersFixedC();

int main()
{
    string dataFileName = "housing.data2";

    Log log;
    log.ExperimentId = 0;
    log.FeauresFile = dataFileName + ".features";
    log.ResultsFile = dataFileName + ".results";
    log.FeaturesName = GetFeaturesNamesC();

    DataSet dataSet;
    //GetDataFactorLog(dataFileName, 1.0, -1, NULL, 13, &dataSet);

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
    MetaParameters defaultParams = GetMetaParametersFixedC();

    paramsSet.push_back(defaultParams);
    TestMetaParametersC(paramsSet, &dataSet, &log);

    return 0;
}

void TestMetaParametersC(const vector<MetaParameters>& paramsSet, DataSet* dataSet, Log *log)
{
    WriteHeaders(log);

    for(int i = 0; i < (int)paramsSet.size(); ++i)
    {
        MetaParameters params = paramsSet[i];
        TestResults results = TestRegression(dataSet, params, 1, 1);

        WriteExperimentInfo(params, results, log);
    }
}

MetaParameters GetMetaParametersFixedC()
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

    params.InitialRegression = NULL;

    params.BaselineRegression.resize(4);
    params.BaselineRegression[0] = (IRegression*)(new AvgRegression("AVG"));
    params.BaselineRegression[1] = (IRegression*)(new OneFeatureRegression("OLESYA", FIRST_ARINA_FEATURE, 1));

    params.Metrics.resize(2);
    params.Metrics[0] = (QualityChecker*)(new MSE(WeightType_Constant));
    params.Metrics[1] = (QualityChecker*)(new LogLikelihood());

    params.AllowedFeatures.resize(68, true);
    params.AllowedFeatures[0] = false;
    params.AllowedFeatures[1] = false;

    return params;
}

vector<string> GetFeaturesNamesC()
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
