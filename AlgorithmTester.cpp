#include "General.hpp"
#include "QualityChecker.hpp"
#include "FilesIO.hpp"
#include "MulticlassClassifier.hpp"
#include "TaskData.hpp"
#include "DataSetIterator.hpp"
#include "Regression.hpp"
#include "DataSet2.hpp"

#include <fstream>
#include <algorithm>
#include <map>

using std::map;

MulticlassClassifier TrainAlgorithm(const MetaParameters& params, DataSet *dataSet)
{
    MulticlassClassifier classifier = MulticlassClassifier();

    int learnSize = dataSet->size() * (1 - params.TestPart);
    int testSize = dataSet->size() - learnSize;

    TaskData taskData;
    taskData.Calculate(dataSet->begin(), dataSet->begin() + learnSize, params.ThresholdsCount);

    DataSetIterator it(dataSet);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
        it.Current()->totalDisabled = false;

    time_t beginTime = clock();
    TestResults results;

    classifier.Learn(taskData, params,
            dataSet->begin(), dataSet->begin() + learnSize,
            dataSet->begin() + learnSize, dataSet->end(),
            false, &results);

    if(params.FilterErrors)
    {
        for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
            if(classifier.GetClass(*(it.Current())) != (int)it.Current()->y)
                it.Current()->totalDisabled = true;

        classifier.Learn(taskData, params,
                dataSet->begin(), dataSet->begin() + learnSize,
                dataSet->begin() + learnSize, dataSet->end(),
                false, &results);
    }

    return classifier;
}

TestResults TestAlgorithm(DataSet *dataSet, const MetaParameters& params, int passes)
{
    MulticlassClassifier classifier = MulticlassClassifier();

    TestResults totalResults;
    totalResults.LearnError = 0;
    totalResults.LearnQuality = 0;
    totalResults.Length = 0;
    totalResults.TestError = 0;
    totalResults.TestQuality = 0;
    totalResults.LearnErrorRelStd = 0;
    totalResults.TestErrorRelStd = 0;
    totalResults.Time = 0;

    vector< vector<double> > g(params.Metrics.size(), vector<double>(params.BaselineRegression.size(), 0.0));
    totalResults.BaseLineMetrics = g;

    for(int j = 0; j < passes; j++)
    {
        if(j > 0)
            std::random_shuffle(dataSet->begin(), dataSet->end());

        int learnSize = dataSet->size() * (1 - params.TestPart);
        int testSize = dataSet->size() - learnSize;

        TaskData taskData;
        taskData.Calculate(dataSet->begin(), dataSet->begin() + learnSize, params.ThresholdsCount);

        DataSetIterator it(dataSet);

        for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
            it.Current()->totalDisabled = false;

        time_t beginTime = clock();
        TestResults results;

        classifier.Learn(taskData, params,
                dataSet->begin(), dataSet->begin() + learnSize,
                dataSet->begin() + learnSize, dataSet->end(),
                false, &results);

        if(params.FilterErrors)
        {
            for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
                if(classifier.GetClass(*(it.Current())) != (int)it.Current()->y)
                    it.Current()->totalDisabled = true;

            classifier.Learn(taskData, params,
                    dataSet->begin(), dataSet->begin() + learnSize,
                    dataSet->begin() + learnSize, dataSet->end(),
                    false, &results);
        }

        totalResults.LearnError += results.LearnError;
        totalResults.TestError += results.TestError;
        totalResults.LearnQuality += results.LearnQuality;
        totalResults.TestQuality += results.TestQuality;
        totalResults.Length += results.Length;
        totalResults.Time += results.Time;

        time_t endTime = clock();
        results.Time += (endTime - beginTime) / CLOCKS_PER_SEC;
    }

    totalResults.LearnError /= passes;
    totalResults.TestError /= passes;
    totalResults.LearnQuality /= passes;
    totalResults.TestQuality /= passes;
    totalResults.Length /= passes;
    totalResults.Time /= passes;

    return totalResults;
}

TestResults TestRegression2(DataSet *dataSet, const MetaParameters& params, double learnPart, double testPart)
{
    Regression regression = Regression();
    TestResults results;

    vector< vector<double> > g(params.Metrics.size(), vector<double>(params.BaselineRegression.size(), 0.0));
    results.BaseLineMetrics  = g;

    results.Metrics = vector<double>(params.Metrics.size(), 0.0);

    int testPointsCount = dataSet->size() * testPart;
    int learnSize = dataSet->size() * learnPart;

    int testSize = testPointsCount;

    DataSet2 dataSet2(dataSet->begin(), dataSet->end());

    if(results.Importance.size() == 0)
        results.Importance = vector<double>(dataSet2.GetFeaturesCount());

    DataSetIterator it(dataSet);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
        it.Current()->totalDisabled = false;

    time_t beginTime = clock();

    SerpIterator itTrainBegin = dataSet->begin();
    SerpIterator itTrainEnd = dataSet->begin() + learnSize;
    SerpIterator itTestBegin = dataSet->end() - testSize;
    SerpIterator itTestEnd = dataSet->end();

    regression.Learn(params, itTrainBegin, itTrainEnd, itTestBegin, itTestEnd, &results);

    regression.GetFeaturesImportance(&results.Importance);

     for(int m = 0; m < params.Metrics.size(); ++m)
     {
        QualityChecker *checker = params.Metrics[m];
        results.Metrics[m] = checker->GetQuality(itTestBegin, itTestEnd) / testSize;
     }

    for(int b = 0; b < params.BaselineRegression.size(); ++b)
    {
        TestResults baselineResults;

        params.BaselineRegression[b]->Learn(params,
            dataSet->begin(), dataSet->begin() + learnSize,
            dataSet->begin() + learnSize, dataSet->end(),
            &baselineResults);

        for(int m = 0; m < params.Metrics.size(); ++m)
        {
            QualityChecker *checker = params.Metrics[m];
            IRegression *regression = params.BaselineRegression[b];

            results.BaseLineMetrics[m][b] = checker->GetQuality(params.BaselineRegression[b], itTestBegin, itTestEnd) / testSize;
        }
    }

    time_t endTime = clock();
    results.Time == (endTime - beginTime) / (double)CLOCKS_PER_SEC;

    ofstream ofs("C:\\1.txt");
    ofs << regression.Get_C_code();
    ofs.close();

    return results;
}

TestResults TestRegression(DataSet *dataSet, const MetaParameters& params, int folds, int passes)
{
    Regression regression = Regression();

    TestResults totalResults;
    totalResults.LearnError = 0;
    totalResults.LearnQuality = 0;
    totalResults.Length = 0;
    totalResults.TestError = 0;
    totalResults.TestQuality = 0;
    totalResults.LearnErrorRelStd = 0;
    totalResults.TestErrorRelStd = 0;
    totalResults.Time = 0;

    vector< vector<double> > g(params.Metrics.size(), vector<double>(params.BaselineRegression.size(), 0.0));
    totalResults.BaseLineMetrics  = g;

    totalResults.Metrics = vector<double>(params.Metrics.size(), 0.0);

    //params.TestPart = 1.0 / folds;

    int testPointsCount = dataSet->size() * params.TestPart;

    for(int i = 0; i < passes; ++i)
    {
        if(i > 0)
            std::random_shuffle(dataSet->begin(), dataSet->end());

        for(int j = 0; j < folds; j++)
        {
            int learnSize = dataSet->size() * (1 - params.TestPart);
            int testSize = dataSet->size() - learnSize;

            if(j > 0)
                std::rotate(dataSet->begin(), dataSet->begin() + testPointsCount, dataSet->end());

            DataSet2 dataSet2(dataSet->begin(), dataSet->end());

            if(totalResults.Importance.size() == 0)
                totalResults.Importance = vector<double>(dataSet2.GetFeaturesCount());

            DataSetIterator it(dataSet);

            for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
                it.Current()->totalDisabled = false;

            time_t beginTime = clock();

            TestResults results;

            regression.Learn(params,
                    dataSet->begin(), dataSet->begin() + learnSize,
                    dataSet->begin() + learnSize, dataSet->end(),
                    &results);

            regression.GetFeaturesImportance(&totalResults.Importance);

            totalResults.LearnError += results.LearnError;
            totalResults.TestError += results.TestError;
            totalResults.LearnQuality += results.LearnQuality;
            totalResults.TestQuality += results.TestQuality;
            totalResults.Length += results.Length;
            totalResults.Stat = results.Stat;

             for(int m = 0; m < params.Metrics.size(); ++m)
             {
                QualityChecker *checker = params.Metrics[m];

                totalResults.Metrics[m] += 
                    checker->GetQuality(dataSet->begin() + learnSize, dataSet->end()) / testSize;
             }

            for(int b = 0; b < params.BaselineRegression.size(); ++b)
            {
                TestResults baselineResults;

                params.BaselineRegression[b]->Learn(params,
                    dataSet->begin(), dataSet->begin() + learnSize,
                    dataSet->begin() + learnSize, dataSet->end(),
                    &baselineResults);

                for(int m = 0; m < params.Metrics.size(); ++m)
                {
                    QualityChecker *checker = params.Metrics[m];
                    IRegression *regression = params.BaselineRegression[b];

                    totalResults.BaseLineMetrics[m][b] += 
                        checker->GetQuality(params.BaselineRegression[b], dataSet->begin() + learnSize, dataSet->end()) / testSize;
                }
            }

            time_t endTime = clock();
            totalResults.Time += (endTime - beginTime) / (double)CLOCKS_PER_SEC;
        }
    }

    totalResults.LearnError /= passes;
    totalResults.TestError /= passes;
    totalResults.LearnQuality /= passes;
    totalResults.TestQuality /= passes;
    totalResults.Length /= passes;
    totalResults.Time /= passes;

    for(int m = 0; m < params.Metrics.size(); ++m)
    {
        for(int b = 0; b < params.BaselineRegression.size(); ++b)
            totalResults.BaseLineMetrics[m][b] /= passes;
        
        totalResults.Metrics[m] /= passes;
    }

    ofstream ofs("C:\\CTR\\1.cpp");
    ofs << regression.Get_C_code();
    ofs.close();

    return totalResults;
}

void FilterFeatures(DataSet* dataSet, Log *log, MetaParameters* params)
{
    WriteLine("*************", log);
    WriteLine("Фильтрация признаков:", log);

    MSE mse(WeightType_Constant);
    QualityChecker *checker = (QualityChecker*)&mse;

    TestResults results = TestRegression(dataSet, *params, 1, 1);
    WriteExperimentInfo(*params, results, log);

    for(int i = 0; i < results.Importance.size(); ++i)
        if(results.Importance[i] < 0.001)
        {
            params->AllowedFeatures[i] = false;
            WriteLine(PadRight(ToString(i), 10) + ToString(results.Importance[i]), log);
        }

    WriteLine("*************", log);
}

void FindWorstFeature(const MetaParameters& params_in, DataSet* dataSet, Log* log, pair<double, int> *featureInfo)
{
    MSE mse(WeightType_Constant);
    QualityChecker *checker = &mse;

    int featureId = 0;
    map<double, int> featuresQuality;

    while(featureId < params_in.AllowedFeatures.size())
    {
        MetaParameters params = params_in;

        if(params.AllowedFeatures[featureId])
            params.AllowedFeatures[featureId] = false;
        else
        {
            ++featureId;
            continue;
        }

        TestResults results = TestRegression(dataSet, params, 1, 1);

        featuresQuality.insert(make_pair(results.TestError, featureId));
        log->ExperimentId++;

        WriteLine(string("Пробуем удалить признак ") + ToString(featureId), log);
        WriteExperimentInfo(params, results, log);

        ++featureId;
    }

    *featureInfo = *featuresQuality.begin();
}

void DelStrategy(const MetaParameters& params_in, int deleteCount, DataSet* dataSet, Log *log)
{
    WriteHeaders(log);
    WriteLine("*************", log);
    WriteLine("Стратегия Del", log);
    WriteLine("*************", log);

    MetaParameters params = params_in;
    FilterFeatures(dataSet, log, &params);

    vector< pair<double,int> > deletedFeatures;

    for(int i = 0; i < deleteCount; ++i)
    {
        int featureId;
        pair<double, int> featureInfo;

        FindWorstFeature(params, dataSet, log, &featureInfo);

        featureId = featureInfo.second;
        deletedFeatures.push_back(featureInfo);

        WriteLine("*************", log);
        WriteLine(string("Удален признак ") + ToString(featureId), log);
        WriteLine("*************", log);

        params.AllowedFeatures[featureId] = false;
    }

    WriteLine("", log);
    WriteLine("*************", log);
    WriteLine(string("Итого удалены признаки:"), log);

    for(int i = 0; i < deletedFeatures.size(); ++i)
    {
        int featureId = deletedFeatures[i].first;
        double metrics = deletedFeatures[i].second;

        WriteLine(PadRight(ToString(metrics), 10) + ToString(featureId), log);
    }

    WriteLine("*************", log);
}
