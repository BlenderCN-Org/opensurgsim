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


#include <gtest/gtest.h>

#include <utility>

#include <SurgSim/Physics/UnitTests/RepresentationUtilities.h>
#include <SurgSim/Physics/CollisionRepresentation.h>
#include <SurgSim/Physics/CollisionPair.h>
#include <SurgSim/Physics/ContactConstraintGeneration.h>
#include <SurgSim/Physics/PhysicsManagerState.h>
#include <SurgSim/Physics/ContactCalculation.h>
#include <SurgSim/Physics/Constraint.h>

namespace SurgSim
{
namespace Physics
{

struct ContactConstraintGenerationTests: public ::testing::Test
{
	virtual void SetUp()
	{
		sphere = makeSphereRepresentation(2.0);
		rigid0 = std::make_shared<RigidRepresentation>("Physics Representation");
		sphere->setPhysicsRepresentation(rigid0);

		plane = makePlaneRepresentation();
		rigid1 = std::make_shared<RigidRepresentation>("Physics Representation");
		plane->setPhysicsRepresentation(rigid1);

		state = std::make_shared<PhysicsManagerState>();
	}

	virtual void TearDown()
	{
	}

	std::shared_ptr<CollisionRepresentation> sphere;
	std::shared_ptr<RigidRepresentation> rigid0;

	std::shared_ptr<CollisionRepresentation> plane;
	std::shared_ptr<RigidRepresentation> rigid1;


	std::shared_ptr<PhysicsManagerState> state;

	std::vector<std::shared_ptr<CollisionPair>> pairs;
};

TEST_F(ContactConstraintGenerationTests, BasicTest)
{
	std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>(sphere, plane);
	// Test case setup, create a pair with a contact and set up the physics state with it
	SpherePlaneDcdContact contactCalculation(false);

	contactCalculation.calculateContact(pair);
	ASSERT_TRUE(pair->hasContacts());

	pairs.push_back(pair);

	state->setCollisionPairs(pairs);

	ContactConstraintGeneration generator;
	generator.update(0.1,state);

	ASSERT_EQ(1u, state->getConstraintGroup(CONSTRAINT_GROUP_TYPE_CONTACT).size());

	auto constraints = state->getConstraintGroup(CONSTRAINT_GROUP_TYPE_CONTACT);
	auto constraint = constraints[0];

	auto localizations = constraint->getLocalizations();
	auto implementations = constraint->getImplementations();
	auto data = constraint->getData();

	EXPECT_NE(nullptr, localizations.first);
	EXPECT_NE(nullptr, localizations.second);
	EXPECT_NE(nullptr, implementations.first);
	EXPECT_NE(nullptr, implementations.second);
	EXPECT_NE(nullptr, data);

}

TEST_F(ContactConstraintGenerationTests, CountTest)
{
	std::shared_ptr<CollisionPair> pair;
	SpherePlaneDcdContact contactCalculation(false);

	pair = std::make_shared<CollisionPair>(sphere, plane);
	contactCalculation.calculateContact(pair);
	contactCalculation.calculateContact(pair);

	pairs.push_back(pair);

	pair = std::make_shared<CollisionPair>(sphere, plane);
	contactCalculation.calculateContact(pair);
	pairs.push_back(pair);

	pair = std::make_shared<CollisionPair>(sphere, plane);
	pairs.push_back(pair);


	state->setCollisionPairs(pairs);
	ContactConstraintGeneration generator;
	generator.update(0.1,state);

	// 3 Contacts should generate 3 constraints
	ASSERT_EQ(3u, state->getConstraintGroup(CONSTRAINT_GROUP_TYPE_CONTACT).size());

}
}; // namespace Physics
}; // namespace SurgSim