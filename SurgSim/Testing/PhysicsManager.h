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

#ifndef SURGSIM_TESTING_PHYSICSMANAGER_H
#define SURGSIM_TESTING_PHYSICSMANAGER_H

#include "SurgSim/Physics/PhysicsManager.h"

namespace SurgSim
{
namespace Testing
{

class PhysicsManager : public SurgSim::Physics::PhysicsManager
{
public:
	virtual bool executeAdditions(const std::shared_ptr<SurgSim::Framework::Component>& component) override
	{
		return SurgSim::Physics::PhysicsManager::executeAdditions(component);
	}

	virtual bool executeRemovals(const std::shared_ptr<SurgSim::Framework::Component>& component) override
	{
		return SurgSim::Physics::PhysicsManager::executeRemovals(component);
	}

	virtual bool doInitialize() override
	{
		return SurgSim::Physics::PhysicsManager::doInitialize();
	}

	virtual bool doStartUp() override
	{
		return SurgSim::Physics::PhysicsManager::doStartUp();
	}

	virtual bool doUpdate(double dt)
	{
		return SurgSim::Physics::PhysicsManager::doUpdate(dt);
	}
};

} // namespace Physics
} // namespace SurgSim

#endif // SURGSIM_TESTING_PERFORMANCETESTINGPHYSICSMANAGER_H
