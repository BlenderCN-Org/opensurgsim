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
/// Tests for the OsgRepresentation class.

#include "SurgSim/Graphics/UnitTests/MockOsgObjects.h"
#include "SurgSim/Graphics/OsgMaterial.h"

#include "SurgSim/Math/Quaternion.h"
#include "SurgSim/Math/Vector.h"

#include <gtest/gtest.h>

#include <random>

using SurgSim::Math::Quaterniond;
using SurgSim::Math::RigidTransform3d;
using SurgSim::Math::Vector3d;

namespace SurgSim
{
namespace Graphics
{

TEST(OsgRepresentationTests, InitTest)
{
	ASSERT_NO_THROW({std::shared_ptr<Representation> representation =
		std::make_shared<MockOsgRepresentation>("test name");});

	std::shared_ptr<Representation> representation = std::make_shared<MockOsgRepresentation>("test name");

	EXPECT_EQ("test name", representation->getName());
	EXPECT_TRUE(representation->isVisible());
}

TEST(OsgRepresentationTests, OsgNodeTest)
{
	std::shared_ptr<OsgRepresentation> representation = std::make_shared<MockOsgRepresentation>("test name");

	EXPECT_NE(nullptr, representation->getOsgNode());

	// Check that the OSG node is a group (MockOsgRepresentation passes a new group as the node into the
	// OsgRepresentation constructor)
	osg::ref_ptr<osg::Group> osgGroup = dynamic_cast<osg::Group*>(representation->getOsgNode().get());
	EXPECT_TRUE(osgGroup.valid()) << "Representation's OSG node should be a group!";
}

TEST(OsgRepresentationTests, VisibilityTest)
{
	std::shared_ptr<Representation> representation = std::make_shared<MockOsgRepresentation>("test name");

	representation->setVisible(true);
	EXPECT_TRUE(representation->isVisible());

	representation->setVisible(false);
	EXPECT_FALSE(representation->isVisible());
}

TEST(OsgRepresentationTests, PoseTest)
{
	std::shared_ptr<Representation> representation = std::make_shared<MockOsgRepresentation>("test name");

	{
		SCOPED_TRACE("Check Initial Pose");
		EXPECT_TRUE(representation->getInitialPose().isApprox(RigidTransform3d::Identity()));
		EXPECT_TRUE(representation->getPose().isApprox(RigidTransform3d::Identity()));
	}

	RigidTransform3d initialPose;
	{
		SCOPED_TRACE("Set Initial Pose");
		initialPose = SurgSim::Math::makeRigidTransform(
			Quaterniond(SurgSim::Math::Vector4d::Random()).normalized(), Vector3d::Random());
		representation->setInitialPose(initialPose);
		EXPECT_TRUE(representation->getInitialPose().isApprox(initialPose));
		EXPECT_TRUE(representation->getPose().isApprox(initialPose));
	}

	{
		SCOPED_TRACE("Set Current Pose");
		RigidTransform3d currentPose = SurgSim::Math::makeRigidTransform(
			Quaterniond(SurgSim::Math::Vector4d::Random()).normalized(), Vector3d::Random());
		representation->setPose(currentPose);
		EXPECT_TRUE(representation->getInitialPose().isApprox(initialPose));
		EXPECT_TRUE(representation->getPose().isApprox(currentPose));
	}

	{
		SCOPED_TRACE("Change Initial Pose");
		initialPose = SurgSim::Math::makeRigidTransform(
			Quaterniond(SurgSim::Math::Vector4d::Random()).normalized(), Vector3d::Random());
		representation->setInitialPose(initialPose);
		EXPECT_TRUE(representation->getInitialPose().isApprox(initialPose));
		EXPECT_TRUE(representation->getPose().isApprox(initialPose));
	}
}

TEST(OsgRepresentationTests, MaterialTest)
{
	std::shared_ptr<Representation> representation = std::make_shared<MockOsgRepresentation>("test name");

	{
		SCOPED_TRACE("Set material");
		std::shared_ptr<Material> material = std::make_shared<OsgMaterial>();
		EXPECT_TRUE(representation->setMaterial(material));
		EXPECT_EQ(material, representation->getMaterial());
	}

	{
		SCOPED_TRACE("Clear material");
		representation->clearMaterial();
		EXPECT_EQ(nullptr, representation->getMaterial());
	}
}

TEST(OsgRepresentationTests, UpdateTest)
{
	std::shared_ptr<MockOsgRepresentation> mockRepresentation = std::make_shared<MockOsgRepresentation>("test name");
	std::shared_ptr<Representation> representation = mockRepresentation;

	EXPECT_EQ(0, mockRepresentation->getNumUpdates());
	EXPECT_EQ(0.0, mockRepresentation->getSumDt());

	double sumDt = 0.0;
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(0.0, 1.0);

	/// Do 10 updates with random dt and check each time that the number of updates and sum of dt are correct.
	for (int i = 1; i <= 10; ++i)
	{
		double dt = distribution(generator);
		sumDt += dt;

		representation->update(dt);
		EXPECT_EQ(i, mockRepresentation->getNumUpdates());
		EXPECT_LT(fabs(sumDt - mockRepresentation->getSumDt()), Eigen::NumTraits<double>::dummy_precision());
	}
}

TEST(OsgRepresentationTests, GroupTest)
{
	std::shared_ptr<Representation> rep = std::make_shared<MockOsgRepresentation>("TestRepresentation");

	EXPECT_TRUE(rep->addGroupReference("group1"));
	EXPECT_FALSE(rep->addGroupReference("group1"));

	EXPECT_TRUE(rep->addGroupReference("group2"));
	EXPECT_TRUE(rep->addGroupReference("group3"));

	std::vector<std::string> groups = rep->getGroupReferences();

	EXPECT_EQ(3U, groups.size());

	EXPECT_NE(std::end(groups), std::find(std::begin(groups), std::end(groups), "group1"));
	EXPECT_NE(std::end(groups), std::find(std::begin(groups), std::end(groups), "group2"));
	EXPECT_NE(std::end(groups), std::find(std::begin(groups), std::end(groups), "group3"));

}

TEST(OsgRepresentationTests, GroupsTest)
{
	std::shared_ptr<Representation> rep = std::make_shared<MockOsgRepresentation>("TestRepresentation");

	std::vector<std::string> newGroups;
	newGroups.push_back("group1");
	newGroups.push_back("group1");
	newGroups.push_back("group2");
	newGroups.push_back("group3");

	rep->addGroupReferences(newGroups);
	std::vector<std::string> groups = rep->getGroupReferences();

	EXPECT_EQ(3U, groups.size());

	EXPECT_NE(std::end(groups), std::find(std::begin(groups), std::end(groups), "group1"));
	EXPECT_NE(std::end(groups), std::find(std::begin(groups), std::end(groups), "group2"));
	EXPECT_NE(std::end(groups), std::find(std::begin(groups), std::end(groups), "group3"));


}

}  // namespace Graphics
}  // namespace SurgSim