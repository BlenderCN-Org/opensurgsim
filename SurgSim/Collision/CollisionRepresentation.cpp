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

#include <SurgSim/Collision/CollisionRepresentation.h>
#include <SurgSim/Physics/Representation.h>

namespace SurgSim
{
namespace Collision
{

CollisionRepresentation::CollisionRepresentation(const std::string& name) :
	Representation(name)
{

}

CollisionRepresentation::~CollisionRepresentation()
{

}

void CollisionRepresentation::setInitialPose(const SurgSim::Math::RigidTransform3d& pose)
{
	SURGSIM_FAILURE() << "Cannot set an initial pose on a collision representation.";
}

const SurgSim::Math::RigidTransform3d& CollisionRepresentation::getInitialPose() const
{
	SURGSIM_FAILURE() << "No initial pose on a collision representation.";
	return SurgSim::Math::RigidTransform3d::Identity();
}

}; // namespace Collision
}; // namespace SurgSim
