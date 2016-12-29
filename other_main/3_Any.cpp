#include "General.hpp"
#include "FilesIO.hpp"
#include "MulticlassClassifier.hpp"
#include "TaskData.hpp"
#include "DataSetIterator.hpp"
#include "AlgorithmTester.hpp"

#include <fstream>
#include <algorithm>

vector<MetaParameters> GetMetaParameters_Any();

int main4444()
{
    srand(123);
    vector<MetaParameters> paramsSet = GetMetaParameters_Any();

    paramsSet[0].TestPart = 0;
    paramsSet[0].StopByTestError = false;
    paramsSet[0].TreesTotal = 200;

    DataSet dataSet;
    GetData("C:\\lyrics_data_2010\\learn-new.txt", &dataSet, 2000);

    MulticlassClassifier classifier = TrainAlgorithm(paramsSet[0], &dataSet);

    WriteTestResultsLyrics("C:\\lyrics_data_2010\\test-new.txt", "C:\\lyrics_data_2010\\answers-new.txt", classifier);

    return 0;
}

int main111()
{ 
    srand(123);
    vector<MetaParameters> paramsSet = GetMetaParameters_Any();

    DataSet dataSet;
    GetData("C:\\lyrics_data_2010\\learn-new.txt", &dataSet, 2000);
    //GetDataARFF("C:\\MLL\\data\\UCI_Liver_Disorders.arff", &dataSet);
    std::random_shuffle(dataSet.begin(), dataSet.end());

    ofstream ofs("C:\\MLL\\results.txt");
    PrintMetaParametersHeader(&ofs);
    ofs << "\t";
    PrintTestResultsHeader(&ofs);
    ofs << "\n";

    for(int i = 0; i < (int)paramsSet.size(); ++i)
    {
        int passes = 10;
        TestResults results= TestAlgorithm(&dataSet, paramsSet[i], passes);
        PrintMetaParameters(paramsSet[i], &ofs);
        ofs << "\t";
        PrintTestResults(results, &ofs);
        ofs << "\n";
    }

    ofs.close();

    return 0;
}

vector<MetaParameters> GetMetaParameters_Any()
{  
    //
    // Значения по умолчанию
    //
    MetaParameters params;
    params.BoostingDelay = 100;
    params.GradientShrinkage = 0.16;
    params.TreeHeight = 3;
    params.TreesTotal = 500;
    params.BaggingPart = 0.5;
    params.ThresholdsCount = 10;
    params.RSMPart = 1;
    params.Features = 0;
    params.StopByTestError = true;
    params.TestPart = 0.3;
    params.FilterErrors = 1;
    params.WeightTrimming = 0.1;
    params.MinLeafSize = 1;

    vector<MetaParameters> paramsSet;

    for(params.TreeHeight = 8; params.TreeHeight <=8; ++params.TreeHeight)
        for(params.GradientShrinkage = 0.05; params.GradientShrinkage < 0.2; params.GradientShrinkage += 2.01)
            for(params.BaggingPart = 0.5; params.BaggingPart < 0.6; params.BaggingPart += 0.5)
                for(params.RSMPart = 1; params.RSMPart < 1.05; params.RSMPart += 1)
                    for(params.Features = 0; params.Features  < 1; params.Features += 5)
                        for(params.NoiseDeletePart = 0.05; params.NoiseDeletePart < 0.20; params.NoiseDeletePart += 1)
                            for(params.FilterErrors = 0; params.FilterErrors < 1; params.FilterErrors += 1)
                                for(params.ThresholdsCount = 35; params.ThresholdsCount < 100; params.ThresholdsCount += 500)
                                    for(params.WeightTrimming = 0.2; params.WeightTrimming < 0.5; params.WeightTrimming += 1.1)
                                        for(int count = 0; count < 1; ++count)
                                            paramsSet.push_back(params);

    return paramsSet;
}

vector<MetaParameters> GetMetaParameters_Default2()
{  
    //
    // Значения по умолчанию
    //
    MetaParameters params;
    params.BoostingDelay = 100;
    params.GradientShrinkage = 0.16;
    params.TreeHeight = 3;
    params.TreesTotal = 500;
    params.BaggingPart = 0.5;
    params.ThresholdsCount = 10;
    params.RSMPart = 1;
    params.Features = 0;
    params.StopByTestError = true;
    params.TestPart = 0.3;
    params.FilterErrors = 1;
    params.WeightTrimming = 0.1;
    params.MinLeafSize = 1;
    params.TreeBuilder = TreeBuilder_FirstBest;
    params.LeafsCountMax = 2;

    vector<MetaParameters> paramsSet;

    for(params.LeafsCountMax = 2; params.TreeHeight <=16; ++params.TreeHeight *=2)
        for(params.GradientShrinkage = 0.01; params.GradientShrinkage < 0.2; params.GradientShrinkage *= 2)
            for(params.BaggingPart = 0.5; params.BaggingPart < 0.6; params.BaggingPart += 0.5)
                for(params.RSMPart = 1; params.RSMPart < 1.05; params.RSMPart += 1)
                    for(params.Features = 0; params.Features  < 1; params.Features += 5)
                        for(params.FilterErrors = 0; params.FilterErrors < 1; params.FilterErrors += 1)
                            for(params.ThresholdsCount = 10; params.ThresholdsCount < 50; params.ThresholdsCount += 10)
                                for(params.WeightTrimming = 0.05; params.WeightTrimming < 0.4; params.WeightTrimming += 0.05)
                                    paramsSet.push_back(params);

    return paramsSet;
}