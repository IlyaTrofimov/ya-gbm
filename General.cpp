#include "General.hpp"
#include "DataSetIterator.hpp"

using std::pair;

bool PointComparer(const Point& point1, const Point& point2)
{
	return (point1.y < point2.y);
}

bool PairSecondCompare(const pair<double, double>& pair1, const pair<double, double>& pair2)
{
	return (pair1.second < pair2.second);
}

bool PairFirstCompare(const pair<double, double>& pair1, const pair<double, double>& pair2)
{
	return (pair1.first < pair2.first);
}

bool PairSecondCompare2(const pair<Point*, double>& pair1, const pair<Point*, double>& pair2)
{
	return (pair1.second < pair2.second);
}

double GetAvg(const vector<Point*>& points)
{
    int count = 0;
    double sum = 0;

    for(int i = 0; i < (int)points.size(); ++i)
    {
        ++count;
        sum += points[i]->gradient;
    }

    return (count > 0 ? sum / count : 0);
}

double GetAvgResponce(DataSet* dataSet)
{
    int count = 0;
    double sum = 0;

    DataSetIterator it(dataSet);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
    {
        ++count;
        sum += it.Current()->y;
    }

    return (count > 0 ? sum / count : 0);
}

double GetAvgResponce(SerpIterator begin, SerpIterator end)
{
    int count = 0;
    double sum = 0;

    DataSetIterator it(begin, end);

    for(it.MoveFirst(); !it.IsDone(); it.MoveNext())
    {
        ++count;
        sum += it.Current()->y;
    }

    return (count > 0 ? sum / count : 0);
}

string ToString(int value)
{
    char buffer[128];
    sprintf(buffer, "%d", value);

    return string(buffer);
}

string ToString(unsigned int value)
{
    char buffer[128];
    sprintf(buffer, "%d", value);

    return string(buffer);
}

string ToString(unsigned long value)
{
    char buffer[128];
    sprintf(buffer, "%ld", value);

    return string(buffer);
}

string ToString(double value)
{
    char buffer[128];
    sprintf(buffer, "%f", value);

    return string(buffer);
}

string PadRight(string s, int width)
{
    if(s.size() < width)
        return s.append(width - s.size(), ' ');
    else
        return s;
}
