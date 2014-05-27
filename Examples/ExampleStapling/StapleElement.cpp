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

#include "Examples/ExampleStapling/StapleElement.h"

#include "SurgSim/Framework/ApplicationData.h"
#include "SurgSim/Framework/Runtime.h"
#include "SurgSim/Graphics/OsgSceneryRepresentation.h"
#include "SurgSim/Math/MeshShape.h"
#include "SurgSim/Physics/RigidCollisionRepresentation.h"
#include "SurgSim/Physics/RigidRepresentation.h"
#include "SurgSim/Physics/RigidRepresentationParameters.h"

using SurgSim::Graphics::SceneryRepresentation;
using SurgSim::Graphics::OsgSceneryRepresentation;
using SurgSim::Math::MeshShape;
using SurgSim::Physics::RigidCollisionRepresentation;
using SurgSim::Physics::RigidRepresentation;
using SurgSim::Physics::RigidRepresentationParameters;

StapleElement::StapleElement(const std::string& name) :
	SurgSim::Framework::BasicSceneElement(name),
	m_hasCollisionRepresentation(true)
{
}

void StapleElement::setHasCollisionRepresentation(bool flag)
{
	m_hasCollisionRepresentation = flag;
}

bool StapleElement::doInitialize()
{
	auto meshShape = std::make_shared<MeshShape>();
	meshShape->setFileName(getRuntime()->getApplicationData()->findFile("/Geometry/staple_collision.ply"));

	RigidRepresentationParameters params;
	params.setDensity(8050); // Stainless steel (in Kg.m-3)
	params.setShapeUsedForMassInertia(meshShape);
	params.setLinearDamping(1e-2);
	params.setAngularDamping(1e-4);

	auto physicsRepresentation = std::make_shared<RigidRepresentation>("Physics");
	physicsRepresentation->setInitialParameters(params);

	std::shared_ptr<SceneryRepresentation> graphicsRepresentation =
		std::make_shared<OsgSceneryRepresentation>("Graphics");
	graphicsRepresentation->setFileName("Geometry/staple.obj");

	addComponent(physicsRepresentation);
	addComponent(graphicsRepresentation);

	if (m_hasCollisionRepresentation)
	{
		auto collisionRepresentation = std::make_shared<RigidCollisionRepresentation>("Collision");
		physicsRepresentation->setCollisionRepresentation(collisionRepresentation);

		addComponent(collisionRepresentation);
	}

	return true;
}
