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

#include "Examples/ExampleStapling/StaplerBehavior.h"

#include <boost/exception/to_string.hpp>

#include "Examples/ExampleStapling/StapleElement.h"
#include "SurgSim/Collision/CollisionPair.h"
#include "SurgSim/Collision/Representation.h"
#include "SurgSim/DataStructures/DataStructuresConvert.h"
#include "SurgSim/DataStructures/DataGroup.h"
#include "SurgSim/Framework/FrameworkConvert.h"
#include "SurgSim/Framework/Log.h"
#include "SurgSim/Framework/Scene.h"
#include "SurgSim/Framework/SceneElement.h"
#include "SurgSim/Graphics/SceneryRepresentation.h"
#include "SurgSim/Input/InputComponent.h"
#include "SurgSim/Physics/Constraint.h"
#include "SurgSim/Physics/ConstraintComponent.h"
#include "SurgSim/Physics/DeformableCollisionRepresentation.h"
#include "SurgSim/Physics/DeformableRepresentation.h"
#include "SurgSim/Physics/FixedRepresentationBilateral3D.h"
#include "SurgSim/Physics/Fem3DRepresentationBilateral3D.h"
#include "SurgSim/Physics/Localization.h"
#include "SurgSim/Physics/RigidCollisionRepresentation.h"
#include "SurgSim/Physics/RigidRepresentation.h"
#include "SurgSim/Physics/RigidRepresentationBilateral3D.h"

using SurgSim::Physics::ConstraintImplementation;
using SurgSim::Physics::FixedRepresentationBilateral3D;
using SurgSim::Physics::RigidRepresentationBilateral3D;
using SurgSim::Physics::Fem3DRepresentationBilateral3D;
using SurgSim::Physics::Localization;

namespace
{
SURGSIM_REGISTER(SurgSim::Framework::Component, StaplerBehavior);
}

StaplerBehavior::StaplerBehavior(const std::string& name):
	SurgSim::Framework::Behavior(name),
	m_numElements(0),
	m_button1Index(-1),
	m_button1IndexCached(false),
	m_buttonPreviouslyPressed(false)
{
	typedef std::array<std::shared_ptr<SurgSim::Collision::Representation>, 2> VirtualTeethArray;
	SURGSIM_ADD_SERIALIZABLE_PROPERTY(StaplerBehavior, std::shared_ptr<SurgSim::Framework::Component>,
		InputComponent, getInputComponent, setInputComponent);
	SURGSIM_ADD_SERIALIZABLE_PROPERTY(StaplerBehavior, std::shared_ptr<SurgSim::Framework::Component>,
		Representation, getRepresentation, setRepresentation);
	SURGSIM_ADD_SERIALIZABLE_PROPERTY(StaplerBehavior, VirtualTeethArray, VirtualStaple,
		getVirtualStaple, setVirtualStaple);
	SURGSIM_ADD_SERIALIZABLE_PROPERTY(StaplerBehavior, std::list<std::string>, StapleEnabledSceneElements,
		getStapleEnabledSceneElements, setStapleEnabledSceneElements);
}

void StaplerBehavior::setInputComponent(std::shared_ptr<SurgSim::Framework::Component> inputComponent)
{
	SURGSIM_ASSERT(nullptr != inputComponent) << "Cannot set nullptr to InputComponent";

	m_from = std::dynamic_pointer_cast<SurgSim::Input::InputComponent>(inputComponent);

	SURGSIM_ASSERT(nullptr != m_from) << "Cannot set other than SurgSim::Input::InputComponent to InputComponent";
}

std::shared_ptr<SurgSim::Input::InputComponent> StaplerBehavior::getInputComponent()
{
	return m_from;
}

void StaplerBehavior::setRepresentation(std::shared_ptr<SurgSim::Framework::Component> staplerRepresentation)
{
	SURGSIM_ASSERT(nullptr != staplerRepresentation) << "Cannot set nullptr to Representation";

	m_representation = std::dynamic_pointer_cast<SurgSim::Framework::Representation>(staplerRepresentation);

	SURGSIM_ASSERT(nullptr != m_representation)
		<< "Cannot set other than SurgSim::Framework::Representation to StaplerRepresentation";
}

std::shared_ptr<SurgSim::Framework::Representation> StaplerBehavior::getRepresentation()
{
	return m_representation;
}

void StaplerBehavior::setVirtualStaple(
	const std::array<std::shared_ptr<SurgSim::Collision::Representation>, 2>& virtualTeeth)
{
	m_virtualTeeth = virtualTeeth;
}

const std::array<std::shared_ptr<SurgSim::Collision::Representation>, 2>& StaplerBehavior::getVirtualStaple()
{
	return m_virtualTeeth;
}

void StaplerBehavior::enableStaplingForSceneElement(std::string sceneElementName)
{
	m_stapleEnabledSceneElements.push_back(sceneElementName);
}

void StaplerBehavior::setStapleEnabledSceneElements(const std::list<std::string>& stapleEnabledSceneElements)
{
	m_stapleEnabledSceneElements = stapleEnabledSceneElements;
}

const std::list<std::string>& StaplerBehavior::getStapleEnabledSceneElements()
{
	return m_stapleEnabledSceneElements;
}

void StaplerBehavior::filterCollisionMapForStapleEnabledRepresentations(
	SurgSim::Collision::Representation::ContactMapType* collisionsMap)
{
	for (auto it = collisionsMap->begin(); it != collisionsMap->end();)
	{
		if (std::find(m_stapleEnabledSceneElements.begin(),
					  m_stapleEnabledSceneElements.end(),
					  (*it).first->getSceneElement()->getName()) == m_stapleEnabledSceneElements.end())
		{
			// Representation's scene element is not in the m_stapleEnabledSceneElements.
			it = collisionsMap->erase(it);
		}
		else
		{
			++it;
		}
	}
}

std::shared_ptr<SurgSim::Physics::Representation> StaplerBehavior::findCorrespondingPhysicsRepresentation(
	std::shared_ptr<SurgSim::Collision::Representation> collisionRepresentation)
{
	std::shared_ptr<SurgSim::Physics::Representation> physicsRepresentation = nullptr;

	// Check if the collisionRepresenation is for a Rigid body.
	std::shared_ptr<SurgSim::Physics::RigidCollisionRepresentation> rigidCollisionRepresentation =
		std::dynamic_pointer_cast<SurgSim::Physics::RigidCollisionRepresentation>(collisionRepresentation);

	if (rigidCollisionRepresentation != nullptr)
	{
		physicsRepresentation = rigidCollisionRepresentation->getRigidRepresentation();
	}
	else
	{
		// Check if the collisionRepresenation is for a deformable body.
		std::shared_ptr<SurgSim::Physics::DeformableCollisionRepresentation> deformableCollisionRepresentation =
			std::dynamic_pointer_cast<SurgSim::Physics::DeformableCollisionRepresentation>(collisionRepresentation);

		if (deformableCollisionRepresentation != nullptr)
		{
			physicsRepresentation = deformableCollisionRepresentation->getDeformableRepresentation();
		}
	}

	return physicsRepresentation;
}

void StaplerBehavior::filterCollisionMapForSupportedRepresentationTypes(
	SurgSim::Collision::Representation::ContactMapType* collisionsMap)
{
	for (auto it = collisionsMap->begin(); it != collisionsMap->end();)
	{
		if (findCorrespondingPhysicsRepresentation((*it).first) == nullptr)
		{
			// Representation type is not supported to be stapled.
			it = collisionsMap->erase(it);
		}
		else
		{
			++it;
		}
	}
}

std::shared_ptr<SurgSim::Physics::Constraint> StaplerBehavior::createBilateral3DConstraint(
	std::shared_ptr<SurgSim::Physics::Representation> stapleRep,
	std::shared_ptr<SurgSim::Physics::Representation> otherRep,
	SurgSim::Collision::Location contraintLocation)
{
	// Create a bilateral constraint between the physicsRepresentation and staple.
	// First find the points where the constraint is going to be applied.
	std::shared_ptr<Localization> stapleRepLocalization
		= stapleRep->createLocalization(contraintLocation);
	stapleRepLocalization->setRepresentation(stapleRep);

	std::shared_ptr<Localization> otherRepLocatization
		= otherRep->createLocalization(contraintLocation);
	otherRepLocatization->setRepresentation(otherRep);

	std::shared_ptr<SurgSim::Physics::Constraint> constraint = nullptr;

	// Create the Constraint with appropriate constraint implementation.
	switch (otherRep->getType())
	{
	case SurgSim::Physics::REPRESENTATION_TYPE_FIXED:
		constraint = std::make_shared<SurgSim::Physics::Constraint>(
						std::make_shared<SurgSim::Physics::ConstraintData>(),
						std::make_shared<RigidRepresentationBilateral3D>(),
						stapleRepLocalization,
						std::make_shared<FixedRepresentationBilateral3D>(),
						otherRepLocatization);
		break;

	case SurgSim::Physics::REPRESENTATION_TYPE_RIGID:
		constraint = std::make_shared<SurgSim::Physics::Constraint>(
						std::make_shared<SurgSim::Physics::ConstraintData>(),
						std::make_shared<RigidRepresentationBilateral3D>(),
						stapleRepLocalization,
						std::make_shared<RigidRepresentationBilateral3D>(),
						otherRepLocatization);
		break;

	case SurgSim::Physics::REPRESENTATION_TYPE_FEM3D:
		constraint = std::make_shared<SurgSim::Physics::Constraint>(
						std::make_shared<SurgSim::Physics::ConstraintData>(),
						std::make_shared<RigidRepresentationBilateral3D>(),
						stapleRepLocalization,
						std::make_shared<Fem3DRepresentationBilateral3D>(),
						otherRepLocatization);
		break;

	default:
		SURGSIM_FAILURE() << "Stapling constraint not supported for this representation type";
		break;
	}

	return constraint;
}

void StaplerBehavior::createStaple()
{
	// Create the staple (not added to the scene right now).
	auto staple = std::make_shared<StapleElement>("staple_" + boost::to_string(m_numElements++));
	staple->setPose(m_representation->getPose());

	int toothId = 0;
	bool stapleAdded = false;
	for (auto virtualTooth = m_virtualTeeth.begin(); virtualTooth != m_virtualTeeth.end(); ++virtualTooth)
	{
		// The virtual tooth could be in contact with any number of objects in the scene.
		// Get its collisionMap.
		SurgSim::Collision::Representation::ContactMapType collisionsMap = (*virtualTooth)->getCollisions();

		// If the virtualTooth has no collision, continue to next loop iteration.
		if (collisionsMap.empty())
		{
			continue;
		}

		// Remove representations from the collision map that are not enabled to be stapled.
		filterCollisionMapForStapleEnabledRepresentations(&collisionsMap);

		// If the collision map is emptied after filtering, continue to next loop iteration.
		if (collisionsMap.empty())
		{
			continue;
		}

		// Filter the map based on supported Physics::Represention types.
		filterCollisionMapForSupportedRepresentationTypes(&collisionsMap);

		// If the collision map is emptied after filtering, continue to next loop iteration.
		if (collisionsMap.empty())
		{
			continue;
		}

		// Find the row (representation, list of contacts) in the map that the virtualTooth has most
		// collision pairs with.
		SurgSim::Collision::Representation::ContactMapType::value_type targetRepresentationContacts
			= *std::max_element(collisionsMap.begin(), collisionsMap.end(),
								[](const SurgSim::Collision::Representation::ContactMapType::value_type& lhs,
								   const SurgSim::Collision::Representation::ContactMapType::value_type& rhs)
								{ return lhs.second.size() < rhs.second.size(); });

		// Iterate through the list of collision pairs to find a contact with the deepest penetration.
		std::shared_ptr<SurgSim::Collision::Contact> targetContact
			= *std::max_element(targetRepresentationContacts.second.begin(), targetRepresentationContacts.second.end(),
								[](const std::shared_ptr<SurgSim::Collision::Contact>& lhs,
								   const std::shared_ptr<SurgSim::Collision::Contact>& rhs)
								{ return lhs->depth < rhs->depth; });

		// Create the staple, before creating the constaint with the staple.
		// The staple is created with no collision representation, because it is going to be constrained.
		if (!stapleAdded)
		{
			staple->setHasCollisionRepresentation(false);
			getScene()->addSceneElement(staple);
			// The gravity of the staple is disabled to prevent it from rotating about the line
			// connecting the two points of constraints on the staple.
			staple->getComponents<SurgSim::Physics::Representation>()[0]->setIsGravityEnabled(false);
			stapleAdded = true;
		}

		// Find the corresponding Phsyics::Representation for the target Collision::Representation.
		// Note that the targetPhysicsRepresentation will NOT be a nullptr, because the
		// collisionsMap was filtered earlier to remove Representations that returned nullptr
		// when the function findCorrespondingPhysicsRepresentation was called.
		// (see filterCollisionMapForStapleEnabledRepresentations above).
		std::shared_ptr<SurgSim::Physics::Representation> targetPhysicsRepresentation =
			findCorrespondingPhysicsRepresentation(targetRepresentationContacts.first);

		// Create a bilateral constraint between the targetPhysicsRepresentation and the staple.
		std::shared_ptr<SurgSim::Physics::Constraint> constraint =
			createBilateral3DConstraint(staple->getComponents<SurgSim::Physics::Representation>()[0],
										targetPhysicsRepresentation,
										targetContact->penetrationPoints.first);

		if (constraint == nullptr)
		{
			SURGSIM_LOG_WARNING(SurgSim::Framework::Logger::getDefaultLogger())
				<< "Failed to create constaint between staple and "
				<< targetRepresentationContacts.first->getSceneElement()->getName()
				<< ". This might be because the createBilateral3DConstraint is not supporting the Physics Type: "
				<< targetPhysicsRepresentation->getType();
			continue;
		}

		// Create a component to store this constraint.
		std::shared_ptr<SurgSim::Physics::ConstraintComponent> constraintComponent =
			std::make_shared<SurgSim::Physics::ConstraintComponent>(
				"Bilateral3DConstraint" + boost::to_string(toothId++));

		constraintComponent->setConstraint(constraint);
		staple->addComponent(constraintComponent);
	}

	if (!stapleAdded)
	{
		// Create the staple element.
		staple->setHasCollisionRepresentation(true);
		getScene()->addSceneElement(staple);
	}
}

void StaplerBehavior::update(double dt)
{
	SurgSim::DataStructures::DataGroup dataGroup;
	m_from->getData(&dataGroup);

	// Get the button1 index.
	if (!m_button1IndexCached)
	{
		m_button1Index = dataGroup.booleans().getIndex("button1");
		m_button1IndexCached = true;
	}

	// Check if the stapler is being pressed.
	bool button1 = false;
	dataGroup.booleans().get(m_button1Index, &button1);

	if (button1 && !m_buttonPreviouslyPressed)
	{
		createStaple();
	}

	m_buttonPreviouslyPressed = button1;
}

int StaplerBehavior::getTargetManagerType() const
{
	return SurgSim::Framework::MANAGER_TYPE_INPUT;
}

bool StaplerBehavior::doInitialize()
{
	SURGSIM_ASSERT(m_from) << "StaplerBehavior: no InputComponent held.";
	return true;
}

bool StaplerBehavior::doWakeUp()
{
	return true;
}
