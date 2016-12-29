#include <cmath>

#include "Regression.hpp"

double Karina::Predict(const Point& point) const
{
    vector<double> karinaFeatures = GetKarinaFeatures(point);

    bool isWeekend = ((int)point.x[WEEKDAY_FEATURE] == 5 || (int)point.x[WEEKDAY_FEATURE] == 6);
    int pageNo = point.x[PAGE_NO_FEATURE];

    double *coefficients;

    if(!isWeekend && pageNo == 1) coefficients = _workdays1;
    if(!isWeekend && pageNo > 1) coefficients = _workdays2;
    if(isWeekend && pageNo == 1) coefficients = _weekend1;
    if(isWeekend && pageNo > 1) coefficients = _weekend2;

    return ScalarProduct(karinaFeatures, coefficients);
}

double Karina::ScalarProduct(const vector<double> array1, double array2[]) const
{
    double result = 0;

    for(int i = 0; i < array1.size(); ++i)
        result += array1[i] * array2[i];

    return result;
}

vector<double> Karina::GetKarinaFeatures(const Point& point) const
{
    vector<double> features;

    features.push_back(1);
    std::copy(point.x.begin() + FIRST_ARINA_FEATURE,
              point.x.begin() + FIRST_ARINA_FEATURE + ARINA_FEATURES_COUNT - 1,
              std::back_inserter(features));

    features.push_back(point.x[PCTR_PAGE_NO_FEATURE]);

    double x = log10(point.x[SEARCH_NOT_CLICKS_FEATURE] + 1);
    double y;

    if((int)point.x[PAGE_NO_FEATURE] == 1)
        y = 0.0991 * CUBE(x) - 0.1629 * SQUARE(x) + 0.2176 * x + 0.5884;
    else
        y = 0.1215 * CUBE(x) - 0.2305 * SQUARE(x) + 0.2948 * x + 0.8671;

    double nc = pow(10, -y);

    for(int i = 0; i < ARINA_FEATURES_COUNT; ++i)
        features.push_back(features[1 + i] * nc);

    return features;
}

double Karina::_workdays1[] = {-0.0163942195454,
                0.391299595352,    -0.0185214717339, -0.0303719321739, 0.00895750753876, -0.139921074405, 1.35591956518,
                0.000774903812183, -2.94860544836,   -0.0279582365415, 0.0248593914209,   0.227271339133, 0.0644628905954, 0.0000,
                1.68986749283,      7.46129012681,    0.979215367182,  0.579732621531,    4.79443174975, -6.29937450321,
               -0.775334434277,    -2.04638207847,    0.485626948092,  0.347965576578,   -1.79488457782, -1.47360457026,   0.0000};

double Karina::_workdays2[] = {-7.73494812637e-05,
            -0.149552032356, 0.0344346385345, -0.0857147644073, 0.0483131319882, 0.284553332616, 5.83798906874,
            -0.56840888542, -11.9908332732, 0.0259711233841, -0.0112271277008, 0.350799938028, 0.837148873853, 5.06853540622,
            -1.36137580254, 1.27266575871,  1.14222045823, 0.670969591954, 2.37183169382, -16.0481129817,
            -2.46747947496, 11.0625119779, 0.540867333262, -0.209042955533, -7.10312855441, 1.31114555772, 158.610941039};

double Karina::_weekend1[] = {-0.020245269813,
            0.566793648155, -0.338635371585, 0.0263134729552, -0.00868540707235, -0.289505133304, 0.897364556666,
            0.369931096743, -3.39386082512, -0.0155531416548, -0.0422785612863,   0.152282160584, 0.131581364609, 0.0000,
            0.869138618683, 11.7568929097,   0.674857091746,   0.723860293339,    7.58301268517, -3.97389885123,
            -2.78143053279, -3.49761136653,  0.469872795168,   0.656280282682,   -1.65499427504, -2.34312709228,  0.0000};

double Karina::_weekend2[] = { -0.00364779343704,
            -0.0666145214453,  -1.09711456803, -0.0275648655446,  0.0375025342841, 0.354438694195, 5.09148271721,
            -0.0721178433442, -18.5180044516,   0.0772080583079, -0.120674408282, -0.621182066334, 1.18925922575, 9.63826005578,
            -4.07348918822,    14.2621628283,   0.965544772149,   1.38903455963,   1.29473839433, -6.72277326547,
           -16.4607856731,     78.2044026728,  -0.025361397057,   1.10028526247,  -2.33269991147, -5.18375720071, 158.93465655};