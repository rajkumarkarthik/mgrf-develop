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
#include "forest/ForestPredictor.h"
#include "forest/ForestPredictors.h"
#include "forest/ForestTrainer.h"
#include "forest/ForestTrainers.h"
#include "utilities/ForestTestUtilities.h"

#include "catch.hpp"


TEST_CASE("honest regression forests are shift invariant", "[regression, forest]") {
  // Run the original forest.
  Data* data = load_data("test/forest/resources/gaussian_data.csv");
  uint outcome_index = 10;
  double alpha = 0.10;

  ForestTrainer trainer = ForestTrainers::regression_trainer(data, outcome_index, alpha);
  ForestTestUtilities::init_honest_trainer(trainer);

  Forest forest = trainer.train(data);
  ForestPredictor predictor = ForestPredictors::regression_predictor(4, 1);
  std::vector<Prediction> predictions = predictor.predict_oob(forest, data);

  // Shift each outcome by 1, and re-run the forest.
  bool error;
  for (size_t r = 0; r < data->get_num_rows(); r++) {
    double outcome = data->get(r, outcome_index);
    data->set(outcome_index, r, outcome + 1, error);
  }

  ForestTrainer shifted_trainer = ForestTrainers::regression_trainer(data, outcome_index, alpha);
  ForestTestUtilities::init_default_trainer(shifted_trainer);

  Forest shifted_forest = trainer.train(data);
  ForestPredictor shifted_predictor = ForestPredictors::regression_predictor(4, 1);
  std::vector<Prediction> shifted_predictions = shifted_predictor.predict_oob(shifted_forest, data);

  REQUIRE(predictions.size() == shifted_predictions.size());
  double delta = 0.0;
  for (size_t i = 0; i < predictions.size(); i++) {
    Prediction prediction = predictions[i];
    Prediction shifted_prediction = shifted_predictions[i];

    double value = prediction.get_predictions()(0,0);
    double shifted_value = shifted_prediction.get_predictions()(0,0);

    delta += shifted_value - value;
  }

  REQUIRE(equal_doubles(delta / predictions.size(), 1, 1e-1));

  delete data;
}

TEST_CASE("regression forests give reasonable variance estimates", "[regression, forest]") {
  Data* data = load_data("test/forest/resources/gaussian_data.csv");
  uint outcome_index = 10;
  double alpha = 0.10;

  ForestTrainer trainer = ForestTrainers::regression_trainer(data, outcome_index, alpha);
  ForestTestUtilities::init_trainer(trainer, false, 2);

  Forest forest = trainer.train(data);
  ForestPredictor predictor = ForestPredictors::regression_predictor(4, 2);
  std::vector<Prediction> predictions = predictor.predict_oob(forest, data);

  for (size_t i = 0; i < predictions.size(); i++) {
    Prediction prediction = predictions[i];
    REQUIRE(prediction.contains_variance_estimates());

    double variance_estimate = prediction.get_variance_estimates()(0,0);
    REQUIRE(variance_estimate > 0);
  }

  delete data;
}

// Karthik test (to be deleted)

TEST_CASE("karthik test") {
  Data* data = load_data("test/forest/resources/karthik4.csv");

  ForestTrainer trainer = ForestTrainers::instrumental_trainer(data, 61, {62}, {62}, 0, 0.05);

  uint mtry = 1;
  uint num_trees = 2000;
  uint seed = 42;
  uint num_threads = 8;
  uint min_node_size = 1;
  std::set<size_t> no_split_variables;
  std::string split_select_weights_file = "";
  bool sample_with_replacement = true;
  std::string sample_weights_file = "";
  double sample_fraction = 0.5;
  bool honesty = true;
  uint ci_group_size = 2;

  trainer.init(mtry, num_trees, seed, num_threads,
               min_node_size, no_split_variables, split_select_weights_file,
               sample_with_replacement, sample_weights_file, sample_fraction,
               honesty, ci_group_size);

  Forest forest = trainer.train(data);
  ForestPredictor predictor = ForestPredictors::instrumental_predictor(num_threads, ci_group_size);
  std::vector<Prediction> predictions = predictor.predict_oob(forest, data);

  std::ofstream karFile("test/forest/resources/karthik_w4.csv");
  for (auto prediction : predictions) {
    karFile << prediction.get_predictions().transpose() << " " << prediction.get_variance_estimates().transpose() << "\n";
  }

}
