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
/// Render Tests for the OsgSphereActor class.

#include <SurgSim/Graphics/OsgManager.h>
#include <SurgSim/Graphics/OsgSphereActor.h>
#include <SurgSim/Graphics/OsgViewElement.h>
#include <SurgSim/Framework/Scene.h>
#include <SurgSim/Framework/SceneElement.h>
#include <SurgSim/Framework/Runtime.h>
#include <SurgSim/Math/Quaternion.h>
#include <SurgSim/Math/Vector.h>

#include <gtest/gtest.h>

#include <random>

using SurgSim::Framework::Runtime;
using SurgSim::Framework::Scene;
using SurgSim::Framework::SceneElement;
using SurgSim::Math::Quaterniond;
using SurgSim::Math::RigidTransform3d;
using SurgSim::Math::Vector3d;
using SurgSim::Math::makeRigidTransform;

namespace SurgSim
{

namespace Graphics
{

TEST(OsgSphereActorRenderTests, MovingSpheresTest)
{
	/// Initial sphere 1 position
	Vector3d startPosition1(-0.1, 0.0, -0.2);
	/// Final sphere 1 position
	Vector3d endPosition1(0.1, 0.0, -0.2);
	/// Initial sphere 1 radius;
	double startRadius1 = 0.001;
	/// Final sphere 1 radius;
	double endRadius1 = 0.01;
	/// Initial sphere 2 position
	Vector3d startPosition2(0.0, -0.1, -0.2);
	/// Final sphere 2 position
	Vector3d endPosition2(0.0, 0.1, -0.2);
	/// Initial sphere 2 radius;
	double startRadius2 = 0.01;
	/// Final sphere 2 radius;
	double endRadius2 = 0.05;

	/// Number of times to step the sphere position and radius from start to end.
	/// This number of steps will be done in 1 second.
	int numSteps = 100;

	std::shared_ptr<Runtime> runtime = std::make_shared<Runtime>();
	std::shared_ptr<OsgManager> manager = std::make_shared<OsgManager>();

	runtime->addManager(manager);

	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	runtime->setScene(scene);

	/// Add a graphics view element to the scene
	std::shared_ptr<OsgViewElement> viewElement = std::make_shared<OsgViewElement>("view element");
	scene->addSceneElement(viewElement);

	/// Add the two sphere actors to the view element so we don't need to make another concrete scene element
	/// \todo	DK-2013-june-03	Use the BasicSceneElement when it gets moved into Framework.
	std::shared_ptr<SphereActor> sphereActor1 = std::make_shared<OsgSphereActor>("sphere actor 1");
	viewElement->addComponent(sphereActor1);
	std::shared_ptr<SphereActor> sphereActor2 = std::make_shared<OsgSphereActor>("sphere actor 2");
	viewElement->addComponent(sphereActor2);

	/// Run the thread
	runtime->start();
	EXPECT_TRUE(manager->isInitialized());
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	for (int i = 0; i < numSteps; ++i)
	{
		/// Calculate t in [0.0, 1.0]
		double t = static_cast<double>(i) / numSteps;
		/// Interpolate position and radius
		sphereActor1->setPose(makeRigidTransform(Quaterniond::Identity(), (1 - t) * startPosition1 + t * endPosition1));
		sphereActor1->setRadius((1 - t) * startRadius1 + t * endRadius1);
		sphereActor2->setPose(makeRigidTransform(Quaterniond::Identity(), (1 - t) * startPosition2 + t * endPosition2));
		sphereActor2->setRadius((1 - t) * startRadius2 + t * endRadius2);
		/// The total number of steps should complete in 1 second
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / numSteps));
	}

	runtime->stop();
}

};  // namespace Graphics

};  // namespace SurgSim
