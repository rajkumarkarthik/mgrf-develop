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

#ifndef MGRF_FORESTSERIALIZER_H
#define MGRF_FORESTSERIALIZER_H

#include <memory>

#include "forest/Forest.h"
#include "serialization/ObservationsSerializer.h"
#include "serialization/TreeSerializer.h"

class ForestSerializer {
public:
  void serialize(std::ostream& stream, const Forest& forest);
  Forest deserialize(std::istream& stream);

private:
  ObservationsSerializer observations_serializer;
  TreeSerializer tree_serializer;
};


#endif //MGRF_FORESTSERIALIZER_H
