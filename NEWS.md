# biglasso 1.4-0 (01/08/2021)
* adaptive screening methods were implemented and set as default when applicable
* added sparse Cox regression
*  removed uncompetitive screening methods and combined naming of screening methods
*  version 1.4-0 for CRAN submission

# biglasso 1.3-7 (08/31/2019)
* update email to personal email
* coef(cvfit) returns only nonzero cells, as a labelled vector
* set HSR rules as default
* option for non-standardization

# biglasso 1.3-6 (04/12/2017)
* optimized the code for computing the slores rule.
* added Slores screening without active cycling (-NAC) for logistic regression, research usage only.
* corrected BEDPP for elastic net.
* fixed a bug related to "exporting SSR-BEDPP".

# biglasso 1.3-5 (03/29/2017)
* redocumented using Roxygen2.
* registered native routines for faster and more stable performance.

# biglasso 1.3-4 (01/29/2017)
* fixed a bug related to `dfmax` option. (thanks you Florian Privé!)

# biglasso 1.3-3 (01/24/2017)
* fixed bugs related to KKT checking for elastic net. (thanks you Florian Privé!)
* added references for screening rules and the technical paper of biglasso package.

# biglasso 1.3-2 (01/16/2017)
* added screening methods without active cycling (-NAC) for comparison, research usage only.
* fixed a bug related to numeric comparison in Dome test.

# biglasso 1.3-1 (12/24/2016)
* fixed bug in SSR-Slores related to numeric equality comparison.

# biglasso 1.3-0 (12/15/2016)
* version 1.3-0 for CRAN submission.
  
# biglasso 1.2-6 (12/15/2016)
* added a newly proposed screening rule, SSR-Slores, for lasso-penalized logistic regression.
* added SSR-BEDPP for elastic-net-penalized linear regression.

# biglasso 1.2-5 (12/10/2016)
*  updated README.md with benchmarking results.
*  added tutorial (vignette).

# biglasso 1.2-4 (11/14/2016)
*  added gaussian.cpp: solve lasso without screening, for research only.
*  added tests.

# biglasso 1.2-3 (11/13/2016)
*  changed convergence criteria of logistic regression to be the same as that in glmnet.
*  optimized source code; preparing for CRAN submission.
*  fixed memory leaks occurred on Windows.

# biglasso 1.2-2 (10/27/2016)
* added internal data set: the colon cancer data.

# biglasso 1.2-1 (10/18/2016)
* Implemented another new screening rule (SSR-BEDPP), also combining hybrid strong rule 
with a safe rule (BEDPP).
* implemented EDPP rule with active set cycling strategy for linear regression.
*  changed convergence criteria to be the same as that in glmnet.

# biglasso 1.1-2 (9/1/2016)
* fixed bugs occurred when some features have identical values for different 
observations. These features are internally removed from model fitting.

# biglasso 1.1-1 (8/31/2016)
* Three sparse screening rules (SSR, EDPP, SSR-Dome) were implemented. Our 
new proposed HSR-Dome combines HSR and Dome test for feature screening,
leading to even better performance as compared to 'glmnet'.	
* OpenMP parallel computing was added to speedup single model fitting.
* Both exact Newton and majorization-minimization (MM) algorithm for logistic regression
were implemented. The latter could be faster, especially in data-larger-than-RAM cases.
* Source code were rewritten in pure cpp.
*  Sparse matrix representation was added using Armadillo library.

# biglasso 1.0-1 (3/1/2016)
* package ready for CRAN submission.