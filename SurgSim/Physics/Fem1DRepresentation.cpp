// This file is a part of the OpenSurgSim project.
// Copyright 2013-2016, SimQuest Solutions Inc.
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

#include "SurgSim/DataStructures/IndexedLocalCoordinate.h"
#include "SurgSim/DataStructures/Location.h"
#include "SurgSim/Framework/Assert.h"
#include "SurgSim/Framework/Asset.h"
#include "SurgSim/Math/OdeState.h"
#include "SurgSim/Math/SparseMatrix.h"
#include "SurgSim/Physics/Fem1DElementBeam.h"
#include "SurgSim/Physics/Fem1DLocalization.h"
#include "SurgSim/Physics/Fem1DRepresentation.h"
#include "SurgSim/Physics/FemElement.h"
#include "SurgSim/Physics/Localization.h"

namespace
{
void transformVectorByBlockOf3(const SurgSim::Math::RigidTransform3d& transform, SurgSim::Math::Vector* x,
							   bool rotationOnly = false)
{
	typedef SurgSim::Math::Vector::Index IndexType;

	IndexType numNodes = x->size() / 6;

	SURGSIM_ASSERT(numNodes * 6 == x->size())
			<< "Unexpected number of dof in a Fem1D state vector (not a multiple of 6)";

	for (IndexType nodeId = 0; nodeId < numNodes; nodeId++)
	{
		// Only the translational dof are transformed, rotational dof remains unchanged
		SurgSim::Math::Vector3d xi = x->segment<3>(6 * nodeId);
		x->segment<3>(6 * nodeId) = (rotationOnly) ? transform.linear() * xi : transform * xi;
	}
}
}

namespace SurgSim
{
namespace Physics
{
SURGSIM_REGISTER(SurgSim::Framework::Component, SurgSim::Physics::Fem1DRepresentation, Fem1DRepresentation);

Fem1DRepresentation::Fem1DRepresentation(const std::string& name) : FemRepresentation(name)
{
	SURGSIM_ADD_SERIALIZABLE_PROPERTY(Fem1DRepresentation, std::shared_ptr<SurgSim::Framework::Asset>, Fem, getFem,
									  setFem);
	SURGSIM_ADD_SETTER(Fem1DRepresentation, std::string, FemFileName, loadFem)
	// Reminder: m_numDofPerNode is held by DeformableRepresentation but needs to be set by all
	// concrete derived classes
	m_numDofPerNode = 6;
	m_fem = std::make_shared<Fem1D>();
}

Fem1DRepresentation::~Fem1DRepresentation()
{
}

void Fem1DRepresentation::loadFem(const std::string& fileName)
{
	auto mesh = std::make_shared<Fem1D>();
	mesh->load(fileName);
	setFem(mesh);
}

void Fem1DRepresentation::setFem(std::shared_ptr<Framework::Asset> mesh)
{
	SURGSIM_ASSERT(!isInitialized()) << "The Fem cannot be set after initialization";

	SURGSIM_ASSERT(mesh != nullptr) << "Mesh for Fem1DRepresentation cannot be a nullptr";
	auto femMesh = std::dynamic_pointer_cast<Fem1D>(mesh);
	SURGSIM_ASSERT(femMesh != nullptr)
			<< "Mesh for Fem1DRepresentation needs to be a SurgSim::Physics::Fem1D";
	m_fem = femMesh;
	auto state = std::make_shared<Math::OdeState>();

	state->setNumDof(getNumDofPerNode(), m_fem->getNumVertices());
	for (size_t i = 0; i < m_fem->getNumVertices(); i++)
	{
		state->getPositions().segment<3>(getNumDofPerNode() * i) = m_fem->getVertexPosition(i);
	}
	for (auto boundaryCondition : m_fem->getBoundaryConditions())
	{
		state->addBoundaryCondition(boundaryCondition);
	}

	// If we have elements, ensure that they are all of the same nature
	if (femMesh->getNumElements() > 0)
	{
		const auto& e0 = femMesh->getElement(0);
		for (auto const& e : femMesh->getElements())
		{
			SURGSIM_ASSERT(e->nodeIds.size() == e0->nodeIds.size()) <<
				"Cannot mix and match elements of different nature." <<
				" Found an element with " << e->nodeIds.size() << " nodes but was expecting " << e0->nodeIds.size();
		}

		// If the FemElement types hasn't been registered yet, let's set a default one
		if (getFemElementType().empty())
		{
			if (e0->nodeIds.size() == 2)
			{
				Fem1DElementBeam beam;
				setFemElementType(beam.getClassName());
			}
		}
	}

	setInitialState(state);
}

std::shared_ptr<Fem1D> Fem1DRepresentation::getFem() const
{
	return m_fem;
}

void Fem1DRepresentation::addExternalGeneralizedForce(std::shared_ptr<Localization> localization,
													  const Math::Vector& generalizedForce,
													  const Math::Matrix& K,
													  const Math::Matrix& D)
{
	using Math::SparseMatrix;

	const size_t dofPerNode = getNumDofPerNode();
	const Math::Matrix::Index expectedSize = static_cast<const Math::Matrix::Index>(dofPerNode);

	SURGSIM_ASSERT(localization != nullptr) << "Invalid localization (nullptr)";
	SURGSIM_ASSERT(generalizedForce.size() == expectedSize) <<
				"Generalized force has an invalid size of " << generalizedForce.size() << ". Expected " << dofPerNode;
	SURGSIM_ASSERT(K.size() == 0 || (K.rows() == expectedSize && K.cols() == expectedSize)) <<
				"Stiffness matrix K has an invalid size (" << K.rows() << "," << K.cols() <<
				") was expecting a square matrix of size " << dofPerNode;
	SURGSIM_ASSERT(D.size() == 0 || (D.rows() == expectedSize && D.cols() == expectedSize)) <<
				"Damping matrix D has an invalid size (" << D.rows() << "," << D.cols() <<
				") was expecting a square matrix of size " << dofPerNode;

	std::shared_ptr<Fem1DLocalization> localization1D =
			std::dynamic_pointer_cast<Fem1DLocalization>(localization);
	SURGSIM_ASSERT(localization1D != nullptr) << "Invalid localization type (not a Fem1DLocalization)";

	const size_t elementId = localization1D->getLocalPosition().index;
	const Math::Vector& coordinate = localization1D->getLocalPosition().coordinate;
	std::shared_ptr<FemElement> element = getFemElement(elementId);

	size_t index = 0;
	for (auto nodeId : element->getNodeIds())
	{
		m_externalGeneralizedForce.segment(dofPerNode * nodeId, dofPerNode) += generalizedForce * coordinate[index];
		index++;
	}

	if (K.size() != 0 || D.size() != 0)
	{
		size_t index1 = 0;
		for (auto nodeId1 : element->getNodeIds())
		{
			size_t index2 = 0;
			for (auto nodeId2 : element->getNodeIds())
			{
				if (K.size() != 0)
				{
					Math::addSubMatrix(coordinate[index1] * coordinate[index2] * K,
									   nodeId1, nodeId2,
									   &m_externalGeneralizedStiffness, true);
				}
				if (D.size() != 0)
				{
					Math::addSubMatrix(coordinate[index1] * coordinate[index2] * D,
									   nodeId1, nodeId2,
									   &m_externalGeneralizedDamping, true);
				}
				index2++;
			}

			index1++;
		}
	}
	m_externalGeneralizedStiffness.makeCompressed();
	m_externalGeneralizedDamping.makeCompressed();
	m_hasExternalGeneralizedForce = true;
}


std::shared_ptr<Localization> Fem1DRepresentation::createNodeLocalization(size_t nodeId)
{
	DataStructures::IndexedLocalCoordinate coordinate;

	SURGSIM_ASSERT(nodeId >= 0 && nodeId < getCurrentState()->getNumNodes()) << "Invalid node id";

	// Look for any element that contains this node
	bool foundNodeId = false;
	for (size_t elementId = 0; elementId < getNumFemElements(); elementId++)
	{
		auto element = getFemElement(elementId);
		auto found = std::find(element->getNodeIds().begin(), element->getNodeIds().end(), nodeId);
		if (found != element->getNodeIds().end())
		{
			coordinate.index = elementId;
			coordinate.coordinate.setZero(element->getNumNodes());
			coordinate.coordinate[found - element->getNodeIds().begin()] = 1.0;
			foundNodeId = true;
			break;
		}
	}
	SURGSIM_ASSERT(foundNodeId) << "Could not find any element containing the node " << nodeId;

	// Fem1DLocalization will verify the coordinate (2nd parameter) based on
	// the Fem1DRepresentation passed as 1st parameter.
	return std::make_shared<Fem1DLocalization>(
		std::static_pointer_cast<Physics::Representation>(getSharedPtr()), coordinate);
}

std::shared_ptr<Localization> Fem1DRepresentation::createElementLocalization(
	const DataStructures::IndexedLocalCoordinate& location)
{
	return std::make_shared<Fem1DLocalization>(
		std::static_pointer_cast<Physics::Representation>(getSharedPtr()), location);
}

std::shared_ptr<Localization> Fem1DRepresentation::createLocalization(const DataStructures::Location& location)
{
	if (location.index.hasValue())
	{
		return createNodeLocalization(*location.index);
	}
	else if (location.elementMeshLocalCoordinate.hasValue())
	{
		return createElementLocalization(*location.elementMeshLocalCoordinate);
	}

	SURGSIM_FAILURE() << "Fem1DRepresentation supports only node and element-based location (not triangle-based)";

	return nullptr;
}

void Fem1DRepresentation::transformState(std::shared_ptr<Math::OdeState> state,
										 const Math::RigidTransform3d& transform)
{
	transformVectorByBlockOf3(transform, &state->getPositions());
	transformVectorByBlockOf3(transform, &state->getVelocities(), true);
}

bool Fem1DRepresentation::doInitialize()
{
	for (auto& element : m_fem->getElements())
	{
		std::shared_ptr<FemElement> femElement;
		femElement = FemElement::getFactory().create(getFemElementType(), element);
		m_femElements.push_back(femElement);
	}

	return FemRepresentation::doInitialize();
}

} // namespace Physics
} // namespace SurgSim
