// This file is a part of the OpenSurgSim project.
// Copyright 2013, SimQuest Solutions Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SURGSIM_PHYSICS_PHYSICSCONVERT_H
#define SURGSIM_PHYSICS_PHYSICSCONVERT_H

#include <yaml-cpp/yaml.h>

namespace SurgSim
{
namespace Physics
{
class RigidRepresentationParameters;
class RigidRepresentationState;
}
}

namespace YAML
{

template <>
struct convert<SurgSim::Physics::RigidRepresentationState>
{
	static Node encode(const SurgSim::Physics::RigidRepresentationState& rhs);
	static bool decode(const Node& node, SurgSim::Physics::RigidRepresentationState& rhs);
};


template <>
struct convert<SurgSim::Physics::RigidRepresentationParameters>
{
	static Node encode(const SurgSim::Physics::RigidRepresentationParameters& rhs);
	static bool decode(const Node& node, SurgSim::Physics::RigidRepresentationParameters& rhs);
};

}; // namespace YAML

#endif // SURGSIM_PHYSICS_PHYSICSCONVERT_H