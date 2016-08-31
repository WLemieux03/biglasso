#include <RcppArmadillo.h>
#include <iostream>
#include "bigmemory/BigMatrix.h"
#include "bigmemory/MatrixAccessor.hpp"
#include "bigmemory/bigmemoryDefines.h"
#include <time.h>
#include <omp.h>

#include "utilities.h"
//#include "defines.h"
using namespace std;

// check strong set
int check_strong_set(int *e1, int *e2, double *z, XPtr<BigMatrix> xpMat, int *row_idx, NumericVector &center, NumericVector &scale,
                     double lambda, double sumResid, double alpha, double *r, double *m, int n, int p) {
  MatrixAccessor<double> xAcc(*xpMat);
  double *xCol, sum, l1;
  int j, violations = 0;
  
  #pragma omp parallel for private(j, sum, l1) reduction(+:violations) schedule(static) 
  for (j = 0; j < p; j++) {
    if (e1[j] == 0 && e2[j] == 1) {
      xCol = xAcc[j];
      sum = 0.0;
      for (int i=0; i < n; i++) {
        sum = sum + xCol[row_idx[i]] * r[i];
      }
      z[j] = (sum - center[j] * sumResid) / (scale[j] * n);
     
      l1 = lambda * m[j] * alpha;
      if(fabs(z[j]) > l1) {
        e1[j] = 1;
        violations++;
      }
    }
  }
  return violations;
}

// check rest set
int check_rest_set(int *e1, int *e2, double *z, XPtr<BigMatrix> xpMat, int *row_idx, NumericVector &center, NumericVector &scale,
                   double lambda, double sumResid, double alpha, double *r, double *m, int n, int p) {
  
  MatrixAccessor<double> xAcc(*xpMat);
 
  double *xCol, sum, l1;
  int j, violations = 0;
  #pragma omp parallel for private(j, sum, l1) reduction(+:violations) schedule(static) 
  for (j = 0; j < p; j++) {
    if (e2[j] == 0) {
      xCol = xAcc[j];
      sum = 0.0;
      for (int i=0; i < n; i++) {
        sum = sum + xCol[row_idx[i]] * r[i];
      }
      z[j] = (sum - center[j] * sumResid) / (scale[j] * n);
      
      l1 = lambda * m[j] * alpha;
      if(fabs(z[j]) > l1) {
        e1[j] = e2[j] = 1;
        violations++;
      }
    }
  }
  return violations;
}

// Coordinate descent for gaussian models
RcppExport SEXP cdfit_gaussian_hsr(SEXP X_, SEXP y_, SEXP row_idx_, SEXP lambda_, 
                                   SEXP nlambda_, SEXP lambda_min_, 
                                   SEXP alpha_, SEXP user_, SEXP eps_, 
                                   SEXP max_iter_, SEXP multiplier_, SEXP dfmax_, 
                                   SEXP ncore_, SEXP verbose_) {
  XPtr<BigMatrix> xMat(X_);
  double *y = REAL(y_);
  int *row_idx = INTEGER(row_idx_);
  // const char *xf_bin = CHAR(Rf_asChar(xf_bin_));
  // int nchunks = INTEGER(nchunks_)[0];
  // int dome = INTEGER(dome_)[0]; // use dome test for screening or not?
  double lambda_min = REAL(lambda_min_)[0];
  double alpha = REAL(alpha_)[0];
  int n = Rf_length(row_idx_); // number of observations used for fitting model
  int p = xMat->ncol();
  int n_total = xMat->nrow(); // number of total observations
  int L = INTEGER(nlambda_)[0];
  int user = INTEGER(user_)[0];
  int verbose = INTEGER(verbose_)[0];
  // int chunk_cols = p / nchunks;
  
  NumericVector lambda(L);
  if (user != 0) {
    lambda = Rcpp::as<NumericVector>(lambda_);
  } 
  
  double eps = REAL(eps_)[0];
  int max_iter = INTEGER(max_iter_)[0];
  double *m = REAL(multiplier_);
  int dfmax = INTEGER(dfmax_)[0];
  
  NumericVector center(p);
  NumericVector scale(p);
  double *z = Calloc(p, double);
  double lambda_max = 0.0;
  double *lambda_max_ptr = &lambda_max;
  int xmax_idx = 0;
  int *xmax_ptr = &xmax_idx;

  if (verbose) {
    char buff1[100];
    time_t now1 = time (0);
    strftime (buff1, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&now1));
    Rprintf("\nPreprocessing start: %s\n", buff1);
  }
  // standardize: get center, scale; get z, lambda_max, xmax_idx;
  standardize_and_get_residual(center, scale, z, lambda_max_ptr, xmax_ptr, xMat, 
                               y, row_idx, lambda_min, alpha, n, p);
  
  if (verbose) {
    char buff1[100];
    time_t now1 = time (0);
    strftime (buff1, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&now1));
    Rprintf("Preprocessing end: %s\n", buff1);
    Rprintf("\n-----------------------------------------------\n");
  }
  
  double *r = Calloc(n, double);
  for (int i=0; i<n; i++) r[i] = y[i];
  double sumResid = sum(r, n);
  
  // beta
  arma::sp_mat beta = arma::sp_mat(p, L);
  double *a = Calloc(p, double); //Beta from previous iteration
//   for (int j = 0; j < p; j++) {
//     a[j] = 0.0;
//   }

  NumericVector loss(L);
  IntegerVector iter(L);
  IntegerVector n_reject(L);
  
  double l1, l2, cutoff, shift;
  int *e1 = Calloc(p, int);
  // for (int j=0; j<p; j++) e1[j] = 0;
  int *e2 = Calloc(p, int);
  // for (int j=0; j<p; j++) e2[j] = 0;
  int converged, lstart = 0, violations;
  
  // lambda, equally spaced on linear scale
//   if (user == 0) {
//     // set up lambda, equally spaced on log scale
//     // double log_lambda_max = log(lambda_max);
//     // double log_lambda_min = log(lambda_min*lambda_max);
//     double delta = (lambda_max - lambda_min*lambda_max) / (L-1);
//     for (int l = 0; l < L; l++) {
//       lambda[l] = lambda_max - l * delta;
//     }
//     loss[0] = gLoss(r,n);
//     // lstart = 1;
//     // n_reject[0] = p; // strong rule rejects all variables at lambda_max
//   } 
 
  // lambda, equally spaced on log scale
  if (user == 0) {
    // set up lambda, equally spaced on log scale
    double log_lambda_max = log(lambda_max);
    double log_lambda_min = log(lambda_min*lambda_max);
    double delta = (log_lambda_max - log_lambda_min) / (L-1);
    for (int l = 0; l < L; l++) {
      lambda[l] = exp(log_lambda_max - l * delta);
    }
    loss[0] = gLoss(r,n);
    // lstart = 1;
    // n_reject[0] = p; // strong rule rejects all variables at lambda_max
  } 
  
  // set up omp
  int useCores = INTEGER(ncore_)[0];
  int haveCores = omp_get_num_procs();
  if(useCores < 1) {
    useCores = haveCores;
  }
  omp_set_dynamic(0);
  omp_set_num_threads(useCores);
  
  // Path
  for (int l=lstart;l<L;l++) {
    if(verbose) {
      // output time
      char buff[100];
      time_t now = time (0);
      strftime (buff, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&now));
      Rprintf("Lambda %d. Now time: %s\n", l, buff);
    }
    
    if (l != 0) {
      // Assign a by previous b
      for (int j=0;j<p;j++) {
        a[j] = beta(j, l-1);
      }
      // Check dfmax
      int nv = 0;
      for (int j=0; j<p; j++) {
        if (a[j] != 0) nv++;
      }
      if (nv > dfmax) {
        for (int ll=l; ll<L; ll++) iter[ll] = NA_INTEGER;
        return List::create(beta, center, scale, lambda, loss, iter, n_reject);
      }
      // strong set
      cutoff = 2*lambda[l] - lambda[l-1];
      for (int j=0; j<p; j++) {
        if (fabs(z[j]) > (cutoff * alpha * m[j])) {
          e2[j] = 1;
        } else {
          e2[j] = 0;
        }
      } 
    } else {
      // strong set
      cutoff = 2*lambda[l] - lambda_max;
      for (int j=0; j<p; j++) {
        if (fabs(z[j]) > (cutoff * alpha * m[j])) {
          e2[j] = 1;
        } else {
          e2[j] = 0;
        }
      }  
    }
    
    n_reject[l] = p - sum_discard(e2, p);
    // Rprintf("\t n_reject[%d] = %d\n", l, n_reject[l]);
    
    while(iter[l] < max_iter) {
      while(iter[l] < max_iter){
        while(iter[l] < max_iter) {
          iter[l]++;
          //solve lasso over ever-active set
          for (int j = 0; j < p; j++) {
            if (e1[j]) {
              z[j] = crossprod_resid(xMat, r, sumResid, row_idx, center[j], scale[j], n, j) / n + a[j];
              // Update beta_j
              l1 = lambda[l] * m[j] * alpha;
              l2 = lambda[l] * m[j] * (1-alpha);
              beta(j, l) = lasso(z[j], l1, l2, 1);
              // Update r
              shift = beta(j, l) - a[j];
              if (shift !=0) {
                update_resid(X_, r, shift, row_idx, center[j], scale[j], n, j);
                sumResid = sum(r, n); //update sum of residual
              }
            }
          }
          // Check for convergence
          converged = checkConvergence(beta, a, eps, l, p);
          // update a
          for (int j = 0; j < p; j++) {
            a[j] = beta(j, l);
          }
          if (converged) break;
        }
        
        // Scan for violations in strong set
        violations = check_strong_set(e1, e2, z, xMat, row_idx, center, scale, lambda[l], 
                                      sumResid, alpha, r, m, n, p); 
        if (violations==0) break;
      }
      
      // Scan for violations in rest set
      // Rprintf("\tlambda[%d] = %f, iteration = %d, start omp 2:\n", l, lambda[l], iter[l]);
      violations = check_rest_set(e1, e2, z, xMat, row_idx, center,  scale, lambda[l], sumResid, alpha, r, m, n, p);
      if (violations == 0) {
        loss[l] = gLoss(r, n);
        break;
      }
    }
  }

  free_memo_hsr(a, r, z, e1, e2);
  return List::create(beta, center, scale, lambda, loss, iter, n_reject);
}
