#ifndef _PREDICATE_HPP_
#define _PREDICATE_HPP_

#include <string>
#include "General.hpp"

struct PredicateStat
{
	int count;
	int left_count;
	int right_count;
	double avg_left;
	double avg_right;
	double avg;
	double left_error;
	double right_error;
	double error;
};

class Predicate
{
public:
    Predicate() {}
    Predicate(int featureId, double threshold) : _featureId(featureId), _threshold(threshold) {}
    bool GetValue(const Point& point) const;  
	string ToString() const;
	int GetFeatureId() const;
	double GetThreshold() const;
	bool operator==(const Predicate& predicate) const;

private:
    int _featureId;
    double _threshold;	
};

#endif /* Predicate.hpp */
