/*
https://stackoverflow.com/questions/36522882/how-can-i-use-gsl-to-calculate-polynomial-regression-data-points
https://www.lost-infinity.com/fofi-a-free-automatic-telescope-focus-finder-software/
we should also have a look at this one :
https://www.lost-infinity.com/v-curve-fitting-with-a-hyperbolic-function/
*/


#include "polynomialfit.h"


#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <cstdlib>
#include <QtCore>


#include <gsl/gsl_fit.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_min.h>
#include <gsl/gsl_multifit.h>

std::vector<double> gsl_polynomial_fit(const double * const data_x,
                                       const double * const data_y,
                                       const int n, const int order,
                                       double & chisq) {
  int status = 0;
  std::vector<double> vc;
  gsl_vector *y, *c;
  gsl_matrix *X, *cov;
  y = gsl_vector_alloc (n);
  c = gsl_vector_alloc (order+1);
  X   = gsl_matrix_alloc (n, order+1);
  cov = gsl_matrix_alloc (order+1, order+1);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < order+1; j++) {
      gsl_matrix_set (X, i, j, pow(data_x[i],j));
    }
    gsl_vector_set (y, i, data_y[i]);
  }

  gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc (n, order+1);
  status = gsl_multifit_linear (X, y, c, cov, &chisq, work);
  if (status != GSL_SUCCESS)
  {
        qDebug() << gsl_strerror(status);
        return vc;
  }
  else qDebug() << "GSL OK";

  gsl_multifit_linear_free (work);


  for (int i = 0; i < order; i++) {
    vc.push_back(gsl_vector_get(c,i));
  }

  gsl_vector_free (y);
  gsl_vector_free (c);
  gsl_matrix_free (X);
  gsl_matrix_free (cov);

  return vc;
}


bool polynomialfit(int obs, int degree, double *dx, double *dy, double *store) /* n, p */
{
      gsl_multifit_linear_workspace *ws;
      gsl_matrix *cov, *X;
      gsl_vector *y, *c;
      double chisq;

      int i, j;

      X = gsl_matrix_alloc(obs, degree);
      y = gsl_vector_alloc(obs);
      c = gsl_vector_alloc(degree);
      cov = gsl_matrix_alloc(degree, degree);

      for(i=0; i < obs; i++) {
        for(j=0; j < degree; j++) {
          gsl_matrix_set(X, i, j, pow(dx[i], j));
        }
        gsl_vector_set(y, i, dy[i]);
      }

      ws = gsl_multifit_linear_alloc(obs, degree);
      gsl_multifit_linear(X, y, c, cov, &chisq, ws);

      /* store result ... */
      for(i=0; i < degree; i++)
      {
        store[i] = gsl_vector_get(c, i);
      }

      gsl_multifit_linear_free(ws);
      gsl_matrix_free(X);
      gsl_matrix_free(cov);
      gsl_vector_free(y);
      gsl_vector_free(c);
      return true; /* we do not "analyse" the result (cov matrix mainly)
              to know if the fit is "good" */
}
