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

#ifndef SURGSIM_COLLISION_OCTREEDCDCONTACT_INL_H
#define SURGSIM_COLLISION_OCTREEDCDCONTACT_INL_H

#include "SurgSim/Collision/CollisionPair.h"
#include "SurgSim/Collision/Representation.h"
#include "SurgSim/Collision/ShapeCollisionRepresentation.h"
#include "SurgSim/Math/BoxShape.h"
#include "SurgSim/Math/OctreeShape.h"
#include "SurgSim/Math/Shape.h"
#include "SurgSim/Math/Vector.h"


namespace SurgSim
{
namespace Collision
{

template <class ContactCalculation, class OctreeNodeData>
OctreeDcdContact<ContactCalculation, OctreeNodeData>::OctreeDcdContact()
{
	SURGSIM_ASSERT(m_contactCalculator.getShapeTypes().first == SurgSim::Math::SHAPE_TYPE_BOX) <<
		"OctreeDcdContact needs a contact calculator that works with Boxes";
	m_shapeTypes = m_contactCalculator.getShapeTypes();
	m_shapeTypes.first = SurgSim::Math::SHAPE_TYPE_OCTREE;
}

template <class ContactCalculation, class OctreeNodeData>
std::pair<int, int> OctreeDcdContact<ContactCalculation, OctreeNodeData>::getShapeTypes()
{
	return m_shapeTypes;
}

template <class ContactCalculation, class OctreeNodeData>
void OctreeDcdContact<ContactCalculation, OctreeNodeData>::doCalculateContact(std::shared_ptr<CollisionPair> pair)
{
	typedef SurgSim::Math::OctreeShape<OctreeNodeData> OctreeShapeType;
	std::shared_ptr<OctreeShapeType> octree = std::static_pointer_cast<OctreeShapeType>(pair->getFirst()->getShape());
	calculateContactWithNode(octree->getRootNode(), pair, SurgSim::Math::OctreePath());
}

template <class ContactCalculation, class OctreeNodeData>
void OctreeDcdContact<ContactCalculation, OctreeNodeData>::calculateContactWithNode(
	std::shared_ptr<SurgSim::DataStructures::OctreeNode<OctreeNodeData>> node, std::shared_ptr<CollisionPair> pair,
	SurgSim::Math::OctreePath nodePath)
{
	if (! node->isActive())
		return;

	SurgSim::Math::Vector3d boxSize = node->getBoundingBox().sizes();
	std::shared_ptr<SurgSim::Math::Shape> boxShape =
		std::make_shared<SurgSim::Math::BoxShape>(boxSize.x(), boxSize.y(), boxSize.z());
	SurgSim::Math::RigidTransform3d boxPose = pair->getFirst()->getPose();
	boxPose.translation() += boxPose.linear() * node->getBoundingBox().center();

	std::shared_ptr<Representation> box =
		std::make_shared<ShapeCollisionRepresentation>("Octree Node", boxShape, boxPose);

	std::shared_ptr<CollisionPair> localPair = std::make_shared<CollisionPair>(box, pair->getSecond());
	m_contactCalculator.calculateContact(localPair);
	if (localPair->hasContacts())
	{
		if (node->hasChildren())
		{
			for (size_t i = 0; i < node->getChildren().size(); i++)
			{
				SurgSim::Math::OctreePath childsPath = nodePath;
				childsPath.push_back(i);
				calculateContactWithNode(node->getChild(i), pair, childsPath);
			}
		}
		else
		{
			const std::list<std::shared_ptr<Contact>>& newContacts = localPair->getContacts();
			for(auto contact=newContacts.cbegin(); contact!=newContacts.cend(); ++contact)
			{
				(*contact)->penetrationPoints.first.octreeNodePath.setValue(nodePath);
				pair->addContact(*contact);
			}
		}
	}
}

};
};

#endif
