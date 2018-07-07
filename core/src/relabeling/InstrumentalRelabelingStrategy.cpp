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

#include "commons/utility.h"
#include "relabeling/InstrumentalRelabelingStrategy.h"

InstrumentalRelabelingStrategy::InstrumentalRelabelingStrategy():
        split_regularization(0) {}

InstrumentalRelabelingStrategy::InstrumentalRelabelingStrategy(double split_regularization):
        split_regularization(split_regularization) {}

std::unordered_map<size_t, Eigen::VectorXd> InstrumentalRelabelingStrategy::relabel(
        const std::vector<size_t>& samples,
        const Observations& observations) {

    // Prepare the relevant averages.
    size_t num_samples = samples.size();

    Eigen::VectorXd total_outcome = Eigen::VectorXd::Zero(1);
    Eigen::VectorXd total_treatment = Eigen::VectorXd::Zero(observations.get_num_treatment());
    Eigen::VectorXd total_instrument = Eigen::VectorXd::Zero(observations.get_num_instrument());

    for (size_t sample : samples) {
        total_outcome += observations.get(Observations::OUTCOME, sample);
        total_treatment += observations.get(Observations::TREATMENT, sample);
        total_instrument += observations.get(Observations::INSTRUMENT, sample);
    }


    Eigen::VectorXd average_outcome = total_outcome / num_samples;
    Eigen::VectorXd average_treatment = total_treatment / num_samples;
    Eigen::VectorXd average_instrument = total_instrument / num_samples;
    Eigen::VectorXd average_regularized_instrument = (1 - split_regularization) * average_instrument
                                                     + split_regularization * average_treatment;

    // Calculate the treatment effect.
    Eigen::VectorXd numerator = Eigen::VectorXd::Zero(observations.get_num_instrument());
    Eigen::MatrixXd denominator = Eigen::MatrixXd::Zero(observations.get_num_instrument(), observations.get_num_treatment());

    for (size_t sample : samples) {
        Eigen::VectorXd outcome = observations.get(Observations::OUTCOME, sample);
        Eigen::VectorXd treatment = observations.get(Observations::TREATMENT, sample);
        Eigen::VectorXd instrument = observations.get(Observations::INSTRUMENT, sample);
        Eigen::VectorXd regularized_instrument = (1 - split_regularization) * instrument + split_regularization * treatment;

        numerator += (regularized_instrument - average_regularized_instrument) * (outcome - average_outcome);
        denominator += (regularized_instrument - average_regularized_instrument) * (treatment - average_treatment).transpose();
    }

    if (equal_doubles(denominator.determinant(), 0.0, 1.0e-10)) {
        return std::unordered_map<size_t, Eigen::VectorXd>(); // Signals that we should not perform a split.
    }
    Eigen::VectorXd local_average_treatment_effect = denominator.inverse() * numerator;

    // Create the new outcomes.
    std::unordered_map<size_t, Eigen::VectorXd> relabeled_outcomes;

    for (size_t sample : samples) {
        Eigen::VectorXd response = observations.get(Observations::OUTCOME, sample);
        Eigen::VectorXd treatment = observations.get(Observations::TREATMENT, sample);
        Eigen::VectorXd instrument = observations.get(Observations::INSTRUMENT, sample);
        Eigen::VectorXd regularized_instrument = (1 - split_regularization) * instrument + split_regularization * treatment;

        double residual = (response - average_outcome)(0) - local_average_treatment_effect.dot(treatment - average_treatment);
        relabeled_outcomes[sample] = (regularized_instrument - average_regularized_instrument) * residual;
    }
    return relabeled_outcomes;
}
