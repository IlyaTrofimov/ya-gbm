#include <cmath>

#include "General.hpp"
#include "Regression.hpp"
#include "DataSetIterator.hpp"
#include "DecisionTree.hpp"
#include "QualityChecker.hpp"

void LogLikelihood::SetLeafValues(SerpIterator begin, SerpIterator end, int classesCount, DecisionTree *tree)
{
    tree->SetLeafsIndexes();

    vector<double> gradientSums(tree->GetTerminalNodesCount());
    vector<double> weightSums(tree->GetTerminalNodesCount());

    DataSetIterator itPoint(begin, end);

    for(itPoint.MoveFirst(); !itPoint.IsDone(); itPoint.MoveNext())
    {
        DecisionTreeNode *node = tree->GetLeaf(*itPoint.Current());
        double gradient = itPoint.Current()->gradient;

        gradientSums[node->index] += gradient;
        weightSums[node->index] += fabs(gradient) * (2 - fabs(gradient));
    }

    for(TreeIterator it = tree->GetIterator(); !it.IsDone(); it.MoveNext())
        if(it.GetCurrent()->nodeType == NodeType_Leaf)
        {
            DecisionTreeNode *node = it.GetCurrent();

            if(weightSums[node->index] < 0.001)
                weightSums[node->index] = 0.001;

            double leafValue = gradientSums[node->index] / weightSums[node->index];

            node->value = leafValue;
        }
}

void LogLikelihood::CalculateGradient(SerpIterator begin, SerpIterator end)
{
    DataSetIterator it(begin, end);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
    {
        double label = TO_MINUS_PLUS(it.Current()->y);
        double regression = it.Current()->regression;

        it.Current()->gradient = 2 * label / (1 + exp(2 * label * regression));
    }
}

double LogLikelihood::GetQuality(ConstSerpIterator begin, ConstSerpIterator end)
{
    double loglikelihood = 0;

    for(vector<Serp>::const_iterator it = begin; it != end; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            double target = it->at(i).target;

            if((int)it->at(i).y == 1)
                loglikelihood += -LOG_LIMITED(target);
            else
                loglikelihood += -LOG_LIMITED(1 - target);
        }

    return loglikelihood;
}

double LogLikelihood::GetQuality(IRegression* regressionMaker, ConstSerpIterator begin, ConstSerpIterator end)
{
    double loglikelihood = 0;

    for(vector<Serp>::const_iterator it = begin; it != end; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            double target = regressionMaker->Predict(it->at(i));

            if((int)it->at(i).y == 1)
                loglikelihood += -LOG_LIMITED(target);
            else
                loglikelihood += -LOG_LIMITED(1 - target);
        }

    return loglikelihood;
}

void LogLikelihood::CalculateTarget(SerpIterator begin, SerpIterator end)
{
    DataSetIterator it(begin, end);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
    {
        Point *point = it.Current();
        point->target = 1 / (1 + EXP_LIMITED(-2 * point->regression));
    }
}

void LogLikelihood::CalculateRegression(SerpIterator begin, SerpIterator end)
{
    DataSetIterator it(begin, end);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
    {
        Point *point = it.Current();
        double label = TO_MINUS_PLUS(point->y);
        double target = point->target;

        if(target < 0.0001)
            target = 0.0001;
        if(target > 0.9999)
            target = 0.9999;

        point->regression = 0.5 * log(target / ( 1 - target));
    }
}
