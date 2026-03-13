#pragma once
#include <vector>
std::vector<double> gsl_polynomial_fit(const double * const data_x,
                                       const double * const data_y,
                                       const int n, const int order,
                                       double & chisq);

bool polynomialfit(int obs, int degree, double *dx, double *dy, double *store); /* n, p */
