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

#include "SurgSim/Collision/OctreePlaneDcdContact.h"

using SurgSim::Math::PlaneShape;


namespace SurgSim
{
namespace Collision
{

OctreePlaneDcdContact::OctreePlaneDcdContact()
{
}

OctreePlaneDcdContact::~OctreePlaneDcdContact()
{
}

std::pair<int, int> OctreePlaneDcdContact::getShapeTypes()
{
	return std::pair<int,int>(SurgSim::Math::SHAPE_TYPE_OCTREE, SurgSim::Math::SHAPE_TYPE_PLANE);
}

std::list<std::shared_ptr<Contact>> OctreePlaneDcdContact::boxContactCalculation(
		const SurgSim::Math::BoxShape& boxShape, const SurgSim::Math::RigidTransform3d& boxPose,
		const SurgSim::Math::Shape& otherShape, const SurgSim::Math::RigidTransform3d& otherPose)
{
	return m_calculator.calculateContact(boxShape, boxPose, static_cast<const PlaneShape&>(otherShape), otherPose);
}

};
};
