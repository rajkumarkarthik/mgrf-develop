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

#ifndef GRF_PREDICTIONRESULTS_H
#define GRF_PREDICTIONRESULTS_H

#include <cstddef>
#include <vector>
#include "eigen3/Eigen/Eigen"

class Prediction {
public:
  Prediction(const Eigen::MatrixXd& predictions);
  Prediction(const Eigen::MatrixXd& predictions,
             const Eigen::MatrixXd& variance_estimates);

  const Eigen::MatrixXd& get_predictions() const {
    return predictions;
  }

  const Eigen::MatrixXd& get_variance_estimates() const {
    return variance_estimates;
  }

  const bool contains_variance_estimates() const {
    return (bool)variance_estimates.size();
  }

  const size_t size() const {
    return predictions.size();
  }

private:
    Eigen::MatrixXd predictions;
    Eigen::MatrixXd variance_estimates;
};


#endif //GRF_PREDICTIONRESULTS_H
