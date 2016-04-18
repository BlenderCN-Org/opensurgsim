// This file is a part of the OpenSurgSim project.
// Copyright 2015, SimQuest Solutions Inc.
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

#include "SurgSim/Collision/ShapeCollisionRepresentation.h"
#include "SurgSim/Framework/Component.h"
#include "SurgSim/Framework/Runtime.h"
#include "SurgSim/Graphics/OsgMeshRepresentation.h"
#include "SurgSim/Graphics/OsgLight.h"
#include "SurgSim/Graphics/OsgMaterial.h"
#include "SurgSim/Graphics/OsgTexture2d.h"
#include "SurgSim/Graphics/RenderTests/RenderTest.h"
#include "SurgSim/Graphics/PaintBehavior.h"
#include "SurgSim/Math/MeshShape.h"
#include "SurgSim/Math/RigidTransform.h"
#include "SurgSim/Math/Vector.h"

using SurgSim::Math::makeRigidTransform;
using SurgSim::Math::makeRigidTranslation;
using SurgSim::Math::RigidTransform3d;
using SurgSim::Math::Vector3d;

namespace SurgSim
{
namespace Graphics
{

class PaintBehaviorRenderTests : public RenderTest
{

};

TEST_F(PaintBehaviorRenderTests, InitTest)
{
	viewElement->enableManipulator(true);

	auto light = std::make_shared<Graphics::OsgLight>("Light");
	light->setDiffuseColor(Math::Vector4d(1.0, 1.0, 1.0, 1.0));
	light->setSpecularColor(Math::Vector4d(0.8, 0.8, 0.8, 1.0));
	light->setLightGroupReference(Graphics::Representation::DefaultGroupName);

	auto lightElement = std::make_shared<Framework::BasicSceneElement>("LightElement");
	lightElement->setPose(makeRigidTranslation(Math::Vector3d(2.0, 2.0, 2.0)));
	lightElement->addComponent(light);
	scene->addSceneElement(lightElement);
	
	auto element = std::make_shared<Framework::BasicSceneElement>("Cube");
	auto graphics = std::make_shared<Graphics::OsgMeshRepresentation>("Graphics");
	graphics->loadMesh("Geometry/wound_deformable_with_texture.ply");

	auto material = Graphics::buildMaterial("Shaders/material.vert", "Shaders/material_multitexture.frag");
	material->addUniform("vec4", "diffuseColor");
	material->setValue("diffuseColor", Math::Vector4f(1.0, 0.0, 0.0, 1.0));
	material->addUniform("vec4", "specularColor");
	material->setValue("specularColor", Math::Vector4f(1.0, 1.0, 1.0, 1.0));
	material->addUniform("float", "shininess");
	material->setValue("shininess", 10.0f);

	auto texture = std::make_shared<Graphics::OsgTexture2d>();
	auto path = applicationData->findFile("Textures/CheckerBoard.png");
	texture->loadImage(path);
	// Need to create RGBA Image object and set that as texture (or load empty texture)
	material->addUniform("sampler2D", "decalTexture");
	material->setValue("decalTexture", texture);

	graphics->setMaterial(material);
	
	element->addComponent(graphics);
	element->addComponent(material);

	auto paintBehavior = std::make_shared<Graphics::PaintBehavior>("Decals");
	paintBehavior->setRepresentation(graphics);
	paintBehavior->setTexture(texture);
	paintBehavior->setPaintColor(Math::Vector4d(1.0, 0.0, 0.0, 1.0));

	std::vector<DataStructures::IndexedLocalCoordinate> coords;
	for (int i = 0; i < 25; i++)
	{
		Math::Vector coord(3);
		coord << 0.5, 0.5, 0.5;
		DataStructures::IndexedLocalCoordinate localCoordinate(50 + i, coord);
		coords.push_back(localCoordinate);
	}
	paintBehavior->setPaintCoordinate(coords);

	element->addComponent(paintBehavior);

	scene->addSceneElement(element);

	runtime->start();
	boost::this_thread::sleep(boost::posix_time::milliseconds(50000));
	runtime->stop();
}

}; // namespace Graphics
}; // namespace SurgSim
