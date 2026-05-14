/**
 * @file polynomialfit.cpp
 * @brief Polynomial regression using GNU Scientific Library
 *
 * References:
 *   - https://stackoverflow.com/questions/36522882/how-can-i-use-gsl-to-calculate-polynomial-regression-data-points
 *   - https://www.lost-infinity.com/fofi-a-free-automatic-telescope-focus-finder-software/
 *   - https://www.lost-infinity.com/v-curve-fitting-with-a-hyperbolic-function/
 *
 * GSL (GNU Scientific Library) provides robust multi-parameter least-squares fitting.
 * Used in guider for determining CCD orientation from calibration drift measurements.
 *
 * CURRENT STATUS: DISABLED
 *   - Called from: guider.cpp:SMComputeCal() (line 584, commented out)
 *   - Would compute CCD angle = atan(slope) from dy/dx polynomial
 *   - Could enable for better pier-side compensation
 */

#include "polynomialfit.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <cstdlib>
#include <QtCore>

// GSL multi-parameter fitting headers
#include <gsl/gsl_fit.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_min.h>
#include <gsl/gsl_multifit.h>

/**
 * @brief Fit polynomial using GSL multi-parameter fitting
 *
 * Performs least-squares polynomial fit: y = c[0] + c[1]*x + c[2]*x² + ...
 * Using GSL's robust linear algebra routines with covariance matrix calculation.
 *
 * Algorithm:
 *  1. Build design matrix X where X[i,j] = x[i]^j
 *  2. Setup vector y with data points
 *  3. Solve linear system: X*c = y (least squares)
 *  4. Return coefficient vector c and chi-square error
 */
std::vector<double> gsl_polynomial_fit(const double * const data_x,
                                       const double * const data_y,
                                       const int n, const int order,
                                       double & chisq) {
  int status = 0;
  std::vector<double> vc;

  // Allocate GSL vectors and matrices
  gsl_vector *y, *c;        // y=data vector, c=coefficient vector
  gsl_matrix *X, *cov;      // X=design matrix, cov=covariance matrix
  y = gsl_vector_alloc (n);
  c = gsl_vector_alloc (order+1);
  X   = gsl_matrix_alloc (n, order+1);
  cov = gsl_matrix_alloc (order+1, order+1);

  // Build design matrix: X[i,j] = (data_x[i])^j
  // This transforms the problem into a linear system
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < order+1; j++) {
      gsl_matrix_set (X, i, j, pow(data_x[i], j));  // Powers: 1, x, x², x³, ...
    }
    gsl_vector_set (y, i, data_y[i]);  // Fill y vector with data points
  }

  // Solve least-squares linear system using GSL
  gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc (n, order+1);
  status = gsl_multifit_linear (X, y, c, cov, &chisq, work);

  // Check for errors
  if (status != GSL_SUCCESS)
  {
    qDebug() << "GSL Error:" << gsl_strerror(status);
    return vc;  // Return empty vector on error
  }
  else
  {
    qDebug() << "GSL polynomial fit OK, chi-square:" << chisq;
  }

  gsl_multifit_linear_free (work);

  // Extract coefficients into output vector
  for (int i = 0; i < order; i++) {
    vc.push_back(gsl_vector_get(c, i));
  }

  // Cleanup GSL memory
  gsl_vector_free (y);
  gsl_vector_free (c);
  gsl_matrix_free (X);
  gsl_matrix_free (cov);

  return vc;
}


/**
 * @brief Polynomial fitting wrapper for calibration drift data
 *
 * Fits polynomial to DX/DY drift measurements during calibration to determine
 * CCD orientation angle.
 *
 * Application in guider:
 *  - During calibration, we send pulses N/S/E/W and measure (dx, dy) drifts
 *  - Fit polynomial: dy = slope * dx + offset
 *  - CCD orientation angle = atan(slope) * 180/π
 *  - Would be stored in calibrationvalues for future use
 *
 * Currently DISABLED in SMComputeCal (line ~584) - commented out
 *
 * @param obs Number of observations (measurements)
 * @param degree Polynomial degree (typically 2 for linear + offset)
 * @param dx Array of X-axis drifts (calibration measurements)
 * @param dy Array of Y-axis drifts (calibration measurements)
 * @param store Array to store result coefficients [c0, c1, c2, ...]
 * @return Always returns true (no error checking on covariance matrix)
 */
bool polynomialfit(int obs, int degree, double *dx, double *dy, double *store)
{
      // Allocate GSL workspace and matrices
      gsl_multifit_linear_workspace *ws;
      gsl_matrix *cov, *X;     // X = design matrix, cov = covariance matrix
      gsl_vector *y, *c;       // y = data vector, c = coefficients
      double chisq;            // Chi-square goodness-of-fit metric

      int i, j;

      // Setup matrices for obs observations and degree polynomial
      X = gsl_matrix_alloc(obs, degree);       // Design matrix: obs × degree
      y = gsl_vector_alloc(obs);               // Data vector: obs elements
      c = gsl_vector_alloc(degree);            // Coefficient vector: degree elements
      cov = gsl_matrix_alloc(degree, degree);  // Covariance matrix: degree × degree

      // Build design matrix: X[i,j] = (dx[i])^j
      // This allows fitting polynomial: dy[i] = c[0] + c[1]*dx[i] + c[2]*(dx[i])² + ...
      for(i = 0; i < obs; i++) {
        for(j = 0; j < degree; j++) {
          gsl_matrix_set(X, i, j, pow(dx[i], j));  // X[i,j] = dx[i]^j
        }
        gsl_vector_set(y, i, dy[i]);  // Fill y vector with measurements
      }

      // Perform least-squares fit: X*c = y
      ws = gsl_multifit_linear_alloc(obs, degree);
      gsl_multifit_linear(X, y, c, cov, &chisq, ws);

      // Extract coefficients to output array
      for(i = 0; i < degree; i++)
      {
        store[i] = gsl_vector_get(c, i);  // store[0] = offset, store[1] = slope, etc.
      }

      // Cleanup GSL memory
      gsl_multifit_linear_free(ws);
      gsl_matrix_free(X);
      gsl_matrix_free(cov);
      gsl_vector_free(y);
      gsl_vector_free(c);

      // NOTE: Always returns true regardless of fit quality
      // Could check covariance matrix for reliability (but currently doesn't)
      return true;
}
