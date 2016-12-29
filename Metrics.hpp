#ifndef _METRICS_HPP
#define _METRICS_HPP

double GetNDCG10_LP(vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd);
double GetNDCG(vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd);
double GetNDCG10(vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd);
double GetKendallCoefficient(vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd);

#endif /* _METRICS_HPP */