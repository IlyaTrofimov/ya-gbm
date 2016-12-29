#include "General.hpp"
#include "Regression.hpp"

Arina::Arina() : IRegression("ARINA")
{
    _PageNo1Regression = GetArinaPageNo1();
    _PageNo2Regression = GetArinaPageNo2();
}

double Arina::Predict(const Point& point) const
{
    if((int)point.x[PAGE_NO_FEATURE] == 1)
        return _PageNo1Regression->Predict(point);
    else
        return _PageNo2Regression->Predict(point) + point.x[PCTR_PAGE_NO_FEATURE] * 18.305776085;
}

Arina::~Arina()
{
    delete _PageNo1Regression;
    delete _PageNo2Regression;
}

LinearRegression* Arina::GetArinaPageNo1()
{
    double constant = -0.0164513551347;
    double coeffs[] = {0.672197722109, 1.61524689029, 0.105844772781, 0.1600513469, 0.585842685592,
                        0.169418079604, -0.318521823625, -1.66774263522, 0.0630650782561, 0.0843481741734,
                        -0.117801186506, -0.202979388402};

    return new LinearRegression("ARINA", FIRST_ARINA_FEATURE, constant, vector<double>(coeffs, coeffs + ARRAY_LEN(coeffs)));
}

LinearRegression* Arina::GetArinaPageNo2()
{
    double constant = -0.00424625497382;
    double coeffs[] = {-0.264396042596, 0.662746210162, 0.0445707784549, 0.100187296491, 0.457966309929,
                        3.8011465987, -0.345694200918, -9.29055163994, 0.0743023211573, -0.00349802707329,
                        -0.904471733523, 0.539988710105};

    return new LinearRegression("ARINA", FIRST_ARINA_FEATURE, constant, vector<double>(coeffs, coeffs + ARRAY_LEN(coeffs)));
}