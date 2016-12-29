#include "General.hpp"
#include "Predicate.hpp"
#include "TaskData.hpp"

#include <algorithm>
#include <cstdio>
#include <limits>
#include <cmath>

#define ERROR_MIN_DELTA 0.0000001

bool Predicate::GetValue(const Point& point) const
{
    return point.x[_featureId] < _threshold;
}

string Predicate::ToString() const
{
	char buffer[256];
	sprintf(buffer, "x%2d < %3.2f", _featureId, _threshold);

	return string(buffer);
}

int Predicate::GetFeatureId() const
{
	return _featureId;
}

double Predicate::GetThreshold() const
{
	return _threshold;
}

bool Predicate::operator==(const Predicate& predicate) const
{
	return (this->GetFeatureId() == predicate.GetFeatureId()) 
		&& (fabs(this->GetThreshold() - predicate.GetThreshold()) < 0.01);
}

PredicateStat GetPredicateStat(const Predicate& predicate, const vector<Point*>& points)
{
    double sum1 = 0, sum2 = 0;
    double avg1 = 0, avg2 = 0;
    int count1 = 0, count2 = 0;

    for(int i = 0; i < (int)points.size(); ++i)
    {
        if(predicate.GetValue(*points[i]))
        {
            sum1 += points[i]->gradient;
            ++count1;
        }
        else
        {
            sum2 += points[i]->gradient;
            ++count2;
        }
    }

    if(count1 > 0) avg1 = sum1 / count1;
    if(count2 > 0) avg2 = sum2 / count2;

    double error1 = 0, error2 = 0;

    for(int i = 0; i < (int)points.size(); ++i)
    {
        if(predicate.GetValue(*points[i]))
            error1 += SQUARE(points[i]->gradient - avg1);
        else
            error2 += SQUARE(points[i]->gradient - avg2);
    }

    PredicateStat stat;

    stat.avg = (sum1 + sum2) / (count1 + count2);
    stat.error = error1 + error2 + SQUARE(avg1 - stat.avg) * count1 + SQUARE(avg2 - stat.avg) * count2;
    stat.count = count1 + count2;

    stat.left_count = count1;
    stat.avg_left = avg1;
    stat.left_error = error1;

    stat.right_count = count2;
    stat.avg_right = avg2;	
    stat.right_error = error2;

    return stat;
}

void CalculateBinData(const vector<Point*>& points, int featureId, vector<double> *avg, vector<int> *count, vector<double> *dispersion)
{
    for(int i = 0; i < (int)points.size(); ++i)
    {
        const Point* point = points[i];
        double feature = point->x[featureId];
        double value = point->gradient;
        int thresholdIndex = point->thresholds[featureId];

        avg->at(thresholdIndex) += value;
        ++count->at(thresholdIndex);
    }

    for(int j = 0; j < (int)count->size(); ++j)
        if(count->at(j) > 0)
            avg->at(j) /= count->at(j);

    for(int i = 0; i < (int)points.size(); ++i)
    {
        const Point* point = points[i];
        double feature = point->x[featureId];
        double value = point->gradient;
        int thresholdIndex = point->thresholds[featureId];

        dispersion->at(thresholdIndex) += SQUARE(value - avg->at(thresholdIndex));
    }
}

void FindBestThreshold(const vector<Point*>& points, int featureId, const vector<double>& thresholds, double *bestThreshold, PredicateStat *bestStat)
{
    vector<double> avg(thresholds.size(), 0);
    vector<int> count(thresholds.size(), 0);
    vector<double> error(thresholds.size(), 0);

    CalculateBinData(points, featureId, &avg, &count, &error);

    double leftError = 0;
    double rightError = 0;
    double leftAvg = 0;
    double rightAvg = 0;
    double prevLeftAvg = 0;
    double prevRightAvg = 0;
    int leftCount = 0;
    int rightCount = 0;
    double leftSum = 0;
    double rightSum = 0;
    
    for(int j = 0; j < (int)count.size(); ++j)
    {
        rightCount += count[j];
        rightSum += avg[j] * count[j];
    }

    double errorMin =  std::numeric_limits<double>::max();
    *bestThreshold = 0;

    for(int j = 0; j < (int)count.size(); ++j)
    {
        prevLeftAvg = leftAvg;
        prevRightAvg = rightAvg;

        if(leftCount > 0) leftAvg = leftSum / leftCount;
        if(rightCount > 0) rightAvg = rightSum / rightCount;

        double leftError = 0;
        double rightError = 0;

        for(int k = 0; k < j; ++k)
            leftError += error[k] + SQUARE(leftAvg - avg[k]) * count[k];

        for(int k = j; k < (int)count.size(); ++k)
            rightError += error[k] + SQUARE(rightAvg - avg[k]) * count[k];

        if(leftError + rightError < errorMin)
        {
            if(j == 0)
            {
                bestStat->avg = rightAvg;
                bestStat->count = rightCount;
                bestStat->error = rightError;
            }
            
            bestStat->avg_left = leftAvg;
            bestStat->avg_right = rightAvg;
            bestStat->left_count = leftCount;
            bestStat->right_count = rightCount;
            bestStat->left_error = leftError;
            bestStat->right_error = rightError;

            *bestThreshold = thresholds[j];
            errorMin = leftError + rightError;
        }

        leftCount += count[j];
        leftSum += avg[j] * count[j];

        rightCount -= count[j];
        rightSum -= avg[j] * count[j];
    }
}

Predicate FindBestPredicateFast(const vector<Point*>& points, const TaskData& taskData, const vector<bool>& allowedFeatures, PredicateStat *bestStat)
{
    double errorMin = std::numeric_limits<double>::max();
    int bestFeatureId = -1;
    double bestThreshold = -1;

    for(int featureId = 0; featureId < taskData.GetFeaturesCount(); ++featureId)
    {
        if(allowedFeatures[featureId])
        {
            const vector<double> *thresholds = taskData.GetThresholds(featureId);

            double threshold;
            PredicateStat stat;
            FindBestThreshold(points, featureId, *thresholds, &threshold, &stat);

            double error = stat.left_error + stat.right_error;

            if(error < errorMin - ERROR_MIN_DELTA)
            {
                *bestStat = stat;
                errorMin = error;
                bestFeatureId = featureId;
                bestThreshold = threshold;
            }
        }
    }

    return Predicate(bestFeatureId, bestThreshold);
}

double GetBestThresholdFull(const vector<Point*>& points, int featureId, PredicateStat *bestStat)
{
    vector< pair<double, double> > values(points.size());

    for(int i = 0; i < (int)points.size(); ++i)
    {
        values[i].first = points[i]->x[featureId];
        values[i].second = points[i]->gradient;
    }

    sort(values.begin(), values.end(), PairFirstCompare);

    double leftError = 0;
    double rightError = 0;
    double leftAvg = 0;
    double rightAvg = 0;
    int leftCount = 0;
    int rightCount = 0;
    double leftSum = 0;
    double rightSum = 0;
    
    for(int j = 0; j < (int)values.size(); ++j)
    {
        ++rightCount;
        rightSum += values[j].second;
    }

    if(rightCount > 0)
        rightAvg = rightSum / rightCount;

    for(int j = 0; j < (int)values.size(); ++j)
    {
        rightError += SQUARE(values[j].second - rightAvg);
    }

    double errorMin = std::numeric_limits<double>::max();
    double bestThreshold = values[0].first;

    for(int j = 0; j < (int)values.size() - 1; ++j)
    {
        if(leftError + rightError < errorMin)
        {
            if(j == 0)
            {
                bestStat->avg = rightAvg;
                bestStat->count = rightCount;
                bestStat->error = rightError;
            }
            
            bestStat->avg_left = leftAvg;
            bestStat->avg_right = rightAvg;
            bestStat->left_count = leftCount;
            bestStat->right_count = rightCount;
            bestStat->left_error = leftError;
            bestStat->right_error = rightError;

            bestThreshold = values[j].first;
            errorMin = leftError + rightError;
        }

        int oldLeftCount = leftCount;
        int oldRightCount = rightCount;

        ++leftCount;
        leftSum += values[j].second;

        --rightCount;
        rightSum -= values[j].second;

        double newLeftAvg;
        double newRightAvg;

        if(leftCount > 0) newLeftAvg = leftSum / leftCount;
        if(rightCount > 0) newRightAvg = rightSum / rightCount;

        leftError += SQUARE(values[j].second - newLeftAvg) + SQUARE(leftAvg - newLeftAvg) * oldLeftCount;
        rightError -= SQUARE(values[j].second - newRightAvg) + SQUARE(rightAvg - newRightAvg) * rightCount;

        leftAvg = newLeftAvg;
        rightAvg = newRightAvg;
    }

    return bestThreshold;
}

Predicate FindBestPredicateFull(const vector<Point*>& points, const TaskData& taskData, const vector<bool>& allowedFeatures, PredicateStat *bestStat)
{
    double errorMin = std::numeric_limits<double>::max();
    int bestFeatureId = -1;
    double bestThreshold = -1;

    for(int featureId = 0; featureId < taskData.GetFeaturesCount(); ++featureId)
    {
        if(allowedFeatures[featureId])
        {
            PredicateStat stat;
            double threshold = GetBestThresholdFull(points, featureId, &stat);

            Predicate predicate(featureId, threshold);
            
            double error = stat.left_error + stat.right_error;

            if((error < errorMin) || (featureId == 0))
            {
                *bestStat = stat;
                errorMin = error;
                bestFeatureId = featureId;
                bestThreshold = threshold;
                }
            }
    }

    if(bestThreshold == -1)
    {
        int klop = 1;
    }

    return Predicate(bestFeatureId, bestThreshold);
}

Predicate FindBestPredicate(const vector<Point*>& points, const TaskData& taskData, const vector<bool>& allowedFeatures, PredicateStat *bestStat)
{    
    int featuresCount = (int)points[0]->x.size();
    double errorMin = std::numeric_limits<double>::max();
    int bestFeatureId = -1;
    double bestThreshold = -1;

    for(int featureId = 0; featureId < featuresCount; ++featureId)
    {
        if(allowedFeatures[featureId])
        {
            const vector<double> *thresholds = taskData.GetThresholds(featureId);

            for(int i = 0; i < (int)thresholds->size(); ++i)
            {
			    Predicate predicate(featureId, thresholds->at(i));
			    PredicateStat stat = GetPredicateStat(predicate, points);
			    double error = stat.left_error + stat.right_error;

                if((error < errorMin - ERROR_MIN_DELTA) || ((featureId == 0) && (i == 0)))
                {
				    *bestStat = stat;
                    errorMin = error;
                    bestFeatureId = featureId;
                    bestThreshold = thresholds->at(i);
                }
            }
        }
    }

    return Predicate(bestFeatureId, bestThreshold);
}

Predicate FindBestPredicate(const vector< vector<Point*> >& points, const TaskData& taskData, const vector<bool>& allowedFeatures, PredicateStat *bestStat)
{    
    int featuresCount = taskData.GetFeaturesCount();
    double errorMin = std::numeric_limits<double>::max();	
    int bestFeatureId = -1;
    double bestThreshold = -1;

    for(int featureId = 0; featureId < featuresCount; ++featureId)
    {
        if(allowedFeatures[featureId])
        {
            const vector<double> *thresholds = taskData.GetThresholds(featureId);

            for(int i = 0; i < (int)thresholds->size(); ++i)
            {
			    Predicate predicate(featureId, thresholds->at(i));
                double error = 0;                

                for(int j = 0; j < (int)points.size(); ++j)
                {
                   PredicateStat stat = GetPredicateStat(predicate, points[j]); 
                   error += stat.left_error + stat.right_error;
                }

                if((error < errorMin) || ((featureId == 0) && (i == 0)))
                {
                    errorMin = error;
                    bestFeatureId = featureId;
                    bestThreshold = thresholds->at(i);
                }
            }
        }
    }

    return Predicate(bestFeatureId, bestThreshold);
}
