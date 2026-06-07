/**
 * @file polynomialfit.h
 * @brief Polynomial fitting utilities for CCD orientation determination (unused)
 *
 * These functions fit a polynomial to calibration drift measurements to determine
 * CCD orientation angle. The CCD orientation would then be used to properly
 * interpret which axis drifts correspond to RA vs DEC.
 *
 * Currently UNUSED in the guider module - see guider.cpp:584 (commented out code).
 * Could be enabled in future for more robust pier-side compensation.
 *
 * @note Requires GSL (GNU Scientific Library) for polynomial fitting
 * @todo Enable polynomial fitting and store _ccdOrientation in calibration values
 */

#pragma once
#include <vector>

/**
 * @brief Fit a polynomial to (x,y) data using GSL
 *
 * Uses GNU Scientific Library's multi-parameter fitting for robustness.
 *
 * @param data_x X values (independent variable)
 * @param data_y Y values (dependent variable)
 * @param n Number of data points
 * @param order Polynomial order (1=linear, 2=quadratic, etc.)
 * @param chisq Output: chi-square error metric (lower = better fit)
 * @return Vector of polynomial coefficients [a0, a1, a2, ...]
 *         Polynomial = a0 + a1*x + a2*x² + ...
 */
std::vector<double> gsl_polynomial_fit(const double * const data_x,
                                       const double * const data_y,
                                       const int n, const int order,
                                       double & chisq);

/**
 * @brief Simple polynomial fitting wrapper
 *
 * Fits polynomial to calibration drift data during calibration phase.
 * Used to determine CCD rotation angle from measured drifts.
 *
 * Application in guider:
 *  - dx[i] = drift in X direction during calibration step i
 *  - dy[i] = drift in Y direction during calibration step i
 *  - Fit polynomial: dy = slope * dx + offset
 *  - CCD orientation = atan(slope) * 180/π
 *
 * @param obs Number of observations (calibration measurements)
 * @param degree Polynomial degree (typically 1 or 2)
 * @param dx Array of X drifts (input)
 * @param dy Array of Y drifts (input)
 * @param store Array to store result coefficients (output)
 * @return true if fit successful, false otherwise
 *
 * @note Currently disabled - see SMComputeCal() for TODO to re-enable
 */
bool polynomialfit(int obs, int degree, double *dx, double *dy, double *store);
