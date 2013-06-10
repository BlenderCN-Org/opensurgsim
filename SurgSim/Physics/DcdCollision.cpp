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

#include <SurgSim/Physics/DcdCollision.h>
#include <SurgSim/Physics/CollisionRepresentation.h>
#include <SurgSim/Physics/RigidActorCollisionRepresentation.h>
#include <SurgSim/Physics/RigidActor.h>
#include <SurgSim/Physics/CollisionPair.h>
#include <SurgSim/Physics/ContactCalculation.h>
#include <SurgSim/Math/RigidTransform.h>
#include <SurgSim/Math/Vector.h>

namespace SurgSim
{
namespace Physics
{

DcdCollision::DcdCollision()
{
	populateCalculationTable();
}

DcdCollision::~DcdCollision()
{
	m_pairs.clear();
}

std::shared_ptr<PhysicsManagerState> DcdCollision::doUpdate(double dt, std::shared_ptr<PhysicsManagerState> state)
{
	std::shared_ptr<PhysicsManagerState> result = std::make_shared<PhysicsManagerState>(*state);

	updatePairs(result);

	std::vector<std::shared_ptr<CollisionPair>> pairs = result->getCollisionPairs();
	auto it = m_pairs.cbegin();
	auto itEnd = m_pairs.cend();
	while (it != itEnd)
	{
		int i = (*it)->getFirst()->getShapeType();
		int j = (*it)->getSecond()->getShapeType();
		m_contactCalculations[i][j]->calculateContact(*it);
		++it;
	}
	return result;
}

void DcdCollision::populateCalculationTable()
{
	for (int i = 0; i < RIGID_SHAPE_TYPE_COUNT; ++i)
	{
		for (int j = 0; j < RIGID_SHAPE_TYPE_COUNT; ++j)
		{
			m_contactCalculations[i][j].reset(new DefaultContactCalculation(false));
		}
	}
	m_contactCalculations[RIGID_SHAPE_TYPE_SPHERE][RIGID_SHAPE_TYPE_SPHERE].reset(new SphereSphereDcdContact());
	m_contactCalculations[RIGID_SHAPE_TYPE_SPHERE][RIGID_SHAPE_TYPE_PLANE].reset(new SpherePlaneDcdContact(false));
	m_contactCalculations[RIGID_SHAPE_TYPE_PLANE][RIGID_SHAPE_TYPE_SPHERE].reset(new SpherePlaneDcdContact(true));
}

void DcdCollision::updatePairs(std::shared_ptr<PhysicsManagerState> state)
{
	std::vector<std::shared_ptr<Actor>> actors = state->getActors();
	std::list<std::shared_ptr<RigidActor>> rigidActors;

	if (actors.size() > 1)
	{
		for (auto it = actors.cbegin(); it != actors.cend(); ++it)
		{
			std::shared_ptr<RigidActor> rigid = std::dynamic_pointer_cast<RigidActor>(*it);
			if (rigid != nullptr && rigid->isActive())
			{
				rigidActors.push_back(rigid);
			}
		}
	}

	if (rigidActors.size() > 1)
	{
		std::vector<std::shared_ptr<CollisionPair>> pairs;
		auto firstEnd = rigidActors.end();
		--firstEnd;
		for (auto first = rigidActors.begin(); first != firstEnd; ++first)
		{
			std::list<std::shared_ptr<RigidActor>>::iterator second = first;
			++second;
			for (; second != rigidActors.end(); ++second)
			{
				std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>();
				pair->setRepresentations(std::make_shared<RigidActorCollisionRepresentation>(*first),
					std::make_shared<RigidActorCollisionRepresentation>(*second));
				pairs.push_back(pair);
			}
		}
		state->setCollisionPairs(pairs);
	}
}

}; // Physics
}; // SurgSim
