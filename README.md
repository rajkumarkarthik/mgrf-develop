[![Build Status](https://travis-ci.org/swager/grf.svg?branch=master)](https://travis-ci.org/swager/grf)
![CRAN Downloads overall](http://cranlogs.r-pkg.org/badges/grand-total/grf)

# mgrf: multivariate generalized random forests

A pluggable package for forest-based statistical estimation and inference. MGRF extends the GRF framework and provides non-parametric methods for multivariate analysis, including least-squares regression and treatment effect estimation (optionally using instrumental variables). 

In addition, MGRF supports 'honest' estimation (where one subset of the data is used for choosing splits, and another for populating the leaves of the tree), and confidence intervals for least-squares regression and treatment effect estimation.

This repo is a fork from https://github.com/swager/grf and will be merged back into there, once the mutivariate functionality is fully developed. This package is currently in beta, and we expect to make continual improvements to its performance and usability.

### Authors

The MGRF version of GRF is written by Karthik Rajkumar (krajkumar@stanford.edu) and the overall GRF package is written and maintained by Julie Tibshirani (jtibs@cs.stanford.edu), Susan Athey, and Stefan Wager. If you use MGRF, please cite it in your work as

Karthik Rajkumar, Susan Athey, Stefan Wager, and Julie Tibshirani. <b>Multivariate Generalized Random Forests</b>, 2017.

The repository first started as a fork of the [ranger](https://github.com/imbs-hl/ranger) repository -- we owe a great deal of thanks to the ranger authors for their useful and free package.

### Installation

MGRF can be installed from source in R:
```

```R
install.packages("https://raw.github.com/rajkumarkartik/mgrf-develop/master/releases/grf_0.9.3.tar.gz", repos = NULL, type = "source")
```

Note that to install from source, a compiler that implements C++11 is required (clang 3.3 or higher, or g++ 4.8 or higher). If installing on Windows, the RTools toolchain is also required.

### Usage Examples

```R
library(grf)

# Generate data.
n = 2000; p = 10
X = matrix(rnorm(n*p), n, p)
X.test = matrix(0, 101, p)
X.test[,1] = seq(-2, 2, length.out = 101)

# Perform multivariate treatment effect estimation.
W1 = rbinom(n, 1, 0.5)
W2 = rbinom(n, 1, 0.5)
W = cbind(W1, W2)
Y = pmax(X[,1], 0) * W1 + X[,2] * W2 + pmin(X[,3], 0) + rnorm(n)
tau.forest = causal_forest(X, Y, W)
tau.hat = predict(tau.forest, X.test)
plot(X.test[,1], tau.hat$predictions[, 1], ylim = range(tau.hat$predictions[,1], 0, 2), xlab = "x", ylab = "tau", type = "l")
lines(X.test[,1], pmax(0, X.test[,1]), col = 2, lty = 2)

# Add confidence intervals for heterogeneous treatment effects; growing more trees is now recommended.
tau.forest = causal_forest(X, Y, W, num.trees = 4000)
tau.hat = predict(tau.forest, X.test, estimate.variance = TRUE)
sigma.hat = sqrt(tau.hat$variance.estimates)
# Look at one treatment
plot(X.test[,1], tau.hat$predictions[,1], ylim = range(tau.hat$predictions[,1] + 1.96 * sigma.hat[,1], tau.hat$predictions[,1] - 1.96 * sigma.hat[,1], 0, 2), xlab = "x", ylab = "tau", type = "l")
lines(X.test[,1], tau.hat$predictions[,1] + 1.96 * sigma.hat[,1], col = 1, lty = 2)
lines(X.test[,1], tau.hat$predictions[,1] - 1.96 * sigma.hat[,1], col = 1, lty = 2)
lines(X.test[,1], pmax(0, X.test[,1]), col = 2, lty = 1)
```

### References

Susan Athey, Julie Tibshirani and Stefan Wager.
<b>Generalized Random Forests</b>, 2016.
[<a href="https://arxiv.org/abs/1610.01271">arxiv</a>]
