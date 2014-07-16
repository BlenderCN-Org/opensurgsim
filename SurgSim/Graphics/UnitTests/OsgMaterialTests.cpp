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
/// Tests for the OsgMaterial class.

#include "SurgSim/Framework/Runtime.h"
#include "SurgSim/Graphics/OsgMaterial.h"
#include "SurgSim/Graphics/OsgShader.h"
#include "SurgSim/Graphics/OsgUniform.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using SurgSim::Math::Vector2f;

namespace
{
std::shared_ptr<SurgSim::Framework::Runtime> runtime = std::make_shared<SurgSim::Framework::Runtime>();
}

namespace SurgSim
{
namespace Graphics
{

/// Uniform that does not subclass OsgUniformBase
class MockUniform : public UniformBase
{
public:
	/// Constructor
	MockUniform() : UniformBase()
	{
	}
};

class MockShader : public Shader
{
public:
	MOCK_CONST_METHOD0(hasGeometryShader, bool());
	MOCK_CONST_METHOD0(hasVertexShader, bool());
	MOCK_CONST_METHOD0(hasFragmentShader, bool());

	MOCK_METHOD1(setGeometryShaderSource, void(const std::string&));
	MOCK_METHOD1(setVertexShaderSource, void(const std::string&));
	MOCK_METHOD1(setFragmentShaderSource, void(const std::string&));

	MOCK_METHOD1(loadGeometryShaderSource, bool(const std::string&));
	MOCK_METHOD1(loadVertexShaderSource, bool(const std::string&));
	MOCK_METHOD1(loadFragmentShaderSource, bool(const std::string&));

	MOCK_CONST_METHOD1(getGeometryShaderSource, bool(std::string*));
	MOCK_CONST_METHOD1(getVertexShaderSource, bool(std::string*));
	MOCK_CONST_METHOD1(getFragmentShaderSource, bool(std::string*));

	MOCK_METHOD0(clearGeometryShader, void());
	MOCK_METHOD0(clearVertexShader, void());
	MOCK_METHOD0(clearFragmentShader, void());

	MOCK_CONST_METHOD0(isGlobalScope, bool());
	MOCK_METHOD1(setGlobalScope, void(bool)); //NOLINT
};


TEST(OsgMaterialTests, InitTest)
{
	OsgMaterial material;

	EXPECT_EQ(0u, material.getNumUniforms());
	EXPECT_EQ(nullptr, material.getShader());

	EXPECT_NE(nullptr, material.getOsgStateSet());
}

TEST(OsgMaterialTests, AddAndRemoveUniformsTest)
{
	std::shared_ptr<OsgMaterial> osgMaterial = std::make_shared<OsgMaterial>();
	std::shared_ptr<Material> material = osgMaterial;
	material->initialize(runtime);

	EXPECT_EQ(0u, material->getNumUniforms());

	std::shared_ptr<OsgUniform<float>> osgUniform1 = std::make_shared<OsgUniform<float>>("float uniform");
	std::shared_ptr<Uniform<float>> uniform1 = osgUniform1;
	std::shared_ptr<OsgUniform<Vector2f>> osgUniform2 = std::make_shared<OsgUniform<Vector2f>>("Vector2f uniform");
	std::shared_ptr<Uniform<Vector2f>> uniform2 = osgUniform2;

	const osg::StateSet::UniformList& uniforms = osgMaterial->getOsgStateSet()->getUniformList();

	// Add a uniform to the material
	EXPECT_TRUE(material->addUniform(uniform1));
	EXPECT_EQ(1u, material->getNumUniforms());
	EXPECT_EQ(uniform1, material->getUniform(0));

	EXPECT_EQ(1u, uniforms.size());
	EXPECT_EQ(osgUniform1->getOsgUniform(), uniforms.at("float uniform").first);

	/// Add another uniform to the material
	EXPECT_TRUE(material->addUniform(uniform2));
	EXPECT_EQ(2u, material->getNumUniforms());
	EXPECT_EQ(uniform2, material->getUniform(1));

	EXPECT_EQ(2u, uniforms.size());
	EXPECT_EQ(osgUniform1->getOsgUniform(), uniforms.at("float uniform").first);
	EXPECT_EQ(osgUniform2->getOsgUniform(), uniforms.at("Vector2f uniform").first);

	/// Remove the first uniform from the material
	EXPECT_TRUE(material->removeUniform(uniform1));
	EXPECT_EQ(1u, material->getNumUniforms());
	EXPECT_EQ(uniform2, material->getUniform(0));

	EXPECT_EQ(1u, uniforms.size());
	EXPECT_EQ(osgUniform2->getOsgUniform(), uniforms.at("Vector2f uniform").first);

	/// Try removing the same uniform again
	EXPECT_FALSE(material->removeUniform(uniform1));
	EXPECT_EQ(1u, material->getNumUniforms());

	/// Try adding a non-OSG Uniform
	std::shared_ptr<MockUniform> nonOsgUniform = std::make_shared<MockUniform>();
	EXPECT_FALSE(material->addUniform(nonOsgUniform)) <<
			"Should not be able to add a uniform that is not a subclass of OsgUniformBase!";
	EXPECT_EQ(1u, material->getNumUniforms());

	/// Try removing a non-OSG Uniform
	EXPECT_FALSE(material->removeUniform(nonOsgUniform)) <<
			"Should not be able to remove a uniform that is not a subclass of OsgUniformBase!";
	EXPECT_EQ(1u, material->getNumUniforms());
}

TEST(OsgMaterialTests, SetAndClearShaderTest)
{
	std::shared_ptr<OsgMaterial> osgMaterial = std::make_shared<OsgMaterial>();
	std::shared_ptr<Material> material = osgMaterial;

	EXPECT_EQ(nullptr, material->getShader());

	std::shared_ptr<OsgShader> osgShader = std::make_shared<OsgShader>();
	std::shared_ptr<Shader> shader = osgShader;

	const osg::StateSet::AttributeList& attributes = osgMaterial->getOsgStateSet()->getAttributeList();

	// Set the material's shader
	EXPECT_TRUE(material->setShader(shader));
	EXPECT_EQ(shader, material->getShader());

	EXPECT_EQ(1u, attributes.size());
	EXPECT_EQ(osgShader->getOsgProgram(), attributes.at(osg::StateAttribute::TypeMemberPair(
				  osg::StateAttribute::PROGRAM, 0)).first) <<
						  "Shader should have been added to the material's state attributes!";

	/// Try setting a non-OSG Shader
	std::shared_ptr<MockShader> nonOsgShader = std::make_shared<MockShader>();
	EXPECT_FALSE(material->setShader(nonOsgShader)) <<
			"Should not be able to set a shader that is not a subclass of OsgShader!";
	EXPECT_NE(nonOsgShader, material->getShader());

	/// Clear the shader
	material->clearShader();
	EXPECT_EQ(nullptr, material->getShader());
	EXPECT_EQ(0u, attributes.size()) << "Shader should have been removed from the material's state attributes!";
}

TEST(OsgMaterialTests, NamedAccessTest)
{
	std::shared_ptr<OsgMaterial> osgMaterial = std::make_shared<OsgMaterial>();
	std::shared_ptr<Material> material = osgMaterial;

	std::string uniform1Name = "float uniform";
	std::shared_ptr<OsgUniform<float>> osgUniform1 = std::make_shared<OsgUniform<float>>(uniform1Name);
	std::shared_ptr<Uniform<float>> uniform1 = osgUniform1;

	std::string uniform2Name = "Vector2f uniform";
	std::shared_ptr<OsgUniform<Vector2f>> osgUniform2 = std::make_shared<OsgUniform<Vector2f>>(uniform2Name);
	std::shared_ptr<Uniform<Vector2f>> uniform2 = osgUniform2;

	material->addUniform(uniform1);
	material->addUniform(uniform2);

	EXPECT_TRUE(material->hasUniform(uniform1Name));
	EXPECT_EQ(uniform1.get(), material->getUniform(uniform1Name).get());

	EXPECT_TRUE(material->hasUniform(uniform2Name));
	EXPECT_EQ(uniform2.get(), material->getUniform(uniform2Name).get());

	EXPECT_FALSE(material->hasUniform("xxx"));
	EXPECT_EQ(nullptr, material->getUniform("xxx"));

	EXPECT_TRUE(material->removeUniform(uniform1Name));
	EXPECT_FALSE(material->hasUniform(uniform1Name));
}

TEST(OsgMaterialTests, AccessibleUniformTest)
{
	auto material = std::make_shared<OsgMaterial>();

	std::string uniform1Name = "ossFloatUniform";
	auto uniform1 = std::make_shared<OsgUniform<float>>(uniform1Name);


	std::string uniform2Name = "ossVector2fUniform";
	auto uniform2 = std::make_shared<OsgUniform<Vector2f>>(uniform2Name);

	material->addUniform(uniform1);
	material->addUniform(uniform2);

	material->setValue(uniform1Name, 2.0f);

	EXPECT_FLOAT_EQ(2.0, uniform1->get());

	uniform1->set(4.0f);
	EXPECT_FLOAT_EQ(4.0f, material->getValue<float>(uniform1Name));

	Vector2f vector1(1.0f, 2.0f);
	Vector2f vector2(3.0f, 4.0f);

	material->setValue(uniform2Name, vector1);

	EXPECT_TRUE(vector1.isApprox(uniform2->get()));

	uniform2->set(vector2);
	EXPECT_TRUE(vector2.isApprox(material->getValue<Vector2f>(uniform2Name)));

	material->removeUniform(uniform1);

	EXPECT_ANY_THROW(material->setValue(uniform1Name, 1.0f));

}


}  // namespace Graphics
}  // namespace SurgSim