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

#ifndef SURGSIM_PHYSICS_RIGIDREPRESENTATIONCONTACT_H
#define SURGSIM_PHYSICS_RIGIDREPRESENTATIONCONTACT_H

#include <SurgSim/Physics/Constraint.h>
#include <SurgSim/Physics/ConstraintData.h>
#include <SurgSim/Physics/ConstraintImplementation.h>
#include <SurgSim/Physics/RigidRepresentation.h>
#include <SurgSim/Physics/Localization.h>

namespace SurgSim
{

namespace Physics
{

/// RigidRepresentation frictionless contact implementation
class RigidRepresentationContact : public ConstraintImplementation
{
public:
	/// Constructor
	/// \param localization The localization of the contact on the rigid representation
	explicit RigidRepresentationContact(std::shared_ptr<Localization> localization) :
	ConstraintImplementation(localization)
	{}

	/// Destructor
	virtual ~RigidRepresentationContact()
	{}

private:
	/// Gets the number of degree of freedom for a frictionless contact
	/// \return 1 as a frictionless contact only has 1 equation of constraint (along the normal direction)
	unsigned int doGetNumDof() const override
	{
		return 1;
	}

	/// Builds the subset of an Mlcp physics problem associated to this implementation
	/// \param dt The time step
	/// \param data The data associated to the constraint
	/// \param [in, out] mlcp The Mixed LCP physics problem to fill up
	/// \param indexOfRepresentation The index of the representation (associated to this implementation) in the mlcp
	/// \param indexOfConstraint The index of the constraint in the mlcp
	/// \param sign The sign of this implementation in the constraint (positive or negative side)
	void doBuild(double dt,
		const ConstraintData& data,
		MlcpPhysicsProblem* mlcp,
		unsigned int indexOfRepresentation,
		unsigned int indexOfConstraint,
		ConstraintSideSign sign) override;

	/// Gets the Mixed Linear Complementarity Problem constraint type for this ConstraintImplementation
	/// \return The MLCP constraint type corresponding to this constraint implementation
	SurgSim::Math::MlcpConstraintType doGetMlcpConstraintType() const override
	{
		return SurgSim::Math::MLCP_UNILATERAL_3D_FRICTIONLESS_CONSTRAINT;
	}
};

};  // namespace Physics

};  // namespace SurgSim

#endif  // SURGSIM_PHYSICS_RIGIDREPRESENTATIONCONTACT_H
