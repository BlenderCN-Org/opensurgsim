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
/// Tests for the OsgCamera class.

#include "SurgSim/Graphics/UnitTests/MockObjects.h"
#include "SurgSim/Graphics/UnitTests/MockOsgObjects.h"

#include "SurgSim/Framework/BasicSceneElement.h"
#include "SurgSim/Framework/FrameworkConvert.h"
#include "SurgSim/Graphics/OsgCamera.h"
#include "SurgSim/Graphics/OsgGroup.h"
#include "SurgSim/Graphics/OsgMatrixConversions.h"
#include "SurgSim/Graphics/OsgTexture2d.h"
#include "SurgSim/Graphics/OsgRenderTarget.h"
#include "SurgSim/Math/Quaternion.h"

#include <osg/Camera>
#include <osg/ref_ptr>

#include <gtest/gtest.h>

#include <random>

using SurgSim::Framework::BasicSceneElement;
using SurgSim::Math::Matrix44d;
using SurgSim::Math::Quaterniond;
using SurgSim::Math::RigidTransform3d;
using SurgSim::Math::Vector3d;
using SurgSim::Math::makeRigidTransform;

namespace SurgSim
{
namespace Graphics
{

TEST(OsgCameraTests, InitTest)
{
	ASSERT_NO_THROW({std::shared_ptr<Camera> camera = std::make_shared<OsgCamera>("test name");});

	std::shared_ptr<OsgCamera> osgCamera = std::make_shared<OsgCamera>("test name");
	std::shared_ptr<Camera> camera = osgCamera;

	EXPECT_EQ("test name", camera->getName());

	EXPECT_TRUE(camera->isVisible());

	EXPECT_TRUE(camera->getPose().matrix().isApprox(
					fromOsg(osgCamera->getOsgCamera()->getViewMatrix()).inverse())) <<
							"Camera's pose should be initialized to the inverse of the osg::Camera's view matrix!";

	EXPECT_TRUE(camera->getViewMatrix().isApprox(fromOsg(osgCamera->getOsgCamera()->getViewMatrix()))) <<
			"Camera's view matrix should be initialized to the osg::Camera's view matrix!";

	EXPECT_TRUE(camera->getProjectionMatrix().isApprox(fromOsg(osgCamera->getOsgCamera()->getProjectionMatrix()))) <<
			"Camera's projection matrix should be initialized to the osg::Camera's projection matrix!";

	EXPECT_EQ(nullptr, camera->getRenderGroup());
}

TEST(OsgCameraTests, OsgNodesTest)
{
	std::shared_ptr<OsgCamera> osgCamera = std::make_shared<OsgCamera>("test name");
	std::shared_ptr<OsgRepresentation> osgRepresentation = osgCamera;

	/// Check that the OSG nodes of the camera are built correctly
	osg::ref_ptr<osg::Node> node = osgRepresentation->getOsgNode();
	osg::ref_ptr<osg::Switch> switchNode = dynamic_cast<osg::Switch*>(node.get());
	EXPECT_TRUE(switchNode.valid());
	EXPECT_EQ(1u, switchNode->getNumChildren());

	osg::ref_ptr<osg::Camera> camera = osgCamera->getOsgCamera();
	EXPECT_EQ(camera.get(), switchNode->getChild(0));
}

TEST(OsgCameraTests, VisibilityTest)
{
	std::shared_ptr<OsgCamera> osgCamera = std::make_shared<OsgCamera>("test name");
	std::shared_ptr<OsgRepresentation> osgRepresentation = osgCamera;
	std::shared_ptr<Camera> camera = osgCamera;

	// Get the osg::Switch from the OsgRepresentation so that we can make sure that the osg::Camera has the
	// correct visibility.
	osg::ref_ptr<osg::Switch> switchNode = dynamic_cast<osg::Switch*>(osgRepresentation->getOsgNode().get());
	EXPECT_TRUE(switchNode.valid());

	EXPECT_TRUE(camera->isVisible());
	EXPECT_TRUE(switchNode->getChildValue(osgCamera->getOsgCamera()));

	camera->setVisible(false);
	EXPECT_FALSE(camera->isVisible());
	EXPECT_FALSE(switchNode->getChildValue(osgCamera->getOsgCamera()));

	camera->setVisible(true);
	EXPECT_TRUE(camera->isVisible());
	EXPECT_TRUE(switchNode->getChildValue(osgCamera->getOsgCamera()));

}

TEST(OsgCameraTests, GroupTest)
{
	std::shared_ptr<OsgCamera> osgCamera = std::make_shared<OsgCamera>("test name");
	std::shared_ptr<Camera> camera = osgCamera;

	EXPECT_EQ(nullptr, camera->getRenderGroup());

	/// Adding an OsgGroup should succeed
	std::shared_ptr<OsgGroup> osgGroup = std::make_shared<OsgGroup>(camera->getRenderGroupReference());
	std::shared_ptr<Group> group = osgGroup;
	EXPECT_TRUE(camera->setRenderGroup(group));
	EXPECT_EQ(group, camera->getRenderGroup());

	/// Check that the OSG node of the group is added to the OSG camera correctly
	EXPECT_EQ(osgGroup->getOsgGroup(), osgCamera->getOsgCamera()->getChild(0)->asGroup()->getChild(0));

	/// Adding a group that does not derive from OsgGroup should fail
	std::shared_ptr<Group> mockGroup = std::make_shared<MockGroup>(camera->getRenderGroupReference());
	EXPECT_FALSE(camera->setRenderGroup(mockGroup));
	EXPECT_EQ(group, camera->getRenderGroup());
	EXPECT_EQ(osgGroup->getOsgGroup(), osgCamera->getOsgCamera()->getChild(0)->asGroup()->getChild(0));
}


TEST(OsgCameraTests, PoseTest)
{
	std::shared_ptr<OsgCamera> osgCamera = std::make_shared<OsgCamera>("test name");
	std::shared_ptr<Camera> camera = osgCamera;
	camera->setRenderGroupReference("Test");
	std::shared_ptr<BasicSceneElement> element = std::make_shared<BasicSceneElement>("element");
	element->addComponent(camera);
	element->initialize();
	camera->wakeUp();

	RigidTransform3d elementPose = SurgSim::Math::makeRigidTransform(
									   Quaterniond(SurgSim::Math::Vector4d::Random()).normalized(), Vector3d::Random());
	RigidTransform3d localPose = SurgSim::Math::makeRigidTransform(
									 Quaterniond(SurgSim::Math::Vector4d::Random()).normalized(), Vector3d::Random());
	RigidTransform3d pose = elementPose * localPose;

	{
		SCOPED_TRACE("Check Initial Pose");
		EXPECT_TRUE(camera->getLocalPose().isApprox(RigidTransform3d::Identity()));
		EXPECT_TRUE(camera->getPose().isApprox(RigidTransform3d::Identity()));
	}

	{
		SCOPED_TRACE("Set Local Pose");
		camera->setLocalPose(localPose);
		EXPECT_TRUE(camera->getLocalPose().isApprox(localPose));
		EXPECT_TRUE(camera->getPose().isApprox(localPose));
		EXPECT_TRUE(camera->getViewMatrix().isApprox(localPose.matrix().inverse()));
		EXPECT_TRUE(camera->getViewMatrix().inverse().isApprox(camera->getInverseViewMatrix()));

		camera->update(0.01);
		EXPECT_TRUE(fromOsg(osgCamera->getOsgCamera()->getViewMatrix()).isApprox(localPose.matrix().inverse()));
	}

	{
		SCOPED_TRACE("Set Element Pose");
		element->setPose(elementPose);
		EXPECT_TRUE(camera->getLocalPose().isApprox(localPose));
		EXPECT_TRUE(camera->getPose().isApprox(pose));
		EXPECT_TRUE(camera->getViewMatrix().isApprox(pose.matrix().inverse()));
		EXPECT_TRUE(camera->getViewMatrix().inverse().isApprox(camera->getInverseViewMatrix()));

		camera->update(0.01);
		EXPECT_TRUE(fromOsg(osgCamera->getOsgCamera()->getViewMatrix()).isApprox(pose.matrix().inverse()));
	}
}

TEST(OsgCameraTests, MatricesTest)
{
	std::shared_ptr<OsgCamera> osgCamera = std::make_shared<OsgCamera>("test name");
	std::shared_ptr<Camera> camera = osgCamera;

	Matrix44d projectionMatrix = Matrix44d::Random();
	camera->setProjectionMatrix(projectionMatrix);
	EXPECT_TRUE(camera->getProjectionMatrix().isApprox(projectionMatrix));
}

TEST(OsgCameraTests, RenderTargetTest)
{
	auto osgCamera = std::make_shared<OsgCamera>("test camera");
	std::shared_ptr<Camera> camera = osgCamera;

	std::shared_ptr<RenderTarget> renderTarget = std::make_shared<OsgRenderTarget2d>(256, 256, 1.0, 2, true);

	EXPECT_NO_THROW(camera->setRenderTarget(renderTarget));
	EXPECT_TRUE(osgCamera->getOsgCamera()->isRenderToTextureCamera());
}

TEST(OsgCameraTests, CameraGroupTest)
{
	std::shared_ptr<Camera> camera = std::make_shared<OsgCamera>("TestRepresentation");

	camera->clearGroupReferences();
	camera->addGroupReference("test1");
	camera->addGroupReference("test2");

	EXPECT_EQ(2u, camera->getGroupReferences().size());

	camera->setRenderGroupReference("otherTest");
	EXPECT_EQ(2u, camera->getGroupReferences().size());

	// Setting the render group of a camera to the group that it is in should remove the group
	// from the set
	camera->setRenderGroupReference("test1");
	EXPECT_EQ(1u, camera->getGroupReferences().size());

	// Should not be able to set group reference to the current render group
	EXPECT_FALSE(camera->addGroupReference("test1"));
	EXPECT_EQ(1u, camera->getGroupReferences().size());
}

TEST(OsgCameraTests, Serialization)
{
	std::shared_ptr<OsgCamera> camera = std::make_shared<OsgCamera>("TestOsgCamera");

	// Set values.
	SurgSim::Math::Matrix44d projection = SurgSim::Math::Matrix44d::Random();
	camera->setValue("ProjectionMatrix", projection);
	camera->setValue("Visible", true);
	camera->setValue("AmbientColor", SurgSim::Math::Vector4d(0.1, 0.2, 0.3, 0.4));

	// Serialize.
	YAML::Node node;
	EXPECT_NO_THROW(node = YAML::convert<SurgSim::Framework::Component>::encode(*camera););

	// Deserialize.
	std::shared_ptr<Camera> newCamera;
	newCamera = std::dynamic_pointer_cast<OsgCamera>(node.as<std::shared_ptr<SurgSim::Framework::Component>>());
	EXPECT_NE(nullptr, newCamera);

	// Verify.
	EXPECT_TRUE(boost::any_cast<SurgSim::Math::Matrix44d>(camera->getValue("ProjectionMatrix")).isApprox(
				boost::any_cast<SurgSim::Math::Matrix44d>(newCamera->getValue("ProjectionMatrix"))));
	EXPECT_EQ(boost::any_cast<bool>(camera->getValue("Visible")),
			  boost::any_cast<bool>(newCamera->getValue("Visible")));
	EXPECT_TRUE(boost::any_cast<SurgSim::Math::Vector4d>(camera->getValue("AmbientColor")).isApprox(
				boost::any_cast<SurgSim::Math::Vector4d>(newCamera->getValue("AmbientColor"))));
}

}  // namespace Graphics
}  // namespace SurgSim
