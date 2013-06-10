// This file is a part of the OpenSurgSim project.
// Copyright 2012-2013, SimQuest Solutions Inc.
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

/// \file
/// Tests for the BasicSceneElement class.

#include <SurgSim/Blocks/BasicSceneElement.h>
#include <SurgSim/Blocks/UnitTests/MockObjects.h>
#include <SurgSim/Framework/BehaviorManager.h>
#include <SurgSim/Framework/Runtime.h>
#include <SurgSim/Framework/Scene.h>
#include <SurgSim/Math/Quaternion.h>
#include <SurgSim/Math/RigidTransform.h>
#include <SurgSim/Math/Vector.h>

#include <gtest/gtest.h>

#include <random>

using SurgSim::Framework::Behavior;
using SurgSim::Math::Quaterniond;
using SurgSim::Math::RigidTransform3d;
using SurgSim::Math::Vector3d;
using SurgSim::Math::makeRigidTransform;

namespace SurgSim
{

namespace Blocks
{

TEST(BasicSceneElementTests, InitTest)
{
	std::shared_ptr<BasicSceneElement> sceneElement = std::make_shared<BasicSceneElement>("test name");

	EXPECT_EQ("test name", sceneElement->getName());
}

TEST(BasicSceneElementTest, InitComponentTest)
{
	std::shared_ptr<SurgSim::Framework::SceneElement> sceneElement = std::make_shared<BasicSceneElement>(
		"SceneElement");

	/// Scene element needs a runtime to initialize
	std::shared_ptr<SurgSim::Framework::Runtime> runtime = std::make_shared<SurgSim::Framework::Runtime>();
	sceneElement->setRuntime(runtime);

	std::shared_ptr<MockRepresentation> representation1 = std::make_shared<MockRepresentation>("TestRepresentation1");
	std::shared_ptr<MockRepresentation> representation2 = std::make_shared<MockRepresentation>("TestRepresentation2");

	sceneElement->addComponent(representation1);
	sceneElement->addComponent(representation2);

	EXPECT_FALSE(representation1->didInit());
	EXPECT_FALSE(representation1->didWakeUp());
	EXPECT_FALSE(representation2->didInit());
	EXPECT_FALSE(representation2->didWakeUp());

	sceneElement->initialize();

	EXPECT_TRUE(representation1->didInit());
	EXPECT_FALSE(representation1->didWakeUp());
	EXPECT_TRUE(representation2->didInit());
	EXPECT_FALSE(representation2->didWakeUp());

	sceneElement->wakeUp();

	EXPECT_TRUE(representation1->didWakeUp());
	EXPECT_TRUE(representation2->didWakeUp());
}

};  // namespace Blocks
};  // namespace SurgSim