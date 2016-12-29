#include <stdlib.h>

#include "General.hpp"
#include "DecisionTree.hpp"
#include "MulticlassClassifier.hpp"
#include "Regression.hpp"
#include "QualityChecker.hpp"

vector<string> Split(const string& line, char separator)
{
    vector<string> parts;
    int prev_position = 0;

    for(int i = 0; i <= (int)line.length(); ++i)
    {
        if(i == (int)line.length() || line.at(i) == separator)
        {
            parts.push_back(line.substr(prev_position, i - prev_position));
            prev_position = i + 1;
        }
    }

    return parts; 
}

void GetDataARFF(const string& fileName, vector<Serp> *allSerps)
{
    ifstream ifs(fileName.c_str());
    Serp serp;

    vector<double> f;
    
    int qid = -1;
    int rank = -1;
    int count = 0;

    while(ifs.good())
    {
        string s;
        ifs >> s;

        if(s == "@DATA")
            break;
    }

    while(ifs.good())
    {
        string s;
        ifs >> s;

        if(s == "%" || s.length() == 0)
            continue;

        vector<string> parts = Split(s, ',');

        Point point;

        for(int i = 0; i < (int)parts.size(); ++i)
        {
            double value = atoi(parts[i].c_str());

            if(i < (int)parts.size() - 1)
                point.x.push_back(value);
            else
                point.y = value;
        }

        serp.push_back(point);
        allSerps->push_back(serp);
        serp.clear();
    }

    ifs.close();
}

void GetDataFactorLog(const string& fileName, double part, int serpsMax, DataSetFilter filter, int labelField, vector<Serp> *allSerps)
{
    ifstream ifs(fileName.c_str());

    while(ifs.good() && ((allSerps->size() < serpsMax) || serpsMax == -1))
    {
        string s;
        std::getline(ifs, s);

        if(s.length() == 0)
            continue;

        //if(rand() % 1024 < part * 1024)
        {
            vector<string> parts = Split(s, ' ');


            allSerps->resize(allSerps->size() + 1);
            allSerps->back().resize(1);

            Point *point = &(allSerps->back()[0]);

            for(int i = 0; i < (int)parts.size(); ++i)
            {
                double value = atof(parts[i].c_str());

                if(i == labelField)
                    point->y = value;
                else {
                    point->x.push_back(value);
                    point->present.push_back(true);
                }
            }

            if(filter != NULL && !filter(point))
                allSerps->pop_back();
        }
    }

    ifs.close();
}

void GetData(const string& fileName, vector<Serp> *allSerps, int serpsMax)
{
    ifstream ifs(fileName.c_str());
    Serp serp;

    vector<double> f;
    
    int qid = -1;
    int rank = -1;
    int count = 0;

    while(ifs.good() && (int)allSerps->size() < serpsMax)
    {
        string s;
        ifs >> s;
        
        if(s.find(":", 0) == -1)
        {
            if(count % 1000 == 0)
                cout << "lines_count = " << count << " serps_count " << allSerps->size() << endl;

            if(!f.empty())
            {   
		Point newPoint;
		newPoint.x = f;
		newPoint.y = rank;
				
                serp.push_back(newPoint);
                f.clear();
                count++;
            }

            rank = atoi(s.c_str());
        }
        else if(s.substr(0, 3) == "qid")
        {
            int new_qid = atoi(s.c_str() + 4);

            if(new_qid != qid)
            {
                qid = new_qid;

                if(!serp.empty())
                {
                    allSerps->push_back(serp);
                    serp.clear();
                }                
            }
        }
        else
        {
            int pos = s.find(":", 0);
            double value = atof(s.c_str() + pos + 1);
            f.push_back(value);
        } 
    }

    if(!serp.empty())
    {
        allSerps->push_back(serp);
        serp.clear();
    }

    ifs.close();
}

void MergeSerps(const vector<Serp>& serps, int fromIndex, int toIndex, vector<Point> *points)
{
	for(int i = fromIndex; i < toIndex && i < (int)serps.size(); ++i)
	{
		points->insert(points->end(), serps[i].begin(), serps[i].end());
	}
}

void WriteTestResults(const string& testFileName, const string& markedTestfileName,
                      const vector< pair<double, DecisionTree> >& series, bool writeEnding)
{
	vector<Serp> allSerps;
	GetData(testFileName.c_str(), &allSerps, 100000);	
	
	vector<Point> points;
	MergeSerps(allSerps, 0, 100000, &points);

	ifstream ifs(testFileName.c_str());
    ofstream ofs(markedTestfileName.c_str());

	for(int i = 0; i < (int)points.size(); ++i)
	{
		string line;
		std::getline(ifs, line);
		string ending = line.substr(1, line.length() - 1);
		
		double value = GetSeriesValue(series, points[i]);

        ofs << value;

        if(writeEnding)
            ofs << value;

        ofs << endl;
	}
}

void WriteTestResultsLyrics(const string& testFileName, const string& markedTestfileName, const MulticlassClassifier& classifier)
{
    vector<Serp> allSerps;
	GetData(testFileName.c_str(), &allSerps, 100000);
	
	vector<Point> points;
	MergeSerps(allSerps, 0, 100000, &points);
	
    ofstream ofs(markedTestfileName.c_str());

	for(int i = 0; i < (int)points.size(); ++i)
    {
        int value = classifier.GetClass(points[i]);

        vector<string> authors(10);
        authors[0] = "blok";
        authors[1] = "brodsky";
        authors[2] = "derzhavin";
        authors[3] = "esenin";
        authors[4] = "lermontov";
        authors[5] = "mandelshtam";
        authors[6] = "pasternak";
        authors[7] = "pushkin";
        authors[8] = "tsvetaeva";
        authors[9] = "tutchev";

        ofs << authors[value] << endl;
	}
}

/*vector<MetaParameters> GetMetaParameters(const string& fileName)
{
    ifstream ifs(fileName.c_str());
    vector<MetaParameters> paramsSet;

    while(ifs.good())
    {
        string line;
        std::getline(ifs, line);
        MetaParameters params;

        int pos = 0;        

        while(pos != -1)
        {
            int pos2 = line.find(" ", pos);
            string s = line.substr(pos, pos2 - pos);
            int pos3 = s.find("=", 0);

            string field = line.substr(pos, pos3);

            if(field == "BoostingDelay")
                params.BoostingDelay = atoi(s.c_str() + pos3 + 1);
            else if(field == "GradientShrinkage")
                params.GradientShrinkage = atof(s.c_str() + pos3 + 1);
            else if(field == "TreeHeight")
                params.TreeHeight = atoi(s.c_str() + pos3 + 1);
            else if(field == "TreesTotal")
                params.TreesTotal = atoi(s.c_str() + pos3 + 1);
            else if(field == "BaggingPart")
                params.BaggingPart = atof(s.c_str() + pos3 + 1);
            else if(field == "ThresholdsCount")
                params.ThresholdsCount = atoi(s.c_str() + pos3 + 1);

            if(pos2 == -1)
                break;

            pos = pos2 + 1;
        }

        paramsSet.push_back(params);
    }

    return paramsSet;
}*/

void PrintMetaParametersHeader(ofstream *ofs)
{
    (*ofs) << "BoostingDelay" << "\t" <<  "GradientShrinkage" << "\t" << "TreeHeight" << "\t"
         << "TreesTotal" << "\t" << "BaggingPart" << "\t" << "ThresholdsCount" << "\t"
         << "RSMPart" <<  "\t" << "Features" << "\t" << "NoiseDeletePart" << "\t"
         << "StopByTestError" << "\t" << "TestPart" << "\t" << "FilterErrors" << "\t"
         << "WeightTrimming" << "\t" << "AllowedFeatures";
}

string ToString(const vector<bool>& allowedFeatures)
{
    string s("0", allowedFeatures.size());

    for(int i = 0; i < allowedFeatures.size(); ++i)
        s[i] = (allowedFeatures[i] ? '1' : '0');

    return s;
}

void PrintMetaParameters(const MetaParameters& params, ofstream *ofs)
{
    (*ofs) << params.BoostingDelay << "\t" <<  params.GradientShrinkage << "\t" << params.TreeHeight << "\t"
         << params.TreesTotal << "\t" << params.BaggingPart << "\t" << params.ThresholdsCount << "\t"
         << params.RSMPart <<  "\t" << params.Features << "\t" << params.NoiseDeletePart << "\t"
         << params.StopByTestError << "\t" << params.TestPart << "\t" << params.FilterErrors << "\t"
         << params.WeightTrimming << "\t" << ToString(params.AllowedFeatures);
}

void PrintMetaParameters(const string& fileName, const vector<MetaParameters>& paramsSet)
{
    ofstream ofs(fileName.c_str());

    for(int i = 0; i < (int)paramsSet.size(); ++i)
    {
        PrintMetaParameters(paramsSet[i], &ofs);
        ofs << endl;
    }

    ofs.close();
}

void PrintTestResultsHeader(ofstream *ofs)
{
    (*ofs) << "LearnError" << "\t" << "TestError" << "\t"
        << "LearnErrorRelStd" << "\t" << "TestErrorRelStd" << "\t"
        << "LearnQuality" << "\t" << "TestQuality" << "\t"
        << "Length" << "\t" << "Time";
}

void PrintTestResults(const TestResults& results, ofstream *ofs)
{
    (*ofs) << results.LearnError << "\t" << results.TestError << "\t"
        << results.LearnErrorRelStd << "\t" << results.TestErrorRelStd << "\t"
        << results.LearnQuality << "\t" << results.TestQuality << "\t"
        << results.Length << "\t" << results.Time;
}

void PrintImportance(const vector<double>& importance, const MetaParameters& params, const vector<string>& featuresNames, ofstream *ofs)
{
    for(int i = 0; i < importance.size(); ++i)
    {
        string name = (i < featuresNames.size() ? featuresNames[i] : "");
        int allowed = (i < params.AllowedFeatures.size() ? params.AllowedFeatures[i] : 1);

        (*ofs) << i << "\t" << name << "\t" << importance[i] << "\t" <<allowed << "\n";
    }
}

void PrintCorrelation(const vector< vector<ThresholdStat> >& stat, const vector<string>& featuresNames, ofstream *ofs)
{
    for(int featureId = 0; featureId < stat.size(); ++featureId)
    {
        double totalDispersion = 0;
        int totalCount = 0;

        for(int j = 0; j < stat[featureId].size(); ++j)
        {
            totalDispersion += stat[featureId][j].Dispersion;
            totalCount += stat[featureId][j].PointsCount;
        }

        totalDispersion /= totalCount;

        string name = (featureId < featuresNames.size() ? featuresNames[featureId] : "");
        (*ofs) << featureId << "\t" << name << "\t" << totalDispersion << "\n";
    }

    for(int featureId = 0; featureId < stat.size(); ++featureId)
    {
        (*ofs) << featureId << "\t" << featuresNames[featureId] << endl;

        for(int j = 0; j < stat[featureId].size(); ++j)
        {
            (*ofs) << PadRight(ToString((double)stat[featureId][j].Threshold), 10) 
                << stat[featureId][j].PointsCount 
                << "\t" << stat[featureId][j].AvgResponce << endl;
        }

        (*ofs) << endl;
    }
}

void WriteHeaders(const Log* log)
{
    ofstream ofs(log->ResultsFile.c_str());

    ofs << "ID\t";
    PrintMetaParametersHeader(&ofs);
    ofs << "\t";
    PrintTestResultsHeader(&ofs);
    ofs << "\n";

    ofs.close();
}

void WriteLine(const string& s, Log* log)
{
    ofstream ofsFeatures(log->FeauresFile.c_str(), std::ios::app);
    ofsFeatures << s << "\n";
    ofsFeatures.close();
}

void WriteExperimentInfo(const MetaParameters& params, const TestResults& results, Log* log)
{
    ofstream ofs(log->ResultsFile.c_str(), std::ios::app);
    ofstream ofsFeatures(log->FeauresFile.c_str(), std::ios::app);

    ofs << log->ExperimentId << "\t";
    PrintMetaParameters(params, &ofs);
    ofs << "\t";
    PrintTestResults(results, &ofs);
    ofs << "\n";

    ofsFeatures << "ID = " << log->ExperimentId << "\n";

    for(int m = 0; m < results.BaseLineMetrics.size(); ++m)
    {
        ofsFeatures << params.Metrics[m]->GetName() << ":" << endl;

        for(int b = 0; b < results.BaseLineMetrics[0].size(); ++b)
        {
            double baseLineMetrics = results.BaseLineMetrics[m][b];
            double metrics = results.Metrics[m];

            if(baseLineMetrics > 0)
            {
                double delta = (metrics - baseLineMetrics) / baseLineMetrics * 100;
                ofsFeatures << params.BaselineRegression[b]->GetName() << " delta = " << delta << "%" << endl;
            }
            else
                ofsFeatures << "baseline delta = ? " << endl;
        }

        ofsFeatures << endl;
    }

    PrintImportance(results.Importance, params, log->FeaturesName, &ofsFeatures);
    ofsFeatures << "\n";
    ofsFeatures << "\n";
    PrintCorrelation(results.Stat, log->FeaturesName, &ofsFeatures);
    ofsFeatures << "\n";

    log->ExperimentId++;

    ofs.close();
    ofsFeatures.close();
}
