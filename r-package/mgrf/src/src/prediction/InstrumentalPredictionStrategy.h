/*-------------------------------------------------------------------------------
  This file is part of generalized random forest (mgrf).

  mgrf is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  mgrf is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with mgrf. If not, see <http://www.gnu.org/licenses/>.
 #-------------------------------------------------------------------------------*/

#ifndef MGRF_INSTRUMENTALPREDICTIONSTRATEGY_H
#define MGRF_INSTRUMENTALPREDICTIONSTRATEGY_H


#include <cstddef>
#include <unordered_map>
#include "commons/DefaultData.h"
#include "prediction/Prediction.h"
#include "prediction/OptimizedPredictionStrategy.h"
#include "prediction/PredictionValues.h"
#include "ObjectiveBayesDebiaser.h"

class InstrumentalPredictionStrategy: public OptimizedPredictionStrategy {
public:
  static const std::size_t OUTCOME;
  static const std::size_t TREATMENT;
  static const std::size_t INSTRUMENT;
  static const std::size_t OUTCOME_INSTRUMENT;
  static const std::size_t TREATMENT_INSTRUMENT;

  size_t prediction_value_length();
  PredictionValues precompute_prediction_values(
      const std::vector<std::vector<size_t>>& leaf_samples,
      const Observations& observations);

  size_t prediction_length();
  Eigen::VectorXd predict(const std::vector<Eigen::MatrixXd>& average);

    Eigen::VectorXd compute_variance(const std::vector<Eigen::MatrixXd>& average,
                          const PredictionValues& leaf_values,
                          uint ci_group_size);

private:
  ObjectiveBayesDebiaser bayes_debiaser;
    size_t prediction_size;
};

#endif //MGRF_INSTRUMENTALPREDICTIONSTRATEGY_H
