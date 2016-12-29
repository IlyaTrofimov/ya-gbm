#include "General.hpp"
#include "DecisionTree.hpp"
#include "MulticlassClassifier.hpp"
#include "TreeBuilder.hpp"
#include "DataSetIterator.hpp"

#include <limits>
#include <algorithm>
#include <cmath>
#include <map>

using std::map;

int MulticlassClassifier::GetClass(const Point& point) const
{
    int classNumber = 0;
    double maxProbability = -std::numeric_limits<double>::max();

    for(int i = 0; i < (int)_classifiers.size(); ++i)
    {
        double probability = GetSeriesValue(_classifiers[i], point);

        if(probability > maxProbability)
        {
            maxProbability = probability;
            classNumber = i;
        }
    }

    return classNumber;
}

double MulticlassClassifier::GetCondidence(const Point& point, int classNumber) const
{
    double sum_exp_f = 0;

    for(int i = 0; i < (int)_classifiers.size(); ++i)
    {
        double f = GetSeriesValue(_classifiers[i], point);
        double exp_f = EXP_LIMITED(f);
        sum_exp_f += exp_f;
    }

    double f_class =  GetSeriesValue(_classifiers[classNumber], point);
    double exp_f_class = EXP_LIMITED(f_class);

    return exp_f_class / sum_exp_f;
}

double MulticlassClassifier::GetError(SerpIterator begin, SerpIterator end)
{
    double error = 0;

    for(SerpIterator it = begin; it != end; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            if((int)it->at(i).y != GetClass(it->at(i)))
                ++error;
        }
        
    return error;
}

void MulticlassClassifier::GetFeaturesStat(vector<int> *stat) const
{
    for(int i = 0; i < (int)_classifiers.size(); i++)
        for(int k = 0; k < (int)_classifiers[i].size(); k++)
        {
            const DecisionTree *tree = &(_classifiers[i][k].second);

            for(TreeIterator it = tree->GetIterator(); !it.IsDone(); it.MoveNext())
            {
                Predicate predicate = it.GetCurrent()->predicate;

                if(it.GetCurrent()->nodeType == NodeType_Internal)
                {
                    ++(stat->at(predicate.GetFeatureId()));
                }
            }
        }
}

void CalculateGradient(SerpIterator learnBegin, SerpIterator learnEnd, int classNumber, const vector< vector<double> >& p)
{
    int pos = 0;

    for(SerpIterator it = learnBegin; it != learnEnd; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            if((int)it->at(i).y == classNumber)
                it->at(i).gradient = 1 - p[pos][classNumber];
            else
                it->at(i).gradient =  -p[pos][classNumber];

            ++pos;
        }
}

void SetLeafValues(SerpIterator begin, SerpIterator end, int classesCount, DecisionTree *tree)
{
    tree->SetLeafsIndexes();

    vector<double> gradientSums(tree->GetTerminalNodesCount());
    vector<double> weightSums(tree->GetTerminalNodesCount());

    for(SerpIterator it = begin; it != end; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            DecisionTreeNode *node = tree->GetLeaf(it->at(i));
            double gradient = it->at(i).gradient;

            gradientSums[node->index] += gradient;
            weightSums[node->index] += fabs(gradient) * (1 - fabs(gradient));
        }

    for(TreeIterator it = tree->GetIterator(); !it.IsDone(); it.MoveNext())
        if(it.GetCurrent()->nodeType == NodeType_Leaf)
        {
            double factor = (classesCount - 1) / (double)classesCount;
            DecisionTreeNode *node = it.GetCurrent();

            if(weightSums[node->index] < 0.001)
                weightSums[node->index] = 0.001;

            double leafValue = factor * gradientSums[node->index] / weightSums[node->index];
            it.GetCurrent()->value = leafValue;
        }
}

void GetClassesCount(const MulticlassClassifier& classifier, ConstSerpIterator begin, ConstSerpIterator end,
                     vector<int> *classesCount, vector<int> *realClassesCount, vector<int> *rightClassesCount)
{
     for(ConstSerpIterator it = begin; it != end; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            int realClass = (int)it->at(i).y;
            ++realClassesCount->at(realClass);

            int predictedClass = classifier.GetClass(it->at(i));
            ++(classesCount->at(predictedClass));

            if(realClass == predictedClass)
                ++rightClassesCount->at(realClass);
        }
}

void SetLowWeightDisabledPoints(double part, DataSetIterator *it)
{
    double weightsTotal = 0;
    vector< pair<Point*, double> > pointWeights;

    for(it->MoveFirst(); !it->IsDone(); it->MoveNext())
    {
        Point *point = it->Current();
        double gradient = point->gradient;
        double weight = fabs(gradient) * (1 - fabs(gradient));
        weightsTotal += weight;

        point->disabled = false;
        pointWeights.push_back(make_pair(point, weight));
    }

    sort(pointWeights.begin(), pointWeights.end(), PairSecondCompare2);
    double weightsPartSum = 0;

    for(int i = 0; i < (int)pointWeights.size(); ++i)
    {
        weightsPartSum += pointWeights[i].second;

        if(weightsPartSum < part * weightsTotal)
            pointWeights[i].first->disabled = true;
        else
            break;
    }
}

double GetLearnPart(SerpIterator learnBegin, SerpIterator learnEnd)
{
    int total = 0;
    int enabled = 0;

    for(SerpIterator it = learnBegin; it != learnEnd; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            ++total;
            if(!it->at(i).disabled && !it->at(i).totalDisabled)
                ++enabled;
        }

    return (double)enabled / total;
}

void CalculateProbabilities(const TaskData& taskData, const vector< vector<double> >& f, vector< vector<double> > *p)
{
    int learnPointsCount = (int)f.size();

    for(int i = 0; i < learnPointsCount; ++i)
    {
        double sum_e_f = 0;
        vector<double> e_f(taskData.GetClassesCount(), 0);

        for(int classNumber = 0; classNumber < taskData.GetClassesCount(); ++classNumber)
        {
            e_f[classNumber] = EXP_LIMITED(f[i][classNumber]);
            sum_e_f += e_f[classNumber];
        }

        for(int classNumber = 0; classNumber < taskData.GetClassesCount(); ++classNumber)
            p->at(i)[classNumber] = e_f[classNumber] / sum_e_f;
    }
}

double GetAvgF1(const MulticlassClassifier& classifier, int classesTotal, ConstSerpIterator testBegin, ConstSerpIterator testEnd)
{
    vector<int> classesCount(classesTotal, 0);
    vector<int> realClassesCount(classesTotal, 0);
    vector<int> rightClassesCount(classesTotal, 0);
    GetClassesCount(classifier, testBegin, testEnd, &classesCount, &realClassesCount, &rightClassesCount);

    double metrics2 = 0;
    for(int i = 0; i < classesTotal; ++i)
    {
        double prec = (100*rightClassesCount[i] + 1) / (classesCount[i] + 1);
        double rec = (100*rightClassesCount[i] + 1) / (realClassesCount[i] + 1);

        metrics2 += 2 / (1/prec  + 1/rec);
    }
    metrics2 /= classesTotal;

    return metrics2;
}

void DisableNotInformativeFeatures(MetaParameters params, vector<bool> *allowedFeatures)
{
    int featuresFreq[] = {71, 72, 63, 202, 52, 66, 73, 183, 47, 0, 58, 37, 64, 41, 64, 38, 0, 55, 35, 32, 0, 61, 79, 38, 44, 69, 53, 72, 32, 64, 51, 56, 33, 31, 31, 51, 46, 42, 0, 46, 54, 24, 32, 43, 46, 143, 46, 85, 10, 40, 129, 0, 166, 81, 0, 0, 0, 0, 40, 12, 35, 52, 92, 49, 68, 89, 0, 62, 56, 3, 0, 19, 20, 28, 11, 16, 22, 26, 47, 7, 17, 55, 0, 40, 29, 31, 23, 0, 0, 0, 0, 0, 0, 0, 41, 30, 17, 44, 47, 31, 17, 24, 48, 0, 27, 25, 36, 53, 30, 49, 25, 27, 26, 29, 111, 58, 29, 22, 0, 19, 0, 34, 0, 14, 0, 12, 0, 0, 41, 0, 24, 0};
    for(int i = 0; i < (int)allowedFeatures->size(); ++i)
        allowedFeatures->at(i) =  allowedFeatures->at(i) & (featuresFreq[i] >= params.Features);
}

void AdjustRegression(const DecisionTree& tree, int classNumber, double alpha,
                      ConstSerpIterator learnBegin, ConstSerpIterator learnEnd, vector< vector<double> > *f)
{
    int pos = 0;
    for(ConstSerpIterator it = learnBegin; it != learnEnd; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            double value = tree.GetValue(it->at(i));
            f->at(pos)[classNumber] += alpha * value;

            ++pos;
        }
}

void MulticlassClassifier::Learn(
                 const TaskData& taskData, const MetaParameters& params,
                 SerpIterator learnBegin, SerpIterator learnEnd,
                 SerpIterator testBegin, SerpIterator testEnd,
                 bool stopAtNoisePart,
                 TestResults *results)
{
    results->TestError = std::numeric_limits<double>::max();

    int bestLength = -1;
    double alpha = params.GradientShrinkage;
    int learnPointsCount = 0, examPointsCount = 0;
    double quality2 = 0;
    int pos;

    for(SerpIterator it = learnBegin; it != learnEnd; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
            ++learnPointsCount;

    for(SerpIterator it = testBegin; it != testEnd; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
            ++examPointsCount;

    int prevLeafs = 0;
    int treeLeafs = 0;

    vector< vector<double> > p(learnPointsCount, vector<double>(taskData.GetClassesCount(), 0));
    vector< vector<double> > f(learnPointsCount, vector<double>(taskData.GetClassesCount(), 0));
    _classifiers.clear();
    _classifiers.resize(taskData.GetClassesCount());

    DataSetIterator it(learnBegin, learnEnd);
  
    for(int treesCount = 0; treesCount < params.TreesTotal; ++treesCount)
    {
        CalculateProbabilities(taskData, f, &p);

        double learnError = GetError(learnBegin, learnEnd);
        double testError = GetError(testBegin, testEnd);
        double quality = GetAvgF1(*this, taskData.GetClassesCount(), testBegin, testEnd);

//        quality = 0.95 * quality + 0.05 * qualityFunction(testBegin, testEnd);		
        treeLeafs /= taskData.GetClassesCount();
        double learnPart = GetLearnPart(learnBegin, learnEnd);
        cout << treesCount << "\t" << treeLeafs << "\t" << (int)(learnPart * 100) << "%\t" << learnError << "\t" << testError << "\t" << quality << "\t";

        treeLeafs = 0;

        for(int classNumber = 0; classNumber < taskData.GetClassesCount(); ++classNumber)
        {
            CalculateGradient(learnBegin, learnEnd, classNumber, p);

            SetLowWeightDisabledPoints(params.WeightTrimming, &it);

            vector<Point*> randomSubset;
            GetRandomSubset(learnBegin, learnEnd, params.BaggingPart, &randomSubset);

            vector<bool> allowedFeatures(taskData.GetFeaturesCount());
            GetRandomSubspace(params.RSMPart, &allowedFeatures);
            DisableNotInformativeFeatures(params, &allowedFeatures);

            int leafRestriction = 1; //(int)sqrt((double)learnPointsCount);
            //DecisionTreeNode *root = BuildNormalTree(taskData, randomSubset, params.TreeHeight, allowedFeatures, leafRestriction);
            DecisionTreeNode *root = BuildFirstBestTree(taskData, randomSubset, params.TreeHeight,  allowedFeatures, leafRestriction);
            

            DecisionTree tree(root);
            treeLeafs += tree.GetLeafsCount();

            //tree = PruneTree(tree, 5);
            SetLeafValues(learnBegin, learnEnd, taskData.GetClassesCount(), &tree);

            // UpdateStatistics(tree);

             if(!_classifiers[classNumber].empty())
                if(tree.EqualStructure(_classifiers[classNumber].back().second))
                    cout << "EQ " << classNumber;

            _classifiers[classNumber].push_back(make_pair(alpha, tree));

            AdjustRegression(tree, classNumber, alpha, learnBegin, learnEnd, &f);
        }
        cout << endl;

        if(testError < results->TestError)
        {
            results->TestError = testError;
            results->LearnError = learnError;
            results->TestQuality = quality;
            bestLength = treesCount + 1;
        }

        if(treesCount - bestLength >= params.BoostingDelay && params.StopByTestError)
        {
            for(int i = 0; i < taskData.GetClassesCount(); ++i)
                _classifiers[i].resize(bestLength);

            break;
        }

        if(stopAtNoisePart && learnError < params.NoiseDeletePart * learnPointsCount)
            break;
    }

    cout << "\n\n" << params.WeightTrimming << "\n\n";

    if(params.StopByTestError)
        results->Length = bestLength;
}