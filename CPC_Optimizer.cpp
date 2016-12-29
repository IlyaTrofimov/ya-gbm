#include "General.hpp"
#include "CPC_Optimizer.hpp"
#include "General.hpp"

#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>

using std::ifstream;
using std::string;

typedef bool (*PointerPointComparer)(const Point*, const Point*);

bool ScoreComparer(const Point* point1, const Point* point2)
{
    return point1->regression < point2->regression; 
}

bool CPMComparer(const Point* point1, const Point* point2)
{
    return point1->x[2] < point2->x[2]; 
}

vector<const Point*> PickPremiumBanners(const Serp& group, int count, PointerPointComparer comparer, bool useZeroThreshold)
{
    vector<const Point*> bestBanners;
    vector<const Point*> bannerPointers;

    for(int i = 0; i < group.size(); ++i)    
        bannerPointers.push_back(&group[i]); 

    std::sort(bannerPointers.rbegin(), bannerPointers.rend(), comparer);

    for(int i = 0; i < bannerPointers.size(); i++)
    {
        if((int)bestBanners.size() == count)
            break;

        if(useZeroThreshold && bannerPointers[i]->regression > 0)
            bestBanners.push_back(bannerPointers[i]);
        else if(!useZeroThreshold)
            bestBanners.push_back(bannerPointers[i]);
    }

    return bestBanners;
}

double GetMoney(const vector<const Point*>& premiumBanners)
{
    double money = 0;

    for(int i = 0; i < (int)premiumBanners.size(); i++)
    {
        double pctr = premiumBanners[i]->x[0];
        double cost = premiumBanners[i]->x[1];

        money += cost * pctr;
    }

    return money;
}

double GetMoney(ConstSerpIterator itBegin, ConstSerpIterator itEnd)
{
    double money = 0;

    for(vector<Serp>::const_iterator it = itBegin; it != itEnd; ++it)    
    {
        vector<const Point*> premiumBanners = PickPremiumBanners(*it, 3, ScoreComparer, false);
        money += GetMoney(premiumBanners);
    }

    return money;
}

double GetHits(ConstSerpIterator itBegin, ConstSerpIterator itEnd)
{
    double hits = 0;

    for(vector<Serp>::const_iterator it = itBegin; it != itEnd; ++it)    
    {
        vector<const Point*> premiumBanners = PickPremiumBanners(*it, 3, ScoreComparer, false);
        if(premiumBanners.size() > 0)
            ++hits;
    }

    return hits / (itEnd - itBegin);
}


double CPC_Optimizer::GetQuality(const std::vector<pair<double, DecisionTree> > &series, vector<Serp>::const_iterator itBegin, vector<Serp>::const_iterator itEnd)
{
    return GetMoney(itBegin, itEnd);
}

double ToP(double score)
{
    if (score > 10)
        return score;
    else if (score < -10)
        return exp(-score);
    else
        return log(1 + exp(score));
}

double GetSum(const vector<const Point*>& banners)
{
    double S = 0.01;

    for(int i = 0; i < (int)banners.size(); ++i)
        S += ToP(banners[i]->regression);

    return S;
}

double GetP(const vector<const Point*>& banners, double S)
{
    double p = 1;

    for(int i = 0; i < (int)banners.size(); ++i)
    {
        double s_i = ToP(banners[i]->regression);
        p *= s_i / S;
        S -= s_i;
    }

    return S;
}

void GetGradients(vector<Point>& banners)
{
    vector<const Point*> selectedBanners(3);
    vector<int> selectedIndexes(3);

    vector< vector< vector<double> > > p_cache(banners.size(), vector< vector<double> >(banners.size(), vector<double>(banners.size(), -1)));

    for(int pointNumber = 0; pointNumber < banners.size(); ++pointNumber)
    {
        double gradient = 0;
        const Point* point = &banners[pointNumber];

        for(int i1 = 0; i1 < (int)banners.size(); ++i1)
            for(int i2 = 0; i2 < (int)banners.size(); ++i2)
                for(int i3 = 0; i3 < (int)banners.size(); ++i3)
                {
                    if(i1 == i2 || i1 == i3 || i2 == i3)
                        continue;

                    if(!((i1 == pointNumber) || (i2 == pointNumber) || (i3 == pointNumber)))
                        continue;

                    selectedIndexes[0] = i1;
                    selectedIndexes[1] = i2;
                    selectedIndexes[2] = i3;

                    selectedBanners[0] = &banners[i1];
                    selectedBanners[1] = &banners[i2];
                    selectedBanners[2] = &banners[i3];

                    double S = GetSum(selectedBanners);

                    if(p_cache[i1][i2][i3] < 0)
                        p_cache[i1][i2][i3] = GetP(selectedBanners, S);

                    double p = p_cache[i1][i2][i3];

                    double c = 0;
                    double money = GetMoney(selectedBanners);
                    double delta = 0;
                    double s_1 = ToP(selectedBanners[0]->regression);
                    double s_2 = ToP(selectedBanners[1]->regression);
                    double s_3 = ToP(selectedBanners[2]->regression);
                    double s_k = ToP(point->regression);

                    if(i1 == pointNumber)
                        delta = p / (s_k + c) - p / (S + c);
                    if(i2 == pointNumber)
                        delta = p / (s_k + c) - p / (S + c) - p / (S - s_1  + c);
                    if(i3 == pointNumber)
                        delta = p / (s_k + c) - p / (S + c) - p / (S - s_1 + c) - p / (S - s_2 + c);

                    if(point->regression > 10)
                        delta *= 1;
                    else if (point->regression < -10)
                        delta *= exp(point->regression);
                    else
                        delta *= exp(point->regression) / (1 + exp(point->regression));

                    gradient += money * delta;
                }

        gradient /= 100;
        banners[pointNumber].gradient = gradient;
    }
}

void CalculateHitsGradient(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd)
{
    for(SerpIterator it = itBegin; it != itEnd; ++it)
    {
        vector<const Point*> bestBanners = PickPremiumBanners(*it, 3, ScoreComparer, true);
        vector<const Point*> bestBannersCopy(bestBanners);

        for(int i = 0; i < (int)it->size(); ++i)
        {
            const Point* point = &(it->at(i));
            const Point* worstInPremium = NULL;

            if(!bestBanners.empty())
                worstInPremium = bestBanners.back();

            double pctr = it->at(i).x[0];
            double cost = it->at(i).x[1];
            double pcpm = pctr * cost;           
            
            bool isBannerInSlot = (std::find(bestBanners.begin(), bestBanners.end(), point) != bestBanners.end());

            double gradient;
                
            if(isBannerInSlot)
                gradient = - 50000 /(point->regression + 100);
            else 
                gradient = 0; 

            it->at(i).gradient = gradient;
        }
    }
}

void CPC_Optimizer::CalculateGradient(vector<Serp>::iterator itBegin, vector<Serp>::iterator itEnd)
{
    for(SerpIterator it = itBegin; it != itEnd; ++it)
    {
        vector<const Point*> bestBanners = PickPremiumBanners(*it, 3, ScoreComparer, false);
        vector<const Point*> bestBannersCopy(bestBanners);

        GetGradients(*it);

        //for(int i = 0; i < (int)it->size(); ++i)
        {
            /*const Point* point = &(it->at(i));
            const Point* worstInPremium = NULL;
            if(bestBanners.size() > 0)
                worstInPremium = bestBanners.back();

            double pctr = it->at(i).x[0];
            double cost = it->at(i).x[1];
            double pcpm = pctr * cost;           
            
            bool isBannerInSlot = (std::find(bestBanners.begin(), bestBanners.end(), point) != bestBanners.end());

            double money_before = GetMoney(bestBanners);

            if(!bestBanners.empty())
                bestBannersCopy[bestBannersCopy.size() - 1] = point;
            else
                bestBannersCopy.push_back(point);

            double money_after = GetMoney(bestBannersCopy);

            double gradient;
                
            if(isBannerInSlot)
                gradient = 0;
            else if(money_after > money_before)
                gradient = (money_after - money_before); /// (worstInPremium->regression - point->regression + 100);
            else
                gradient = 0;*/

            //double gradient = GetGradient(*it, i);
            //it->at(i).gradient = gradient;

            //it->at(i).gradient = gradient / 5;
        }
    }
}

double GetIdealMaxMoney(ConstSerpIterator itBegin, ConstSerpIterator itEnd)
{
    double money = 0;

    for(vector<Serp>::const_iterator it = itBegin; it != itEnd; ++it)    
    {
        vector<const Point*> premiumBanners = PickPremiumBanners(*it, 3, CPMComparer, false);
        vector<const Point*> scoreMaxBanners = PickPremiumBanners(*it, 3, ScoreComparer, false);

        std::sort(scoreMaxBanners.rbegin(), scoreMaxBanners.rend(), CPMComparer);

        if(scoreMaxBanners != premiumBanners)
        {
            int klop = 1;
        }

        money += GetMoney(premiumBanners);
    }

    return money;
}

double GetMoneyFraction(ConstSerpIterator begin, ConstSerpIterator end)
{
    return GetMoney(begin ,end) / GetIdealMaxMoney(begin, end);
}