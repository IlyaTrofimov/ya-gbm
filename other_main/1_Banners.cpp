#include "General.hpp"
#include "TaskData.hpp"
#include "QualityChecker.hpp"
#include "CPC_Optimizer.hpp"
#include "Metrics.hpp"
#include "DecisionTree.hpp"
#include "TreeBuilder.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <limits>

using std::ifstream;
using std::string;

Point ParsePoint(const string& line)
{
    int pos = 0;
    int count = 0;

    pos = line.find('\t', pos + 1);    

    Point point;
    point.x.resize(4);

    while(pos != -1)
    {
        if(count == 2)
            point.x[0] = atoi(line.c_str() + pos + 1) /(double)10000 ;     // PCTR
        else if(count == 4)
            point.x[1] = atoi(line.c_str() + pos + 1) /(double)10000;      // BID
        else if(count == 3)
            point.x[3] = atoi(line.c_str() + pos + 1) /(double)100;        // PREMIUM_RANK
        
        //else if(count == 5)
        //   point.x[3] = atoi(line.c_str() + pos + 1);      // PHRASE_ID

        pos = line.find('\t', pos + 1);
        count ++;
    }

    point.x[2] = point.x[0] * point.x[1];
    point.y = point.x[2];

    return point;
}

void LoadBannerData(const string& fileName, vector<Serp> *groups)
{
    ifstream dataStream(fileName.c_str());
    Serp group;

    string line;
    std::getline(dataStream, line);

    do
    { 
        if(line.substr(0, 6) == "BANNER")
        {
            Point point = ParsePoint(line);
            if(point.x[2] > 0)
                group.push_back(point);
        }
        else if(!group.empty())
        {
            groups->push_back(group);
            group.clear();
        }

        std::getline(dataStream, line);
    }
    while(dataStream.good());
}

class DecisionTree;

void BuildSeries_B(const TaskData& taskData,  
				 const MetaParameters& params, QualityChecker *checker, MetricsFunction metricsFunction,
				 SerpIterator itLearnBegin, SerpIterator itLearnEnd,
				 SerpIterator itExamBegin, SerpIterator itExamEnd,
                 double *minError, double *bestQuality, TreeSeries *series)
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
            it->at(i).regression = 1;            
            ++learnPointsCount;
        }    

    for(vector<Serp>::iterator it = itExamBegin; it != itExamEnd; ++it)        
        for(int i = 0; i < (int)it->size(); ++i)
        {
            it->at(i).regression = 1;                       
        }    

    vector<double> gradient(learnPointsCount, 0);
    vector<double> p(learnPointsCount, 0);
    double hits = 0;

    int prevLeafs = 0;

    for(int treesCount = 0; treesCount < params.TreesTotal; ++treesCount)
    {
		double learnError = checker->GetQuality(*series, itLearnBegin, itLearnEnd);
		double examError = checker->GetQuality(*series, itExamBegin, itExamEnd);
        hits = GetHits(itLearnBegin, itLearnEnd);
        //metrics2 = 0.5 * metrics2 + 0.5 * metricsFunction(itExamBegin, itExamEnd);
        metrics2 = metricsFunction(itExamBegin, itExamEnd);
        int treeLeafs = (series->empty() ? 0 : series->back().second.GetLeafsCount());
    
		cout << treesCount << "\t" << prevLeafs << "\t" << treeLeafs << "\t" << examError << "\t" << metrics2 << "\t" << hits << endl;

        if(hits < 1.2)
        { 
            cout << "Mon$ ";
            checker->CalculateGradient(itLearnBegin, itLearnEnd);
        }
        else
        {
            cout << "Hits ";
            CalculateHitsGradient(itLearnBegin, itLearnEnd);
        }

        vector<Point*> randomSubset;
        vector<bool> allowedFeatures(itLearnBegin->at(0).x.size());
        GetRandomSubset(itLearnBegin, itLearnEnd, params.BaggingPart, &randomSubset);
        GetRandomSubspace(params.RSMPart, &allowedFeatures);

        allowedFeatures[0] = false;
        allowedFeatures[1] = false;
        allowedFeatures[2] = true;
        allowedFeatures[3] = false;

        DecisionTreeNode *root = BuildNormalTree(taskData, randomSubset, params.TreeHeight, allowedFeatures, 0);
        //DecisionTreeNode *root = BuildMatrixTree(taskData, randomSubset, params.TreeHeight, allowedFeatures);
		DecisionTree tree(root);

        prevLeafs =  tree.GetLeafsCount();
		//tree = PruneTree(tree, 5);
        //OptimizeTree(itLearnBegin, itLearnEnd, &tree);

        if(!series->empty())
            if(tree.EqualStructure(series->back().second))
                cout << " EQUAL ";

		//cout << endl;
        
		series->push_back(make_pair(alpha, tree));

        ofstream ofs1("C:\\Banners\\regression.arff");
        ofstream ofs2("C:\\Banners\\gradient.arff");

        ofs1 << "@RELATION Banners" << endl;
        ofs1 << "@ATTRIBUTE cpm NUMERIC" << endl;
        ofs1 << "@ATTRIBUTE regression NUMERIC" << endl;
        ofs1 << "@DATA" << endl;

        ofs2 << "@RELATION Banners" << endl;
        ofs2 << "@ATTRIBUTE cpm NUMERIC" << endl;
        ofs2 << "@ATTRIBUTE gradient NUMERIC" << endl;
        ofs2 << "@DATA" << endl;

        for(SerpIterator it = itLearnBegin; it != itLearnEnd; ++it)
            for(int i = 0; i < (int)it->size(); ++i)
            {
                ofs1 << it->at(i).x[2] << "," << it->at(i).regression << endl;
                ofs2 << it->at(i).x[2] << "," << it->at(i).gradient << endl;
            }			    

        ofs1.close();
        ofs2.close();
    
        
        for(SerpIterator it = itLearnBegin; it != itLearnEnd; ++it)
            for(int i = 0; i < (int)it->size(); ++i)
			    it->at(i).regression += alpha * tree.GetValue(it->at(i));

        for(SerpIterator it = itExamBegin; it != itExamEnd; ++it)
            for(int i = 0; i < (int)it->size(); ++i)
			    it->at(i).regression += alpha * tree.GetValue(it->at(i));

		if(hits < 0.2 && metrics2 > *bestQuality)
		{
			*minError = examError;
            *bestQuality = metrics2;
			bestLength = series->size() - 1;
		}
		
        if(treesCount - bestLength >= params.BoostingDelay)
        {
            while((int)series->size() > bestLength)
				series->pop_back();
            break;
        }

        /*vector<Point> points(100);
        ofstream ofs("C:\\Banners\\regression.txt");

        for(int i = 0; i < (int)points.size(); ++i)
        {
            points[i].x.resize(4);
            points[i].x[2] = i * 10;            
            points[i].regression  = GetSeriesValue(*series, points[i]);
            ofs << points[i].x[2] << " " << points[i].regression << endl;
        }

        ofs.close();*/
        
    }
}

void SortData1(vector<Serp> *serps)
{
	for(int i = 0; i < (int)serps->size(); ++i)
	{
        sort(serps->at(i).rbegin(), serps->at(i).rend(), PointComparer);

        if(serps->at(i).size() > 10)
            serps->at(i).resize(10);

        std::random_shuffle(serps->at(i).begin(), serps->at(i).end());
	}
}

void RemoveData(vector<Serp> *serps)
{
	for(int i = 0; i < (int)serps->size(); ++i)
    {
        for(int j = 0; j < (int)serps->at(i).size(); ++j)
        {
            if(serps->at(i)[j].x[2] > 10000)
            {
                std::swap(serps->at(i)[j], serps->at(i).back());
                serps->at(i).pop_back();
            }
        }
    }
}

int main1()
{
    vector<Serp> groups;
    LoadBannerData("C:\\banner_unique_log.txt", &groups);
    //groups.resize(10000);

    SortData1(&groups);
    RemoveData(&groups);

    MetaParameters params;
	params.BoostingDelay = 1500;
	params.GradientShrinkage = 1;
	params.TreeHeight = 4;
	params.TreesTotal = 1500;
	params.BaggingPart = 0.5;
    params.ThresholdsCount = 10;
    params.RSMPart = 1;
    params.Features = 0;

    TaskData taskData;
    taskData.Calculate(groups.begin(), groups.end(), 30);

    QualityChecker *checker = new CPC_Optimizer();
    double minError;

    vector< pair<double, DecisionTree> > series;

    /*BuildSeries_B(taskData, params, checker, GetMoneyFraction, 
                groups.begin(), groups.end(),
                groups.begin(), groups.end(),
                &minError, &minError, &series);*/

    return 1;
}