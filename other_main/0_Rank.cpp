#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <algorithm>

#include "TaskData.hpp"
#include "General.hpp"
#include "DecisionTree.hpp"
#include "QualityChecker.hpp"
#include "Metrics.hpp"
#include "FilesIO.hpp"
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

vector<int> g_FeaturesInCount;
vector<Predicate> g_Predicates;

void UpdateStatistics(const DecisionTree& tree)
{
    /*TreeIterator it = tree.GetIterator();

	while(!it.IsDone())
	{
		Predicate predicate = it.GetCurrent()->predicate;
			
		if(it.GetCurrent()->nodeType == NodeType_Internal)
		{
	    	g_Predicates.push_back(predicate);
			++g_FeaturesInCount[predicate.GetFeatureId()];
		}

        it.MoveNext();
    }*/
}


void OptimizeTree(vector<Serp>::iterator begin, vector<Serp>::iterator end, DecisionTree *tree)
{
    for(TreeIterator it = tree->GetIterator(); !it.IsDone(); it.MoveNext())
        if(it.GetCurrent()->nodeType == NodeType_Leaf)
        {
            it.GetCurrent()->stat.count = 0;
            it.GetCurrent()->value = 0;
        }
            

    for(vector<Serp>::const_iterator it = begin; it != end; ++it)
        for(int i = 0; i < (int)it->size(); ++i)
        {
            DecisionTreeNode *node = tree->GetLeaf(it->at(i));
            ++(node->stat.count);
            node->value += it->at(i).gradient;
        }    

    for(TreeIterator it = tree->GetIterator(); !it.IsDone(); it.MoveNext())
        if(it.GetCurrent()->nodeType == NodeType_Leaf)
        {
            if(it.GetCurrent()->stat.count > 0)
                it.GetCurrent()->value /= it.GetCurrent()->stat.count;
        }
}

void BuildSeries(const TaskData& taskData,  
				 const MetaParameters& params, QualityChecker *checker, MetricsFunction metricsFunction,
				 vector<Serp>::iterator itLearnBegin, vector<Serp>::iterator itLearnEnd,
				 vector<Serp>::iterator itExamBegin, vector<Serp>::iterator itExamEnd,
                 double *minError, double *bestQuality, vector< pair<double, DecisionTree> > *series)
{	
	*minError = std::numeric_limits<double>::max();
	int bestLength = -1;	
    double alpha = params.GradientShrinkage;
    int learnPointsCount = 0;
    double metrics2 = 0;

    vector<Serp>::iterator it;

    for(vector<Serp>::iterator it = itLearnBegin; it != itLearnEnd; ++it)        
        for(int i = 0; i < (int)it->size(); ++i)
        {
            it->at(i).regression = 0;            
            ++learnPointsCount;
        }    

    for(vector<Serp>::iterator it = itExamBegin; it != itExamEnd; ++it)        
        for(int i = 0; i < (int)it->size(); ++i)
        {
            it->at(i).regression = 0;                       
        }    

    vector<double> gradient(learnPointsCount, 0);
    vector<double> p(learnPointsCount, 0);

    int prevLeafs = 0;

    for(int treesCount = 0; treesCount < params.TreesTotal; ++treesCount)
    {
		checker->CalculateGradient(itLearnBegin, itLearnEnd);

        it = itLearnBegin;
        int pos = 0;
        while(it != itLearnEnd)
        {
            for(int i = 0; i < (int)it->size(); ++i)	
            {
                gradient[pos] = it->at(i).gradient;
                ++pos;
            }
            
            ++it;
        }        

        double beta = 0;

        for(int i = 0; i < (int)learnPointsCount; ++i)        
            p[i] = (1 - beta) * gradient[i] + beta * p[i];        

        it = itLearnBegin;
        pos = 0;
        while(it != itLearnEnd)
        {
            for(int i = 0; i < (int)it->size(); ++i)	
            {
                it->at(i).gradient = p[pos];
                ++pos;
            }
            
            ++it;
        }
        
		double learnError = checker->GetQuality(*series, itLearnBegin, itLearnEnd);
		double examError = checker->GetQuality(*series, itExamBegin, itExamEnd);

        metrics2 = 0.95 * metrics2 + 0.05 * metricsFunction(itExamBegin, itExamEnd);
		
        int treeLeafs = (series->empty() ? 0 : series->back().second.GetLeafsCount());
    
		cout << treesCount << "\t" << prevLeafs << "\t" << treeLeafs << "\t" << learnError << "\t" << examError << "\t" << metrics2;

        vector<Point*> randomSubset;
        vector<bool> allowedFeatures(itLearnBegin->at(0).x.size());
        GetRandomSubset(itLearnBegin, itLearnEnd, params.BaggingPart, &randomSubset);
        GetRandomSubspace(params.RSMPart, &allowedFeatures);

        /*int f[] = {32,5,11,11,11,8,3,22,6,11,76,7,58,85,24,93,45,36,29,0,27,0,27,4,22,24,0,0,0,18,20,0,6,0,16,22,2,8,1,20,15,0,8,10,12,20,14,39,25,50,24,5,20,13,29,12,4,34,14,29,18,1,22,15,20,0,0,16,10,0,25,8,41,18,26,35,0,20,10,36,13,1,29,20,23,26,0,37,9,9,17,1,24,11,12,0,0,0,0,0,27,6,13,6,21,28,3,64,16,30,32,36,38,34,27,25,34,17,19,12,27,36,24,8,17,34,77,57,101,135,77,66,84,0,40,19,0,0};

        for(int i = 0; i < 136; ++i)
            allowedFeatures[i] = (f[i] > params.Features);*/

        DecisionTreeNode *root = BuildNormalTree(taskData, randomSubset, params.TreeHeight, allowedFeatures, 0);		
        //DecisionTreeNode *root = BuildMatrixTree(taskData, randomSubset, params.TreeHeight, allowedFeatures);		
		DecisionTree tree(root);
        prevLeafs =  tree.GetLeafsCount();
		//tree = PruneTree(tree, 5);
        //OptimizeTree(itLearnBegin, itLearnEnd, &tree);

        if(!series->empty())
            if(tree.EqualStructure(series->back().second))
                cout << " EQUAL ";

		cout << endl;		

        UpdateStatistics(tree);
        
		series->push_back(make_pair(alpha, tree));

        for(vector<Serp>::iterator it = itLearnBegin; it != itLearnEnd; ++it)        
            for(int i = 0; i < (int)it->size(); ++i)       
			    it->at(i).regression += alpha * tree.GetValue(it->at(i));        

        for(vector<Serp>::iterator it = itExamBegin; it != itExamEnd; ++it)        
            for(int i = 0; i < (int)it->size(); ++i)       
			    it->at(i).regression += alpha * tree.GetValue(it->at(i));        

		if(examError < *minError)
		{
			*minError = examError;
            *bestQuality = metrics2;
			bestLength = series->size() - 1;
		}
		
        if(treesCount - bestLength >= params.BoostingDelay)		
        {
            while((int)series->size() > bestLength)
				series->pop_back();

            /*treesCount = bestLength;

            it = itLearnBegin;
            while(it != itLearnEnd)
            {
                for(int i = 0; i < (int)it->size(); ++i)       
			        it->at(i).regression =  GetSeriesValue(*series, it->at(i));
                ++it;
            }

            if(alpha > 0.001)
                alpha /= 2;
            else
                break;*/

            break;
        }
    }
}

void SortData(vector<Serp> *serps)
{
	for(int i = 0; i < (int)serps->size(); ++i)
	{
		sort(serps->at(i).rbegin(), serps->at(i).rend(), PointComparer);
	}
}

void RandomShrinkSerps(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd, double probability)
{
    for(vector<Serp>::iterator it = itBegin; it != itEnd; ++it)
    {
        int pos = 0;

        for(int i = 0; i < (int)it->size(); ++i)
        {
            if((rand() % 1024) < (1024 * probability))
            {
                it->at(pos) = it->at(i);
                ++pos;
            }
        }
        
        it->resize(pos);
    }	
}

void ShrinkSerps(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd, int count)
{
    for(vector<Serp>::iterator it = itBegin; it != itEnd; ++it)
    {
        if((int)it->size() > count)
            it->resize(count);       
    }	
}

vector<MetaParameters> GetMetaParameters()
{
    vector<MetaParameters> paramsSet;
    
    MetaParameters params;  // по умолчанию
	params.BoostingDelay = 10;
	params.GradientShrinkage = 0.1;
	params.TreeHeight = 1;
	params.TreesTotal = 500;
	params.BaggingPart = 0.2;
    params.ThresholdsCount = 10;
    params.RSMPart = 1;

    for(params.TreeHeight = 4; params.TreeHeight <=8; ++params.TreeHeight)
        for(params.GradientShrinkage = 0.02; params.GradientShrinkage < 0.025; params.GradientShrinkage += 0.01)
            for(params.BaggingPart = 0.1; params.BaggingPart < 0.15; params.BaggingPart += 0.1)
                for(params.RSMPart = 1; params.RSMPart < 1.1; params.RSMPart += 0.5)
                    for(params.Features = 0; params.Features <= 5; params.Features += 10)
                    {
                        paramsSet.push_back(params);
                    }

    return paramsSet;
}

void LearnAndTest(DataSetParams dataSetParams, MetaParameters params, MetaResults *results,
                  const TaskData& taskData,
                  vector<Serp>::iterator itLearnBegin, vector<Serp>::iterator itLearnEnd,
                  vector<Serp>::iterator itExamBegin, vector<Serp>::iterator itExamEnd,
                  vector< pair<double, DecisionTree> > *series)
{
	time_t beginTime = clock();

    series->clear();
	BuildSeries(taskData, params, dataSetParams.checker ,dataSetParams.Metrics,
                itLearnBegin, itLearnEnd,
                itExamBegin, itExamEnd,		
                &results->Error, &results->Quality, series);

    time_t endTime = clock();
    results->Time = (endTime - beginTime) / CLOCKS_PER_SEC;
    results->SeriesLength = series->size();     	

	/*cout << endl;

	for(int i = 0; i < (int)g_FeaturesInCount.size(); ++i)
	{
		cout << i << " " << g_FeaturesInCount[i] << "\t";
	}

	cout << endl;

	for(int i = 0; i < (int)g_Predicates.size(); ++i)
	{
		cout << g_Predicates[i].ToString() << endl;
	}*/

	//int y;
	//cin >> y;
}

vector<MetaParameters> GetMetaParameters1()
{
    /*MetaParameters params;
	params.BoostingDelay = 5000;
	params.GradientShrinkage = 0.0016;
	params.TreeHeight = 4;
	params.TreesTotal = 500;
	params.BaggingPart = 0.3;
    params.ThresholdsCount = 10;
    params.RSMPart = 1;
    params.Features = 0;*/

    MetaParameters params;
	params.BoostingDelay = 50;
	params.GradientShrinkage = 0.02;
	params.TreeHeight = 7;
	params.TreesTotal = 1000;
	params.BaggingPart = 0.1;
    params.ThresholdsCount = 10;
    params.RSMPart = 1;
    params.Features = 0;

    return vector<MetaParameters>(1, params);
}

vector<DataSetParams> GetDataSetParams()
{
    vector<DataSetParams> params(3);
 
    params[0].LearnFile = "c:\\dataset1\\learn.txt";
    params[0].TaskFile = "c:\\dataset1\\test.txt";
    params[0].MarkedTaskFile = "c:\\dataset1\\test_with_ranks.txt";    
    params[0].ParamsFile = "c:\\dataset1\\params.txt";
    params[0].ResultsFile = "c:\\dataset1\\results.txt";
    params[0].checker = new MSE(WeightType_Constant);
    params[0].Metrics = GetNDCG10;
    params[0].LearnSize = 8000;
    params[0].ExamSize = 2000;
    params[0].SerpShrinkage = 1;

    params[1].LearnFile = "c:\\dataset2\\learn.txt";
    params[1].TaskFile = "c:\\dataset2\\test.txt";
    params[1].MarkedTaskFile = "c:\\dataset2\\test_with_ranks.txt";    
    params[1].ParamsFile = "c:\\dataset2\\params.txt";
    params[1].ResultsFile = "c:\\dataset2\\results.txt";
    params[1].checker = new LogFunctional(false);
    params[1].Metrics = GetKendallCoefficient;
    params[1].LearnSize = 620;
    params[1].ExamSize = 5;
    params[1].SerpShrinkage = 0.2;

    params[2].LearnFile = "c:\\dataset1\\learn.txt";
    params[2].TaskFile = "c:\\dataset1\\test.txt";
    params[2].MarkedTaskFile = "c:\\rank11\\test_with_ranks.txt";    
    params[2].ParamsFile = "c:\\rank11\\params.txt";
    params[2].ResultsFile = "c:\\rank11\\results.txt";
    params[2].checker = new MSE(WeightType_Constant);//new LogFunctional(true); //
    params[2].Metrics = GetNDCG10;
    params[2].LearnSize = 1000;
    params[2].ExamSize = 1000;
    params[2].SerpShrinkage = 1;

    return params;
}

void ProcessData(DataSetParams dataSetParams, const vector<MetaParameters>& paramsSet, vector< pair<double, DecisionTree> > *series)
{
    int learnSize = dataSetParams.LearnSize;
	int examSize = dataSetParams.ExamSize;

	vector<Serp> allSerps;
    GetData(dataSetParams.LearnFile, &allSerps, learnSize + examSize);
    RandomShrinkSerps(allSerps.begin(), allSerps.end(), dataSetParams.SerpShrinkage);
     
    SortData(&allSerps);	   
    //ShrinkSerps(allSerps.begin(), allSerps.begin() + learnSize , 100);  
    
    TaskData taskData;
    taskData.Calculate(allSerps.begin(), allSerps.begin() + learnSize, 10);
    
    ofstream ofs(dataSetParams.ResultsFile.c_str());

    for(int i = 0; i < (int)paramsSet.size(); ++i)
    {
        MetaResults results;
        LearnAndTest(dataSetParams, paramsSet[i], &results, taskData,
            allSerps.begin(), allSerps.begin() + learnSize,
            allSerps.begin() + learnSize, allSerps.begin() + learnSize + examSize,
            series);
        
        ofs << results.Error << "\t" << results.Quality << "\t" << results.SeriesLength << "\t" << results.Time << endl;        
    }    

    ofs.close();
}

int main()
{
	g_FeaturesInCount.resize(138);
    srand(123);   

    DataSetParams dataSetParams = GetDataSetParams()[0];

    //vector<MetaParameters> paramsSet = GetMetaParameters("c:\\dataset1\\params.txt");
    //vector<MetaParameters> paramsSet = GetMetaParameters();
    vector<MetaParameters> paramsSet = GetMetaParameters1();

    PrintMetaParameters(dataSetParams.ParamsFile, paramsSet);  

    vector< pair<double, DecisionTree> > series;
    ProcessData(dataSetParams, paramsSet, &series);   

    //WriteTestResults(dataSetParams.TaskFile, dataSetParams.MarkedTaskFile, series);

    return 0;
}
