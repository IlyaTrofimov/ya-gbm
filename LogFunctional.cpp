#include <cmath>
#include <algorithm>

#include "QualityChecker.hpp"
#include "DecisionTree.hpp"

LogFunctional::LogFunctional(bool useRankWeight)
{
    _useRankWeight = useRankWeight;
}

double LogFunctional::GetQuality(const vector< pair<double, DecisionTree> >& series, 
								 vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd)
{	
	double quality = 0;

	vector<Serp>::const_iterator itSerp = itBegin;

	while(itSerp != itEnd)
	{
		quality += GetSerpQuality(series, *itSerp);
		++itSerp;
	}

	return quality;
}

double LogFunctional::GetWeight(const Point& point1, const Point& point2)
{
    if(_useRankWeight)
        return (point1.y - point2.y);
        //return SQUARE(point1.y - point2.y);
    else
        return 1;
}

double LogFunctional::GetSerpQuality(const vector< pair<double, DecisionTree> >& series, const vector<Point>& points)
{
	double quality = 0;
	vector<double> rank(points.size());
    vector<double> exp_s(points.size());

	for(int i = 0; i < (int)rank.size(); ++i)
    {
        rank[i] = points[i].regression;
        exp_s[i] = exp(rank[i]);
    }

	for(int i = 0; i < (int)rank.size(); ++i)
		for(int j = i + 1; j < (int)rank.size(); ++j)
        {
            double weight = GetWeight(points[i], points[j]);
            quality +=  weight * log(1 + exp_s[j] / exp_s[i]);
        }

	return quality;
}

void LogFunctional::CalculateGradient(SerpIterator itBegin, SerpIterator itEnd)
{
	for(SerpIterator it = itBegin; it != itEnd; ++it)
	{
        vector<double> exp_s(it->size());

        for(int i = 0; i < (int)it->size(); ++i)
            exp_s[i] = exp(it->at(i).regression);

		for(int k = 0; k < (int)it->size(); ++k) 
		{
			double gradient = 0;

			for(int j = 0; j < (int)it->size(); ++j)
			{
                double weight;

                if(j < k)
                    weight = GetWeight(it->at(j), it->at(k));
                else if( j > k)
                    weight = GetWeight(it->at(k), it->at(j));

                if(j < k)
					gradient += weight * exp_s[k] / (exp_s[j] + exp_s[k]);
				else if(j > k)
					gradient += -weight * exp_s[j] / (exp_s[j] + exp_s[k]);
			}

			it->at(k).gradient = -gradient;
		}
	}
}