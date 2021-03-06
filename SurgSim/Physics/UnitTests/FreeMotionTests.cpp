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

/// \file FreeMotionTests.cpp
/// Simple Test for FreeMotion calculation

#include <gtest/gtest.h>

#include <string>
#include <memory>

#include "SurgSim/Framework/Runtime.h"
#include "SurgSim/Math/Quaternion.h"
#include "SurgSim/Math/RigidTransform.h"
#include "SurgSim/Math/SphereShape.h"
#include "SurgSim/Math/Vector.h"
#include "SurgSim/Physics/PhysicsManager.h"
#include "SurgSim/Physics/FixedRepresentation.h"
#include "SurgSim/Physics/RigidRepresentation.h"
#include "SurgSim/Physics/FreeMotion.h"
#include "SurgSim/Physics/PhysicsManagerState.h"

using SurgSim::Math::SphereShape;
using SurgSim::Math::Vector3d;

using SurgSim::Physics::Representation;
using SurgSim::Physics::RigidRepresentation;
using SurgSim::Physics::FreeMotion;
using SurgSim::Physics::PhysicsManagerState;

TEST(FreeMotionTest, RunTest)
{
	std::vector<std::shared_ptr<Representation>> representations = std::vector<std::shared_ptr<Representation>>();
	std::shared_ptr<RigidRepresentation> representation = std::make_shared<RigidRepresentation>("TestSphere");

	representation->setDensity(700.0); // Wood
	std::shared_ptr<SphereShape> shape = std::make_shared<SphereShape>(0.01); // 1cm Sphere
	representation->setShape(shape);

	representations.push_back(representation);

	std::shared_ptr<PhysicsManagerState> state = std::make_shared<PhysicsManagerState>();
	state->setRepresentations(representations);

	FreeMotion computation;

	representation->setIsGravityEnabled(false);
	EXPECT_TRUE(representation->getCurrentState().getPose().translation().isZero());
	state = computation.update(1.0,state);
	EXPECT_TRUE(representation->getCurrentState().getPose().translation().isZero());

	representation->setIsGravityEnabled(true);
	state = computation.update(1.0,state);
	EXPECT_FALSE(representation->getCurrentState().getPose().translation().isZero());
}
