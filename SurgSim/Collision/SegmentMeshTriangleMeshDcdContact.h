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

#ifndef SURGSIM_COLLISION_SEGMENTMESHTRIANGLEMESHDCDCONTACT_H
#define SURGSIM_COLLISION_SEGMENTMESHTRIANGLEMESHDCDCONTACT_H

#include <memory>

#include "SurgSim/Collision/ContactCalculation.h"
#include "SurgSim/Math/MeshShape.h"
#include "SurgSim/Math/SegmentMeshShape.h"

namespace SurgSim
{
namespace Collision
{

class CollisionPair;

/// Class to calculate intersections between a segment mesh and a triangle mesh
class SegmentMeshTriangleMeshDcdContact : public ContactCalculation
{
public:
	/// Constructor.
	SegmentMeshTriangleMeshDcdContact();

	using ContactCalculation::calculateContact;

	std::pair<int, int> getShapeTypes() override;

	/// Calculate the contacts using the typed shapes directly
	/// \param segmentMeshShape the segment mesh shape
	/// \param triangleMeshShape the triangle mesh shape
	/// \return a list of contacts between the shapes, if any
	std::list<std::shared_ptr<Contact>> calculateContact(const Math::SegmentMeshShape& segmentMeshShape,
		const Math::MeshShape& triangleMeshShape);

private:
	void doCalculateContact(std::shared_ptr<CollisionPair> pair) override;
};

}; // namespace Collision
}; // namespace SurgSim

#endif // SURGSIM_COLLISION_SEGMENTMESHTRIANGLEMESHDCDCONTACT_H