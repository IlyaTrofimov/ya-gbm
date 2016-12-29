#include "General.hpp"
#include "FilesIO.hpp"
#include "MulticlassClassifier.hpp"
#include "TaskData.hpp"

#include <fstream>
#include <algorithm>

vector<MetaParameters> GetMetaParameters_Lyrics();

int main666()
{ 
    srand(123);
    vector<Serp> groups;

    MetaParameters params;
	params.BoostingDelay = 1;
	params.GradientShrinkage = 0.16;
	params.TreeHeight = 3;
	params.TreesTotal = 5;
	params.BaggingPart = 0.5;
    params.ThresholdsCount = 5;
    params.RSMPart = 1;
    params.Features = 0;

    //vector<MetaParameters> paramsSet(1, params);
    vector<MetaParameters> paramsSet = GetMetaParameters_Lyrics();
    PrintMetaParameters("C:\\lyrics_data_2010\\params.txt", paramsSet);

    /*int learnSize = 1718;
    int examSize = 0;   */
    int learnSize = 1300;
    int examSize = 418; 

    GetData("C:\\lyrics_data_2010\\learn-new.txt", &groups, learnSize + examSize);
    std::random_shuffle(groups.begin(), groups.end());

    TaskData taskData;
    taskData.Calculate(groups.begin(), groups.begin() + learnSize, 10);
   
    double minError;
    double bestQuality;

    MulticlassClassifier classifier = MulticlassClassifier();
    ofstream ofs("C:\\lyrics_data_2010\\results.txt");

    int folds = 10;
   
    for(int i = 0; i < (int)paramsSet.size(); ++i)
    {
        TestResults results;
        double error = 0;
        double quality = 0;
        time_t beginTime = clock();

        for(int j = 0; j < folds; j++)
        {
            std::random_shuffle(groups.begin(), groups.end());

            for(SerpIterator it = groups.begin(); it != groups.begin() + learnSize; ++it)
                for(int i = 0; i < (int)it->size(); ++i)
                    it->at(i).totalDisabled = false;

            MetaParameters p = paramsSet[i];
            //p.TreesTotal = 50;

            classifier.Learn(taskData, p,
                    groups.begin(), groups.begin() + learnSize,
                    groups.begin() + learnSize, groups.end(),
                    true, &results);

            for(SerpIterator it = groups.begin(); it != groups.begin() + learnSize; ++it)
                for(int i = 0; i < (int)it->size(); ++i)
                {
                    if(classifier.GetClass(it->at(i)) != (int)it->at(i).y)
                        it->at(i).totalDisabled = true;
                }

            classifier.Learn(taskData, paramsSet[i],
                    groups.begin(), groups.begin() + learnSize,
                    groups.begin() + learnSize, groups.end(),
                    false, &results);

            error += results.TestError;
            quality += results.TestQuality;
        }

        time_t endTime = clock();
        results.Time = (endTime - beginTime) / CLOCKS_PER_SEC;

        ofs << error / folds << "\t" << quality / folds << "\t" << results.Length << "\t" << results.Time << endl;

        vector<int> freq(taskData.GetFeaturesCount(), 0);
        classifier.GetFeaturesStat(&freq);

        ofstream ofs2("C:\\lyrics_data_2010\\features.txt");
        for(int i = 0; i < (int)freq.size(); ++i)
        {
            ofs2 << i + 1 << " " << freq[i] << "\n";
        }
        ofs2.close();
    }

    ofs.close();

    //WriteTestResults("C:\\lyrics_data_2010\\test.txt", "C:\\lyrics_data_2010\\answers.txt", classifier);

    return 1;
}

vector<MetaParameters> GetMetaParameters_Lyrics()
{  
    MetaParameters params;  // по умолчанию
	params.BoostingDelay = 50;
	params.GradientShrinkage = 0.16;
	params.TreeHeight = 3;
	params.TreesTotal = 500;
	params.BaggingPart = 0.5;
    params.ThresholdsCount = 10;
    params.RSMPart = 1;
    params.Features = 0;
    params.StopByTestError = true;

    vector<MetaParameters> paramsSet;

    for(params.TreeHeight = 3; params.TreeHeight <=3; ++params.TreeHeight)
        for(params.GradientShrinkage = 0.16; params.GradientShrinkage > 0.06; params.GradientShrinkage -= 0.01)
            for(params.BaggingPart = 0.5; params.BaggingPart < 0.6; params.BaggingPart += 0.5)
                for(params.RSMPart = 0.5; params.RSMPart < 0.6; params.RSMPart += 0.2)
                    for(params.Features = 0; params.Features  < 1; params.Features += 5)
                        for(params.NoiseDeletePart = 0.05; params.NoiseDeletePart < 0.20; params.NoiseDeletePart += 1)
                        {
                            for(int count = 0; count < 1; ++count)
                                paramsSet.push_back(params);
                        }

    return paramsSet;
}