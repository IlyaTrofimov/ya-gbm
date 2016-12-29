#include <algorithm>
#include <cmath>

#include "General.hpp"
#include "DecisionTree.hpp"

using std::prev_permutation;
using std::next_permutation;

double GetP(const vector< pair<double, double> >& ranks)
{
    double sum_p = 0;

    for(int i = 0; i < (int)ranks.size(); ++i) 
        sum_p += (ranks[i].second + 0.01);

    double p = 1;

     for(int i = 0; i < (int)ranks.size(); ++i)
     {
        p *= (ranks[i].second + 0.01) / sum_p;
        sum_p -= (ranks[i].second + 0.01);
     }   

     return p;
}

double GetDCG(const vector< pair<double, double> >& ranks)
{      
    double dcg = 0;

    for(int i = 0; i < (int)ranks.size(); ++i)
        dcg += (pow(2, ranks[i].first) - 1) / log((double)2 + i);

    if(dcg < 0.01)
        return 1;

    return dcg;
}

double GetDCG_LP(const Serp& serp, int count)
{    
    vector< pair<double, double> > ranks(serp.size());

    for(int i = 0; i < (int)serp.size(); ++i)
    {
        // на начальных этапах много одинаковых рангов,
        // нужна рандомизация, т.к. строки уже упорядочены по релевантности.
        double delta = (rand() % 1024) / (double)1024000;

        ranks[i].first = serp[i].y;
        ranks[i].second = serp[i].regression + delta;
    }

    //std::sort(ranks.rbegin(), ranks.rend(), PairSecondCompare);
    if(ranks.size() > 5)
        ranks.resize(5);

    double dcg_lp = 0;

    while(next_permutation(ranks.rbegin(), ranks.rend(), PairSecondCompare))
    {
        dcg_lp += GetP(ranks) * GetDCG(ranks);
    };   

    return dcg_lp;
}

double GetDCG(const Serp& serp, int count)
{    
    vector< pair<double, double> > ranks(serp.size());

    for(int i = 0; i < (int)serp.size(); ++i)
    {
        // на начальных этапах много одинаковых рангов,
        // нужна рандомизация, т.к. строки уже упорядочены по релевантности.
        double delta = (rand() % 1024) / (double)1024000;

        ranks[i].first = serp[i].y;
        ranks[i].second = serp[i].regression + delta;
    }

    std::sort(ranks.rbegin(), ranks.rend(), PairSecondCompare);
    if(ranks.size() > count)
        ranks.resize(count);

    double dcg = GetDCG(ranks);
    return dcg;
}

double GetIdealDCG(const Serp& serp, int count)
{
    double idcg = 0;

    // Строки в серпе уже отсортированиы по убыванию релевантности
    for(int i = 0; i < (int)serp.size() && i < count; ++i)    
        idcg += (pow(2, serp[i].y) - 1) / log((double)2 + i);

    if(idcg < 0.01)
        return 1;

    return idcg;
}

double GetNDCG(vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd)
{  
    double sum = 0;

    for(vector<Serp>::const_iterator it = itBegin; it != itEnd; ++it)
    {
        sum += GetDCG(*it, 10000) / GetIdealDCG(*it, 10000);        
    }

    return sum;
}

double GetNDCG10_LP(vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd)
{  
    double sum = 0;

    for(vector<Serp>::const_iterator it = itBegin; it != itEnd; ++it)
    {
        sum += GetDCG_LP(*it, 10) / GetIdealDCG(*it, 10);        
    }

    return sum;
}

double GetNDCG10(vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd)
{  
    double sum = 0;

    for(vector<Serp>::const_iterator it = itBegin; it != itEnd; ++it)
    {
        sum += GetDCG(*it, 10) / GetIdealDCG(*it, 10);        
    }

    return sum / (itEnd - itBegin);
}

double GetKendallCoefficient(vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd)
{	
	double quality = 0;

	for(vector<Serp>::const_iterator itSerp = itBegin; itSerp != itEnd; ++itSerp)
	{		
		int concordant = 0;
		int nonconcordant = 0;

		for(int k = 0; k < (int)itSerp->size(); ++k)	
		{
			double gradient = 0;

			for(int j = k + 1; j < (int)itSerp->size(); ++j)
			{
				if(itSerp->at(j).regression < itSerp->at(k).regression)
					++concordant;
				else
					++nonconcordant;
			}
		}

		int count = (int)itSerp->size();
		double kendall_coefficient = (double)(concordant - nonconcordant) / (count * (count - 1) / 2);

		quality += kendall_coefficient;
	}

	return quality / (itEnd - itBegin);
}