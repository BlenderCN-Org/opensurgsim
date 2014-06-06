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

#include <memory>

#include <gtest/gtest.h>

#include "SurgSim/Framework/ApplicationData.h"
#include "SurgSim/Framework/FrameworkConvert.h"
#include "SurgSim/Math/MeshShape.h"
#include "SurgSim/Math/OdeSolver.h"
#include "SurgSim/Math/Vector.h"
#include "SurgSim/Physics/DeformableCollisionRepresentation.h"
#include "SurgSim/Physics/DeformableRepresentation.h"
#include "SurgSim/Physics/Fem3DRepresentation.h"
#include "SurgSim/Physics/UnitTests/MockObjects.h"

namespace
{
const double epsilon = 1e-10;
}

namespace SurgSim
{
namespace Physics
{

struct DeformableCollisionRepresentationTest : public ::testing::Test
{
	void SetUp()
	{
		m_filename = std::string("Geometry/wound_deformable.ply");
		m_applicationData = std::make_shared<SurgSim::Framework::ApplicationData>("config.txt");
		m_meshShape = std::make_shared<SurgSim::Math::MeshShape>();
		m_meshShape->setFileName(m_filename);
		m_meshShape->initialize(*m_applicationData);
		m_deformableRepresentation = std::make_shared<MockDeformableRepresentation>("DeformableRepresentation");
		m_deformableCollisionRepresentation =
			std::make_shared<DeformableCollisionRepresentation>("DeformableCollisionRepresentation");
	}

	std::string m_filename;
	std::shared_ptr<SurgSim::Framework::ApplicationData> m_applicationData;
	std::shared_ptr<SurgSim::Math::MeshShape> m_meshShape;
	std::shared_ptr<SurgSim::Physics::DeformableRepresentation> m_deformableRepresentation;
	std::shared_ptr<SurgSim::Physics::DeformableCollisionRepresentation> m_deformableCollisionRepresentation;
};

TEST_F(DeformableCollisionRepresentationTest, InitTest)
{
	EXPECT_NO_THROW(DeformableCollisionRepresentation("TestDeformableCollisionRepresentation"));
	EXPECT_NO_THROW(std::make_shared<DeformableCollisionRepresentation>("DeformableCollisionRepresentation"));
}

TEST_F(DeformableCollisionRepresentationTest, SetGetDeformableRepresentationTest)
{
	ASSERT_NO_THROW(m_deformableCollisionRepresentation->setDeformableRepresentation(m_deformableRepresentation));
	EXPECT_EQ(m_deformableRepresentation, m_deformableCollisionRepresentation->getDeformableRepresentation());
}

TEST_F(DeformableCollisionRepresentationTest, ShapeTest)
{
	EXPECT_ANY_THROW(m_deformableCollisionRepresentation->getShapeType());
	m_deformableCollisionRepresentation->setShape(m_meshShape);
	EXPECT_EQ(SurgSim::Math::SHAPE_TYPE_MESH, m_deformableCollisionRepresentation->getShapeType());

	auto meshShape =
		std::dynamic_pointer_cast<SurgSim::Math::MeshShape>(m_deformableCollisionRepresentation->getShape());
	EXPECT_NEAR(m_meshShape->getVolume(), meshShape->getVolume(), epsilon);
	EXPECT_TRUE(m_meshShape->getCenter().isApprox(meshShape->getCenter()));
	EXPECT_TRUE(m_meshShape->getSecondMomentOfVolume().isApprox(meshShape->getSecondMomentOfVolume()));
}

TEST_F(DeformableCollisionRepresentationTest, MeshTest)
{
	m_deformableCollisionRepresentation->setMesh(m_meshShape->getMesh());
	EXPECT_EQ(m_meshShape->getMesh()->getNumVertices(),
		m_deformableCollisionRepresentation->getMesh()->getNumVertices());
	EXPECT_EQ(m_meshShape->getMesh()->getNumEdges(),
		m_deformableCollisionRepresentation->getMesh()->getNumEdges());
	EXPECT_EQ(m_meshShape->getMesh()->getNumTriangles(),
		m_deformableCollisionRepresentation->getMesh()->getNumTriangles());
}

TEST_F(DeformableCollisionRepresentationTest, SerializationTest)
{
	auto shape = std::dynamic_pointer_cast<SurgSim::Math::Shape>(m_meshShape);
	m_deformableCollisionRepresentation->setValue("Shape", shape);

	YAML::Node node;
	ASSERT_NO_THROW(node = YAML::convert<SurgSim::Framework::Component>::encode(*m_deformableCollisionRepresentation));

	std::shared_ptr<SurgSim::Physics::DeformableCollisionRepresentation> newDeformableCollisionRepresentation;
	ASSERT_NO_THROW(newDeformableCollisionRepresentation =
		std::dynamic_pointer_cast<SurgSim::Physics::DeformableCollisionRepresentation>
			(node.as<std::shared_ptr<SurgSim::Framework::Component>>())
		);

	auto mesh = std::dynamic_pointer_cast<SurgSim::Math::MeshShape>(newDeformableCollisionRepresentation->getShape());
	mesh->initialize(*m_applicationData);
	EXPECT_NEAR(m_meshShape->getVolume(), mesh->getVolume(), epsilon);
	EXPECT_TRUE(m_meshShape->getCenter().isApprox(mesh->getCenter()));
	EXPECT_TRUE(m_meshShape->getSecondMomentOfVolume().isApprox(mesh->getSecondMomentOfVolume()));

	EXPECT_EQ(m_meshShape->getMesh()->getNumVertices(), mesh->getMesh()->getNumVertices());
	EXPECT_EQ(m_meshShape->getMesh()->getNumEdges(), mesh->getMesh()->getNumEdges());
	EXPECT_EQ(m_meshShape->getMesh()->getNumTriangles(), mesh->getMesh()->getNumTriangles());
}

TEST_F(DeformableCollisionRepresentationTest, UpdateTest)
{
	EXPECT_ANY_THROW(m_deformableCollisionRepresentation->update(0.0));

	auto fem3DRepresentation = std::make_shared<SurgSim::Physics::Fem3DRepresentation>("Fem3DRepresentation");
	fem3DRepresentation->setFilename("Data/Geometry/wound_deformable.ply");
	fem3DRepresentation->setIntegrationScheme(SurgSim::Math::INTEGRATIONSCHEME_EXPLICIT_EULER);

	// Note: Directly calling loadFile is a workaround. Member data 'odeState' will be created while loading.
	ASSERT_TRUE(fem3DRepresentation->loadFile());

	// Connect Physics representation with Collision representation.
	fem3DRepresentation->setCollisionRepresentation(m_deformableCollisionRepresentation);

	// Set the shape used by Collision representation.
	m_deformableCollisionRepresentation->setShape(m_meshShape);
	EXPECT_NO_THROW(m_deformableCollisionRepresentation->update(0.0));
}

TEST_F(DeformableCollisionRepresentationTest, DoInitializationTest)
{
	EXPECT_ANY_THROW(m_deformableCollisionRepresentation->doInitialize());

	auto fem3DRepresentation = std::make_shared<SurgSim::Physics::Fem3DRepresentation>("Fem3DRepresentation");
	fem3DRepresentation->setFilename("Data/Geometry/wound_deformable.ply");
	fem3DRepresentation->setIntegrationScheme(SurgSim::Math::INTEGRATIONSCHEME_EXPLICIT_EULER);

	// Note: Directly calling loadFile is a workaround. Member data 'odeState' will be created while loading.
	ASSERT_TRUE(fem3DRepresentation->loadFile());

	// Connect Physics representation with Collision representation.
	fem3DRepresentation->setCollisionRepresentation(m_deformableCollisionRepresentation);

	// Set the shape used by Collision representation.
	m_deformableCollisionRepresentation->setShape(m_meshShape);
	EXPECT_NO_THROW(m_deformableCollisionRepresentation->doInitialize());
}

} // namespace Physics
} // namespace SurgSim