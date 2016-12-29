#ifndef _ALGORITHM_TESTER_HPP
#define _ALGORITHM_TESTER_HPP

class IRegression;

TestResults TestAlgorithm(DataSet *dataSet, const MetaParameters& params, int passes);
TestResults TestRegression(DataSet *dataSet, const MetaParameters& params, int folds, int passes);
TestResults TestRegression2(DataSet *dataSet, const MetaParameters& params, double learnPart, double testPart);
MulticlassClassifier TrainAlgorithm(const MetaParameters& params, DataSet *dataSet);
void DelStrategy(const MetaParameters& params_in, int deleteCount, DataSet* dataSet, Log *log);

#endif /* _ALGORITHM_TESTER_HPP */