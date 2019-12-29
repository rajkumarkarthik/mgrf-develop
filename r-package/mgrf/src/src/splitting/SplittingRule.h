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

#ifndef MGRF_SPLITTINGRULE_H
#define MGRF_SPLITTINGRULE_H

#include <unordered_map>
#include <vector>
#include "eigen3/Eigen/Eigen"

class SplittingRule {
public:
  virtual ~SplittingRule() {}
  virtual bool find_best_split(size_t node,
                               const std::vector<size_t>& possible_split_vars,
                               const std::unordered_map<size_t, Eigen::VectorXd>& responses_by_sample,
                               const std::vector<std::vector<size_t>>& samples,
                               std::vector<size_t>& split_vars,
                               std::vector<double>& split_values) = 0;
};

#endif //MGRF_SPLITTINGRULE_H


