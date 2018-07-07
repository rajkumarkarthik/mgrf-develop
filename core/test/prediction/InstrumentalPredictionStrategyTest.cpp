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

#include <map>
#include <unordered_set>
#include <fstream>
#include "commons/Observations.h"
#include "commons/utility.h"
#include "prediction/InstrumentalPredictionStrategy.h"

#include "catch.hpp"

TEST_CASE("flipping signs of treatment flips predictions", "[instrumental, prediction]") {
  //  outcomes: {-9.99984, -7.36924, 5.11211, -0.826997, 0.655345,
//             -5.62082, -9.05911, 3.57729, 3.58593, 8.69386}
//  treatment: {1, 0, 0, 0, 1, 0, 1, 0, 0, 0}
//  instrument: {0, 0, 1, 1, 1, 0, 1, 0, 1, 0}

  Eigen::MatrixXd one = Eigen::MatrixXd::Constant(1, 1, 1);
  std::vector<Eigen::MatrixXd> averages = {-1.1251472*one,0.3*one, 0.5*one, -0.1065444*one, 0.2*one};
  std::vector<Eigen::MatrixXd> flipped_averages = {-1.1251472*one, 0.7*one, 0.5*one, -0.1065444*one, 0.3*one};

  InstrumentalPredictionStrategy prediction_strategy;
  Eigen::VectorXd first_prediction = prediction_strategy.predict(averages);

  Eigen::VectorXd second_prediction = prediction_strategy.predict(flipped_averages);

  REQUIRE(first_prediction.size() == 1);
  REQUIRE(second_prediction.size() == 1);
  REQUIRE(equal_doubles(first_prediction(0), -second_prediction(0), 1.0e-10));
}

TEST_CASE("instrumental variance estimates are positive", "[regression, prediction]") {
  Eigen::MatrixXd one = Eigen::MatrixXd::Constant(1, 1, 1);
  std::vector<Eigen::MatrixXd> averages = {1*one, 0*one, 4.5*one, 2*one, 0.75*one};
  std::vector<std::vector<Eigen::MatrixXd>> leaf_values =
          {{1*one, 1*one, 1*one, 1*one, 1*one}, {2*one, 2*one, 2*one, 2*one, 2*one},
           {-2*one, -3*one, 5*one, -3*one, -1*one}, {1*one, 0*one, 1*one, 2*one, 1*one}};

  InstrumentalPredictionStrategy prediction_strategy;
  Eigen::VectorXd variance = prediction_strategy.compute_variance(
      averages, PredictionValues(leaf_values, 4, 5), 2);

  REQUIRE(variance.size() == 1);
  REQUIRE(variance(0) > 0);
}

TEST_CASE("scaling outcome scales instrumental variance", "[instrumental, prediction]") {
  Eigen::MatrixXd one = Eigen::MatrixXd::Constant(1,1,1);
  std::vector<Eigen::MatrixXd> averages = {1*one, 0*one, 4.5*one, 2*one, 0.75*one};
  std::vector<std::vector<Eigen::MatrixXd>> leaf_values =
      {{1*one, 1*one, 1*one, 1*one, 1*one}, {2*one, 2*one, 2*one, 2*one, 2*one},
       {-2*one, -3*one, 5*one, -3*one, -1*one}, {1*one, 0*one, 1*one, 2*one, 1*one}};

  std::vector<Eigen::MatrixXd> scaled_average = {2*one, 0*one, 4.5*one, 4*one, 0.75*one};
  std::vector<std::vector<Eigen::MatrixXd>> scaled_leaf_values =
      {{2*one, 1*one, 1*one, 2*one, 1*one}, {4*one, 2*one, 2*one, 4*one, 2*one},
       {-4*one, -3*one, 5*one, -6*one, -1*one}, {2*one, 0*one, 1*one, 4*one, 1*one}};

  InstrumentalPredictionStrategy prediction_strategy;
  Eigen::VectorXd first_variance = prediction_strategy.compute_variance(
      averages,
      PredictionValues(leaf_values, 4, 5),
      2);
  Eigen::VectorXd second_variance = prediction_strategy.compute_variance(
      scaled_average,
      PredictionValues(scaled_leaf_values, 4, 5),
      2);

  REQUIRE(first_variance.size() == 1);
  REQUIRE(second_variance.size() == 1);
  REQUIRE(equal_doubles(first_variance(0), second_variance(0) / 4, 10e-10));
}
