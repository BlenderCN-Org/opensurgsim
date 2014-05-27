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

#include "SurgSim/Collision/CollisionPair.h"
#include "SurgSim/Collision/ShapeCollisionRepresentation.h"
#include "SurgSim/DataStructures/BufferedValue.h"
#include "SurgSim/Framework/BasicSceneElement.h"
#include "SurgSim/Math/PlaneShape.h"
#include "SurgSim/Math/Quaternion.h"
#include "SurgSim/Math/RigidTransform.h"
#include "SurgSim/Math/SphereShape.h"
#include "SurgSim/Math/Vector.h"

using SurgSim::Collision::Contact;
using SurgSim::Collision::ContactMapType;
using SurgSim::Collision::Location;
using SurgSim::Collision::ShapeCollisionRepresentation;
using SurgSim::DataStructures::ReadAccessor;
using SurgSim::Framework::BasicSceneElement;
using SurgSim::Math::makeRigidTransform;
using SurgSim::Math::PlaneShape;
using SurgSim::Math::Quaterniond;
using SurgSim::Math::RigidTransform3d;
using SurgSim::Math::SphereShape;
using SurgSim::Math::Vector3d;

namespace
{
	const double epsilon = 1e-10;
};

namespace SurgSim
{
namespace Collision
{

struct RepresentationTest : public ::testing::Test
{
	virtual void SetUp()
	{
		element = std::make_shared<BasicSceneElement>("Element");
		plane = std::make_shared<PlaneShape>();
		sphere = std::make_shared<SphereShape>(1.0);
		planeRep = std::make_shared<ShapeCollisionRepresentation>("PlaneShape");
		sphereRep = std::make_shared<ShapeCollisionRepresentation>("SphereShape");

		planeRep->setShape(plane);
		planeRep->setLocalPose(RigidTransform3d::Identity());

		sphereRep->setShape(sphere);
		sphereRep->setLocalPose(RigidTransform3d::Identity());

		element->addComponent(planeRep);
		element->addComponent(sphereRep);
		element->initialize();
		planeRep->wakeUp();
		sphereRep->wakeUp();
	}

	virtual void TearDown()
	{
	}

	std::shared_ptr<BasicSceneElement> element;
	std::shared_ptr<PlaneShape> plane;
	std::shared_ptr<SphereShape> sphere;
	std::shared_ptr<ShapeCollisionRepresentation> planeRep;
	std::shared_ptr<ShapeCollisionRepresentation> sphereRep;
};

TEST_F(RepresentationTest, InitTest)
{
	EXPECT_NO_THROW(ShapeCollisionRepresentation("Plane"));
}

TEST_F(RepresentationTest, PoseTest)
{
	RigidTransform3d initialPose = makeRigidTransform(Quaterniond::Identity(), Vector3d(1.0, 2.0, 3.0));
	planeRep->setLocalPose(initialPose);
	EXPECT_TRUE(initialPose.isApprox(planeRep->getPose(), epsilon));

	RigidTransform3d pose = makeRigidTransform(Quaterniond::Identity(), Vector3d(0.0, 2.0, 0.0));
	element->setPose(pose);
	EXPECT_TRUE(pose.isApprox(sphereRep->getPose(), epsilon));

	sphereRep->setLocalPose(initialPose);
	EXPECT_TRUE((pose * initialPose).isApprox(sphereRep->getPose(), epsilon));
}

TEST_F(RepresentationTest, ShapeTest)
{
	EXPECT_EQ(SurgSim::Math::SHAPE_TYPE_PLANE, planeRep->getShapeType());
	EXPECT_EQ(SurgSim::Math::SHAPE_TYPE_SPHERE, sphereRep->getShapeType());

	EXPECT_EQ(plane, planeRep->getShape());
	EXPECT_EQ(sphere, sphereRep->getShape());
}

TEST_F(RepresentationTest, EmptyCollisionTest)
{
	ReadAccessor<ContactMapType> planeCollisions(planeRep->getCollisions());
	ReadAccessor<ContactMapType> sphereCollisions(sphereRep->getCollisions());

	EXPECT_TRUE(planeCollisions->empty());
	EXPECT_TRUE(sphereCollisions->empty());
}

TEST_F(RepresentationTest, CollisionTest)
{
	ReadAccessor<ContactMapType> planeCollisions(planeRep->getCollisions());
	ReadAccessor<ContactMapType> sphereCollisions(sphereRep->getCollisions());

	EXPECT_TRUE(planeCollisions->empty());
	EXPECT_TRUE(sphereCollisions->empty());

	std::shared_ptr<Contact> dummyContact =
		std::make_shared<Contact>(0.0, Vector3d::Zero(), Vector3d::Zero(), std::make_pair(Location(), Location()));
	EXPECT_NO_THROW(sphereRep->addCollisionWith(planeRep, dummyContact));

	sphereRep->update(0.0);
	planeRep->update(0.0);

	EXPECT_EQ(1u, sphereCollisions->size());
	auto spherePlanePair = sphereCollisions->find(planeRep);
	EXPECT_NE(sphereCollisions->end(), spherePlanePair);
	std::list<std::shared_ptr<SurgSim::Collision::Contact>> spherePlaneContacts = spherePlanePair->second;
	EXPECT_EQ(dummyContact, spherePlaneContacts.front());

	// Collision is only added to 'sphereRep', thus the plane should have no collisions.
	EXPECT_TRUE(planeCollisions->empty());

	EXPECT_NO_THROW(planeRep->addCollisionWith(sphereRep, dummyContact));
	sphereRep->update(0.0);
	planeRep->update(0.0);
	EXPECT_EQ(1u, planeCollisions->size());
	auto planeSpherePair = planeCollisions->find(sphereRep);
	EXPECT_NE(planeCollisions->end(), planeSpherePair);
	std::list<std::shared_ptr<SurgSim::Collision::Contact>> planeSphereContacts = planeSpherePair->second;
	EXPECT_EQ(dummyContact, planeSphereContacts.front());

	sphereRep->clearCollisions();
	sphereRep->update(0.0);
	EXPECT_TRUE(sphereCollisions->empty());
}


}; // namespace Collision
}; // namespace SurgSim
