#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <algorithm>
#include <cstdlib>

#include "TaskData.hpp"
#include "General.hpp"
#include "DecisionTree.hpp"
#include "QualityChecker.hpp"
#include "Metrics.hpp"
#include "FilesIO.hpp"
#include "TreeBuilder.hpp"
#include "Regression.hpp"
#include "DataSetIterator.hpp"
#include "DataSet2.hpp"
#include "TreeBuilder.hpp"

using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::endl;
using std::pair;
using std::cout;
using std::cin;
using std::make_pair;
using std::back_inserter;
using std::copy;
using std::sort;

DecisionTree GetConstantTree(double value)
{
    DecisionTreeNode *node = new DecisionTreeNode();
    node->left = NULL;
    node->right = NULL;
    node->nodeType = NodeType_Leaf;
    node->value = value;

    return DecisionTree(node);
}

void AdjustRegression(SerpIterator begin, SerpIterator end, const pair<double, DecisionTree>& term)
{
    DataSetIterator it(begin, end);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
        it.Current()->regression += term.first * term.second.GetValue(*it.Current());
}

void CalculateThresholdData(DataSet2& dataSet, const TaskData& taskData, vector< vector<ThresholdStat> >* stat)
{
    DataSetIterator it = dataSet.GetIterator();

    stat->resize(dataSet.GetFeaturesCount());

    for(int featureId = 0; featureId < dataSet.GetFeaturesCount(); ++featureId)
    {
        int thresholdsCount = taskData.GetThresholds(featureId)->size();
        vector<ThresholdStat>* featureStat = &(stat->at(featureId));
        featureStat->resize(thresholdsCount);

        for(int j = 0; j < thresholdsCount; ++j)
        {
            featureStat->at(j).Threshold = taskData.GetThresholds(featureId)->at(j);
            featureStat->at(j).PointsCount = 0;
            featureStat->at(j).AvgResponce = 0;
            featureStat->at(j).Dispersion = 0;
        }

        for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
        {
            const Point* point = it.Current();
            double feature = point->x[featureId];
            double value = point->y;
            int thresholdIndex = point->thresholds[featureId];

            featureStat->at(thresholdIndex).AvgResponce += value;
            featureStat->at(thresholdIndex).PointsCount++;
        }

        for(int j = 0; j < thresholdsCount; ++j)
        {
            if(stat->at(featureId)[j].PointsCount > 0)
                stat->at(featureId)[j].AvgResponce /= stat->at(featureId)[j].PointsCount;
            else
                stat->at(featureId)[j].AvgResponce = 0;
        }

        for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
        {
            const Point* point = it.Current();
            double feature = point->x[featureId];
            double value = point->y;
            int thresholdIndex = point->thresholds[featureId];

            stat->at(featureId)[thresholdIndex].Dispersion += SQUARE(value - stat->at(featureId)[thresholdIndex].AvgResponce);
        }
    }
}

string Regression::Get_C_code(double shinkage, DecisionTreeNode *node, int level) const
{
    string code;
    string indentation = string(level, '\t');

    if(node->nodeType == NodeType_Internal)
    {
        char buffer[1024];
        sprintf(buffer, "if (x[%d] < %f)\n", node->predicate.GetFeatureId(), node->predicate.GetThreshold());
        code += indentation + string(buffer);
        code += Get_C_code(shinkage, node->left, level + 1);

        code += indentation + "else\n";
        code += Get_C_code(shinkage, node->right, level + 1);
    }
    else
    {
        char buffer[1024];
        sprintf(buffer, "value += %f;\n", shinkage * node->value);

        code += indentation + string(buffer);
    }

    return code;
}

string Regression::Get_C_code(const pair<double, DecisionTree>& term) const
{
    return Get_C_code(term.first, term.second.GetRoot(), 1);
}

string Regression::Get_C_code() const
{
    string code = "";

    code += "double get_regression(double x[])\n";
    code += "{\n";
    code += "   double value = 0;\n";
    code += "\n";

    for(int i = 0; i < _series.size(); ++i)
    {
        code += Get_C_code(_series[i]);
        code += "\n";
    }

    code += "    return value;\n";
    code += "}\n";

    return code;
}

void Regression::Learn(const MetaParameters& params,
                         SerpIterator itTrainBegin, SerpIterator itTrainEnd,
                         SerpIterator itTestBegin, SerpIterator itTestEnd,
                         TestResults *results)
{
    results->TestError = std::numeric_limits<double>::max();
    int bestLength = -1;

    _taskData.Calculate(itTrainBegin, itTrainEnd, params.ThresholdsCount);
    FillThresholdsData(_taskData.GetAllThresholds(), itTrainBegin, itTrainEnd);

    DataSetIterator itTrain(itTrainBegin, itTrainEnd);
    DataSetIterator itTest(itTestBegin, itTestEnd);

    DataSet2 dataSetLearn(itTrainBegin, itTrainEnd);
    CalculateThresholdData(dataSetLearn, _taskData, &(results->Stat));

    for(itTrain.MoveFirst(); !itTrain.IsDone(); itTrain.MoveNext())
    {
        if(params.InitialRegression != NULL) {
            itTrain.Current()->target = params.InitialRegression->Predict(*itTrain.Current());
        }

        itTrain.Current()->disabled = false;
    }

    for(itTest.MoveFirst(); !itTest.IsDone(); itTest.MoveNext())
    {
        if(params.InitialRegression != NULL) {
            itTest.Current()->target = params.InitialRegression->Predict(*itTest.Current());
        }

        itTest.Current()->disabled = false;
        
        for (int i = 0; i < 10; ++i) {
            //itTest.Current()->present[i] = false;
        }
    }

    for(itTest.MoveFirst(); !itTest.IsDone(); itTest.MoveNext())
    {
        for (int i = 0; i < 20; ++i) {
            //cout << i << " " << itTest.Current()->present[i] << endl;
        }
    }

    QualityChecker* metrics = params.Metrics[0];

    metrics->CalculateRegression(itTrainBegin, itTrainEnd);
    metrics->CalculateRegression(itTestBegin, itTestEnd);

    _series.clear();

    for(int treesCount = 0; treesCount < params.TreesTotal; ++treesCount)
    {
        metrics->CalculateGradient(itTrainBegin, itTrainEnd);

        double learnError = metrics->GetQuality(itTrainBegin, itTrainEnd) / (itTrainEnd - itTrainBegin);
        double examError = metrics->GetQuality(itTestBegin, itTestEnd) / (itTestEnd - itTestBegin);
        double examError2 = params.Metrics[1]->GetQuality(itTestBegin, itTestEnd) / (itTestEnd - itTestBegin);

        int treeLeafs = (_series.empty() ? 0 : _series.back().second.GetLeafsCount());

        cout << treesCount << "\t" << treeLeafs << "\t" << learnError << "\t" << examError << "\t" << examError2;
        

        vector<Point*> randomSubset;
        GetRandomSubset(itTrainBegin, itTrainEnd, params.BaggingPart, &randomSubset);

        vector<bool> allowedFeatures(_taskData.GetFeaturesCount());
        GetRandomSubspace(params.RSMPart, &allowedFeatures);

        for(int i = 0; i < allowedFeatures.size() && i < params.AllowedFeatures.size(); ++i)
            allowedFeatures[i] = params.AllowedFeatures[i];

        DecisionTreeNode *root = BuildNormalTree(_taskData, randomSubset, params.TreeHeight, allowedFeatures, params.MinLeafSize);
        DecisionTree tree(root);

        metrics->SetLeafValues(itTrainBegin, itTrainEnd, 2, &tree);

        if(!_series.empty())
            if(tree.EqualStructure(_series.back().second))
                cout << " EQUAL ";

        cout << "\t" << GetTreeError(tree);
        cout << endl;

        _series.push_back(make_pair(params.GradientShrinkage, tree));

        AdjustRegression(itTrainBegin, itTrainEnd, _series.back());
        AdjustRegression(itTestBegin, itTestEnd, _series.back());

        metrics->CalculateTarget(itTrainBegin, itTrainEnd);
        metrics->CalculateTarget(itTestBegin, itTestEnd);

        //cout << tree.ToString();

        results->Length = _series.size();

        if(examError < results->TestError)
        {
            results->LearnError = learnError;
            results->TestError = examError;
            results->TestQuality = examError2;
            bestLength = _series.size() - 1;
        }

        if(params.StopByTestError && (treesCount - bestLength >= params.BoostingDelay))
        {
            results->Length = bestLength;
            _series.resize(bestLength);
            break;
        }
    }

    int treeLeafs = (_series.empty() ? 0 : _series.back().second.GetLeafsCount());
    double learnError = metrics->GetQuality(itTrainBegin, itTrainEnd) / (itTrainEnd - itTrainBegin);
    double examError = metrics->GetQuality(itTestBegin, itTestEnd) / (itTestEnd - itTestBegin);
    
    cout << treeLeafs << " " << learnError << " " << examError << endl;
    cout << _series.back().second.ToString();
}

double Regression::Predict(const Point& point) const
{
    return GetSeriesValue(_series, point);
}

void Regression::GetFeaturesImportance(vector<double> *importance) const
{
    for(int i = 0; i < _series.size(); ++i)
    {
        for(TreeIterator it = _series[i].second.GetIterator(); !it.IsDone(); it.MoveNext())
        {
            if(it.GetCurrent()->nodeType == NodeType_Internal)
            {
                PredicateStat stat = it.GetCurrent()->stat;
                int featureId = it.GetCurrent()->predicate.GetFeatureId();

                importance->at(featureId) += stat.error - stat.left_error - stat.right_error;
            }
        }
    }
}
