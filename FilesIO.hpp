#ifndef _FILESIO_HPP_
#define _FILESIO_HPP_

#include "General.hpp"

class DecisionTree;
class MulticlassClassifier;
//class ofstream;

void GetData(const string& fileName, vector<Serp> *allSerps, int serpsMax);
void GetDataARFF(const string& fileName, vector<Serp> *allSerps);
void GetDataFactorLog(const string& fileName, double part, int serpsMax, DataSetFilter filter, int labelField, vector<Serp> *allSerps);

void WriteTestResults(const string& testFileName, const string& markedTestfileName, const vector< pair<double, DecisionTree> >& series, bool writeEnding);
void WriteTestResultsLyrics(const string& testFileName, const string& markedTestfileName, const MulticlassClassifier& classifier);

vector<MetaParameters> GetMetaParameters(const string& fileName);
void PrintMetaParameters(const MetaParameters& params, ofstream *ofs);
void PrintMetaParameters(const string& fileName, const vector<MetaParameters>& paramsSet);
void PrintTestResults(const TestResults& results, ofstream *ofs);
void PrintImportance(const vector<double>& importance, const MetaParameters& params, const vector<string>& featuresNames, ofstream *ofs);

void PrintMetaParametersHeader(ofstream *ofs);
void PrintTestResultsHeader(ofstream *ofs);

void WriteHeaders(const Log* log);
void WriteExperimentInfo(const MetaParameters& params, const TestResults& results, Log* log);
void WriteLine(const string& s, Log* log);

vector<string> Split(const string& line, char separator);

#endif /* _FILESIO_HPP_ */
