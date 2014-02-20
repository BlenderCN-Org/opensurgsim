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

#include <vector>

#include "SurgSim/Physics/DcdCollision.h"
#include "SurgSim/Collision/CollisionPair.h"
#include "SurgSim/Collision/ContactCalculation.h"
#include "SurgSim/Collision/DcdCollision.h"
#include "SurgSim/Collision/Representation.h"
#include "SurgSim/Physics/PhysicsManagerState.h"

using SurgSim::Collision::CollisionPair;

namespace SurgSim
{
namespace Physics
{

DcdCollision::DcdCollision(bool doCopyState) : Computation(doCopyState)
{
	populateCalculationTable();
}

DcdCollision::~DcdCollision()
{
}

std::shared_ptr<PhysicsManagerState> DcdCollision::doUpdate(
	const double& dt,
	const std::shared_ptr<PhysicsManagerState>& state)
{
	std::shared_ptr<PhysicsManagerState> result = state;
	updatePairs(result);

	std::vector<std::shared_ptr<CollisionPair>> pairs = result->getCollisionPairs();
	auto it = pairs.cbegin();
	auto itEnd = pairs.cend();
	while (it != itEnd)
	{
		m_contactCalculations[(*it)->getFirst()->getShapeType()][(*it)->getSecond()->getShapeType()]->
			calculateContact(*it);
		++it;
	}
	return result;
}

void DcdCollision::populateCalculationTable()
{
	for (int i = 0; i < SurgSim::Math::SHAPE_TYPE_COUNT; ++i)
	{
		for (int j = 0; j < SurgSim::Math::SHAPE_TYPE_COUNT; ++j)
		{
			m_contactCalculations[i][j].reset(new SurgSim::Collision::DefaultContactCalculation(false));
		}
	}
	setDcdContactInTable(std::make_shared<SurgSim::Collision::SphereSphereDcdContact>());
	setDcdContactInTable(std::make_shared<SurgSim::Collision::SphereDoubleSidedPlaneDcdContact>());
	setDcdContactInTable(std::make_shared<SurgSim::Collision::SpherePlaneDcdContact>());
	setDcdContactInTable(std::make_shared<SurgSim::Collision::BoxCapsuleDcdContact>());
	setDcdContactInTable(std::make_shared<SurgSim::Collision::BoxDoubleSidedPlaneDcdContact>());
	setDcdContactInTable(std::make_shared<SurgSim::Collision::BoxPlaneDcdContact>());
	setDcdContactInTable(std::make_shared<SurgSim::Collision::BoxSphereDcdContact>());
	setDcdContactInTable(std::make_shared<SurgSim::Collision::CapsuleSphereDcdContact>());

	// Add the Octree contact calculations using the box contact calculations
	setDcdContactInTable(std::make_shared<SurgSim::Collision::OctreeDcdContact>(
				std::make_shared<SurgSim::Collision::BoxCapsuleDcdContact>()));
	setDcdContactInTable(std::make_shared<SurgSim::Collision::OctreeDcdContact>(
				std::make_shared<SurgSim::Collision::BoxDoubleSidedPlaneDcdContact>()));
	setDcdContactInTable(std::make_shared<SurgSim::Collision::OctreeDcdContact>(
				std::make_shared<SurgSim::Collision::BoxPlaneDcdContact>()));
	setDcdContactInTable(std::make_shared<SurgSim::Collision::OctreeDcdContact>(
				std::make_shared<SurgSim::Collision::BoxSphereDcdContact>()));
}

void DcdCollision::updatePairs(std::shared_ptr<PhysicsManagerState> state)
{
	std::vector<std::shared_ptr<SurgSim::Collision::Representation>> representations =
		state->getCollisionRepresentations();

	if (representations.size() > 1)
	{
		std::vector<std::shared_ptr<CollisionPair>> pairs;
		auto firstEnd = std::end(representations);
		--firstEnd;
		for (auto first = std::begin(representations); first != firstEnd; ++first)
		{
			auto second = first;
			++second;
			for (; second != std::end(representations); ++second)
			{
				std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>();
				pair->setRepresentations(*first,*second);
				pairs.push_back(pair);
			}
		}
		state->setCollisionPairs(pairs);
	}
}

void DcdCollision::setDcdContactInTable(std::shared_ptr<SurgSim::Collision::ContactCalculation> dcdContact)
{
	std::pair<int,int> shapeTypes = dcdContact->getShapeTypes();
	m_contactCalculations[shapeTypes.first][shapeTypes.second] = dcdContact;
	if(shapeTypes.first != shapeTypes.second)
	{
		m_contactCalculations[shapeTypes.second][shapeTypes.first] = dcdContact;
	}
}

}; // Physics
}; // SurgSim

