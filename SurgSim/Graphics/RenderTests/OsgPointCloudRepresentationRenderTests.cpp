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

#include <memory>
#include <vector>

#include "SurgSim/DataStructures/Vertices.h"
#include "SurgSim/Framework/Runtime.h"
#include "SurgSim/Framework/Scene.h"
#include "SurgSim/Graphics/OsgBoxRepresentation.h"
#include "SurgSim/Graphics/OsgManager.h"
#include "SurgSim/Graphics/OsgPointCloudRepresentation.h"
#include "SurgSim/Graphics/OsgViewElement.h"
#include "SurgSim/Graphics/PointCloudRepresentation.h"
#include "SurgSim/Graphics/RenderTests/RenderTest.h"
#include "SurgSim/Math/Quaternion.h"
#include "SurgSim/Math/RigidTransform.h"
#include "SurgSim/Math/Vector.h"
#include "SurgSim/Testing/MathUtilities.h"

using SurgSim::Math::Vector3d;
using SurgSim::Math::Vector4d;
using SurgSim::Math::Quaterniond;
using SurgSim::Math::RigidTransform3d;
using SurgSim::Math::makeRigidTransform;
using SurgSim::Math::makeRotationQuaternion;

using SurgSim::Testing::interpolate;
using SurgSim::Testing::interpolatePose;

namespace SurgSim
{
namespace Graphics
{

typedef SurgSim::Graphics::PointCloudRepresentation::ValueType CloudMesh;

struct OsgPointCloudRepresentationRenderTests : public RenderTest
{


protected:
	std::vector<Vector3d> makeCube()
	{
		std::vector<Vector3d> result;
		result.push_back(Vector3d(0.01, -0.01, 0.01));
		result.push_back(Vector3d(0.01, -0.01, 0.01));
		result.push_back(Vector3d(-0.01, -0.01, 0.01));
		result.push_back(Vector3d(-0.01, -0.01, -0.01));
		result.push_back(Vector3d(0.01, -0.01, -0.01));

		result.push_back(Vector3d(0.01, 0.01, 0.01));
		result.push_back(Vector3d(-0.01, 0.01, 0.01));
		result.push_back(Vector3d(-0.01, 0.01, -0.01));
		result.push_back(Vector3d(0.01, 0.01, -0.01));
		return result;
	}

	std::shared_ptr<PointCloudRepresentation> makeCloud(std::vector<Vector3d> vertices)
	{
		std::shared_ptr<PointCloudRepresentation> cloud =
					std::make_shared<OsgPointCloudRepresentation>("cloud representation");

		cloud->setLocalPose(makeRigidTransform(Quaterniond::Identity(), Vector3d(0.0, 0.0, -0.2)));
		for (auto it = std::begin(vertices); it != std::end(vertices); ++it)
		{
			cloud->getVertices()->addVertex(SurgSim::Graphics::PointCloudRepresentation::ValueType::VertexType(*it));
		}

		viewElement->addComponent(cloud);

		return cloud;
	}
};

TEST_F(OsgPointCloudRepresentationRenderTests, PointAdd)
{
	std::vector<Vector3d> vertices = makeCube();

	auto representation = std::make_shared<OsgPointCloudRepresentation>("pointcloud representation");
	auto pointCloud = representation->getVertices();
	representation->setPointSize(2.0);

	RigidTransform3d pose = makeRigidTransform(makeRotationQuaternion(0.2, Vector3d(1.0, 1.0, 1.0)),
							Vector3d(0.0, 0.0, -0.2));
	representation->setLocalPose(pose);

	viewElement->addComponent(representation);

	/// Run the thread
	runtime->start();
	EXPECT_TRUE(graphicsManager->isInitialized());
	EXPECT_TRUE(viewElement->isInitialized());
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));


	for (size_t i = 0; i < vertices.size(); ++i)
	{
		pointCloud->addVertex(CloudMesh::VertexType(vertices[i]));
		boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	}
}

TEST_F(OsgPointCloudRepresentationRenderTests, StaticRotate)
{
	std::shared_ptr<PointCloudRepresentation> cloud = makeCloud(makeCube());

	/// Run the thread
	runtime->start();
	EXPECT_TRUE(graphicsManager->isInitialized());
	EXPECT_TRUE(viewElement->isInitialized());
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));

	int numSteps = 100;

	Vector3d startAngles(0.0, 0.0, 0.0);
	Vector3d endAngles(M_PI_4, M_PI_2, M_PI_2);
	Vector3d startPosition(-0.1, 0.0, -0.0);
	Vector3d endPosition(0.1, 0.0, -0.4);

	for (int i = 0; i < numSteps; ++i)
	{
		/// Calculate t in [0.0, 1.0]
		double t = static_cast<double>(i) / numSteps;
		cloud->setLocalPose(interpolatePose(startAngles, endAngles, startPosition, endPosition, t));
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / numSteps));
	}
}

TEST_F(OsgPointCloudRepresentationRenderTests, DynamicRotate)
{
	std::vector<Vector3d> startVertices = makeCube();
	std::shared_ptr<PointCloudRepresentation> cloud = makeCloud(startVertices);
	std::shared_ptr<CloudMesh> mesh = cloud->getVertices();

	/// Run the thread
	runtime->start();
	EXPECT_TRUE(graphicsManager->isInitialized());
	EXPECT_TRUE(viewElement->isInitialized());
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));

	int numSteps = 100;

	RigidTransform3d start = makeRigidTransform(makeRotationQuaternion(-M_PI_2, Vector3d(1.0, 1.0, 1.0)),
							 Vector3d(-0.1, 0.0, 0.2));

	RigidTransform3d end = makeRigidTransform(makeRotationQuaternion(M_PI_2, Vector3d(1.0, 1.0, 1.0)),
						   Vector3d(0.1, 0.0, -0.2));

	for (int i = 0; i < numSteps; ++i)
	{
		/// Calculate t in [0.0, 1.0]
		double t = static_cast<double>(i) / numSteps;
		RigidTransform3d currentPose = interpolate(start, end, t);

		int id = 0;
		for (auto it = std::begin(startVertices); it != std::end(startVertices); ++it, ++id)
		{
			mesh->setVertexPosition(id, currentPose * (*it));
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / numSteps));
	}
}

TEST_F(OsgPointCloudRepresentationRenderTests, PointSizeAndColor)
{
	std::shared_ptr<PointCloudRepresentation> cloud = makeCloud(makeCube());
	/// Run the thread
	runtime->start();
	EXPECT_TRUE(graphicsManager->isInitialized());
	EXPECT_TRUE(viewElement->isInitialized());
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));

	int numSteps = 100;
	std::pair<double, double> size = std::make_pair(0.0, 20.0);
	std::pair<Vector4d, Vector4d> color =
		std::make_pair(Vector4d(0.0, 1.0, 0.0, 1.0), Vector4d(1.0, 0.0, 1.0, 1.0));

	for (int i = 0; i < numSteps; ++i)
	{
		/// Calculate t in [0.0, 1.0]
		double t = static_cast<double>(i) / numSteps;
		cloud->setPointSize(interpolate(size, t));
		cloud->setColor(interpolate(color, t));
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000 / numSteps));
	}
}
}; // namespace Graphics
}; // namespace SurgSim
