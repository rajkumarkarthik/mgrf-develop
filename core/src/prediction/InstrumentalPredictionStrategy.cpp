/*-------------------------------------------------------------------------------
  This file is part of generalized random forest (grf).

  grf is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  grf is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with grf. If not, see <http://www.gnu.org/licenses/>.
 #-------------------------------------------------------------------------------*/

#include <cmath>
#include <string>
#include <vector>

#include "commons/Observations.h"
#include "commons/utility.h"
#include "prediction/InstrumentalPredictionStrategy.h"

const std::size_t InstrumentalPredictionStrategy::OUTCOME = 0;
const std::size_t InstrumentalPredictionStrategy::TREATMENT = 1;
const std::size_t InstrumentalPredictionStrategy::INSTRUMENT = 2;
const std::size_t InstrumentalPredictionStrategy::OUTCOME_INSTRUMENT = 3;
const std::size_t InstrumentalPredictionStrategy::TREATMENT_INSTRUMENT = 4;

const std::size_t NUM_TYPES = 5;

size_t InstrumentalPredictionStrategy::prediction_length() {
    return prediction_size;
}

Eigen::VectorXd InstrumentalPredictionStrategy::predict(const std::vector<Eigen::MatrixXd>& average) {
  prediction_size = average.at(INSTRUMENT).size();
  Eigen::VectorXd instrument_effect = average.at(OUTCOME_INSTRUMENT) - average.at(INSTRUMENT) * average.at(OUTCOME);
  Eigen::MatrixXd first_stage = average.at(TREATMENT_INSTRUMENT) - average.at(INSTRUMENT) * average.at(TREATMENT).transpose();

  return first_stage.inverse() * instrument_effect;
}

Eigen::VectorXd InstrumentalPredictionStrategy::compute_variance(
    const std::vector<Eigen::MatrixXd>& average,
    const PredictionValues& leaf_values,
    uint ci_group_size) {

  Eigen::VectorXd instrument_effect = average.at(OUTCOME_INSTRUMENT) - average.at(INSTRUMENT) * average.at(OUTCOME);
  Eigen::MatrixXd first_stage = average.at(TREATMENT_INSTRUMENT) - average.at(INSTRUMENT) * average.at(TREATMENT).transpose();
  Eigen::VectorXd treatment_estimate = first_stage.inverse() * instrument_effect;
  double main_effect  = (average.at(OUTCOME) - treatment_estimate.transpose() * average.at(TREATMENT))(0, 0);

  double num_good_groups = 0;
  size_t num_W = first_stage.cols();
  Eigen::MatrixXd psi_squared = Eigen::MatrixXd::Zero(num_W + 1, num_W + 1);
  Eigen::MatrixXd psi_grouped_squared = Eigen::MatrixXd::Zero(num_W + 1, num_W + 1);

  for (size_t group = 0; group < leaf_values.get_num_nodes() / ci_group_size; ++group) {
    bool good_group = true;
    for (size_t j = 0; j < ci_group_size; ++j) {
      if (leaf_values.empty(group * ci_group_size + j)) {
        good_group = false;
      }
    }
    if (!good_group) continue;

    num_good_groups++;

    Eigen::VectorXd group_psi_1 = Eigen::VectorXd::Zero(num_W);
    double group_psi_2 = 0;

    for (size_t j = 0; j < ci_group_size; ++j) {

      size_t i = group * ci_group_size + j;
      const std::vector<Eigen::MatrixXd>& leaf_value = leaf_values.get_values(i);

      Eigen::VectorXd psi_1 = leaf_value.at(OUTCOME_INSTRUMENT)
                     - leaf_value.at(TREATMENT_INSTRUMENT) * treatment_estimate
                     - leaf_value.at(INSTRUMENT) * main_effect;
      double psi_2 = (leaf_value.at(OUTCOME)
                     - leaf_value.at(TREATMENT).transpose() * treatment_estimate)(0,0)
                     - main_effect;

      psi_squared.topLeftCorner(num_W, num_W) += psi_1 * psi_1.transpose();
      psi_squared.topRightCorner(num_W, 1) += psi_1 * psi_2;
      psi_squared.bottomLeftCorner(1, num_W) += psi_1.transpose() * psi_2;
      psi_squared.bottomRightCorner(1, 1) += psi_2 * psi_2 * Eigen::MatrixXd::Identity(1, 1);

      group_psi_1 += psi_1;
      group_psi_2 += psi_2;
    }

    group_psi_1 /= ci_group_size;
    group_psi_2 /= ci_group_size;

    psi_grouped_squared.topLeftCorner(num_W, num_W) += group_psi_1 * group_psi_1.transpose();
    psi_grouped_squared.topRightCorner(num_W, 1) += group_psi_1 * group_psi_2;
    psi_grouped_squared.bottomLeftCorner(1, num_W) += group_psi_1.transpose() * group_psi_2;
    psi_grouped_squared.bottomRightCorner(1, 1) += group_psi_2 * group_psi_2 * Eigen::MatrixXd::Identity(1, 1);

  }

  psi_squared /= (num_good_groups * ci_group_size);
  psi_grouped_squared /= num_good_groups;


  Eigen::MatrixXd bread (num_W + 1, num_W + 1);
  bread.topLeftCorner(num_W, num_W) = average.at(TREATMENT_INSTRUMENT);
  bread.topRightCorner(num_W, 1) = average.at(INSTRUMENT);
  bread.bottomLeftCorner(1, num_W) = average.at(TREATMENT).transpose();
  bread.bottomRightCorner(1, 1) = Eigen::MatrixXd::Identity(1, 1);
  bread = bread.inverse();

  Eigen::MatrixXd var_between = bread * psi_grouped_squared * bread.transpose();
  Eigen::MatrixXd var_total = bread * psi_squared * bread.transpose();

  // This is the amount by which var_between is inflated due to using small groups
  Eigen::MatrixXd group_noise = (var_total - var_between) / (ci_group_size - 1);

  // A simple variance correction, would be to use:
  // var_debiased = var_between - group_noise.
  // However, this may be biased in small samples; we do an objective
  // Bayes analysis of variance instead to avoid negative values.

  // For the multivariate version, we do an element-by-element debiasing.

  Eigen::VectorXd var_debiased (num_W);
  for (int Wi = 0; Wi < num_W; ++Wi) {
    var_debiased(Wi) = bayes_debiaser.debias(var_between(Wi, Wi), group_noise(Wi, Wi), num_good_groups);
  }
  return var_debiased;
}

size_t InstrumentalPredictionStrategy::prediction_value_length() {
  return NUM_TYPES;
}

PredictionValues InstrumentalPredictionStrategy::precompute_prediction_values(
    const std::vector<std::vector<size_t>>& leaf_samples,
    const Observations& observations) {
  size_t num_leaves = leaf_samples.size();

  std::vector<std::vector<Eigen::MatrixXd>> values(num_leaves);

  for (size_t i = 0; i < leaf_samples.size(); ++i) {
    size_t leaf_size = leaf_samples[i].size();
    if (leaf_size == 0) {
      continue;
    }

    std::vector<Eigen::MatrixXd>& value = values[i];
    value.resize(NUM_TYPES);

    Eigen::VectorXd sum_Y = Eigen::VectorXd::Zero(1);
    Eigen::VectorXd sum_W = Eigen::VectorXd::Zero(observations.get_num_treatment());
    Eigen::VectorXd sum_Z = Eigen::VectorXd::Zero(observations.get_num_instrument());
    Eigen::VectorXd sum_YZ = Eigen::VectorXd::Zero(observations.get_num_instrument());
    Eigen::MatrixXd sum_WZ = Eigen::MatrixXd::Zero(observations.get_num_instrument(), observations.get_num_treatment());

    for (auto& sample : leaf_samples[i]) {
      sum_Y += observations.get(Observations::OUTCOME, sample);
      sum_W += observations.get(Observations::TREATMENT, sample);
      sum_Z += observations.get(Observations::INSTRUMENT, sample);
      sum_YZ += observations.get(Observations::INSTRUMENT, sample) * observations.get(Observations::OUTCOME, sample);
      sum_WZ += observations.get(Observations::INSTRUMENT, sample) * observations.get(Observations::TREATMENT, sample).transpose();
    }

    value[OUTCOME] = sum_Y / leaf_size;
    value[TREATMENT] = sum_W / leaf_size;
    value[INSTRUMENT] = sum_Z / leaf_size;
    value[OUTCOME_INSTRUMENT] = sum_YZ / leaf_size;
    value[TREATMENT_INSTRUMENT] = sum_WZ / leaf_size;
  }

  return PredictionValues(values, num_leaves, NUM_TYPES);
}
