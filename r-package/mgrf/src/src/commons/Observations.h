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

#ifndef MGRF_OBSERVATIONS_H
#define MGRF_OBSERVATIONS_H


#include <vector>
#include <string>
#include "eigen3/Eigen/Eigen"

class Observations {
public:
    Observations();

    Observations(const std::vector<Eigen::MatrixXd>& observations_by_type,
                 size_t num_samples);

    Eigen::VectorXd get(std::size_t type, size_t sample) const;

    const std::vector<Eigen::MatrixXd>& get_observations_by_type() const {
        return observations_by_type;
    }

    size_t get_num_samples() const {
        return num_samples;
    }

    size_t get_num_treatment() const {
        return (size_t)observations_by_type[TREATMENT].cols();
    }

    size_t get_num_instrument() const {
        return (size_t)observations_by_type[INSTRUMENT].cols();
    }

    static const std::size_t OUTCOME;
    static const std::size_t TREATMENT;
    static const std::size_t INSTRUMENT;

private:
    std::vector<Eigen::MatrixXd> observations_by_type;
    size_t num_samples;
};


#endif //MGRF_OBSERVATIONS_H
