// This file is a part of the OpenSurgSim project.
// Copyright 2016, SimQuest Solutions Inc.
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

#ifndef SURGSIM_PHYSICS_FEMCONSTRAINTFIXEDROTATIONVECTOR_H
#define SURGSIM_PHYSICS_FEMCONSTRAINTFIXEDROTATIONVECTOR_H

#include "SurgSim/Physics/ConstraintImplementation.h"

namespace SurgSim
{

namespace Physics
{

/// Base class for all FemRepresentation fixed rotation vector constraint implementation.
class FemConstraintFixedRotationVector : public ConstraintImplementation
{
public:
	/// Constructor
	FemConstraintFixedRotationVector();

	/// Destructor
	virtual ~FemConstraintFixedRotationVector();

	SurgSim::Physics::ConstraintType getConstraintType() const override;

private:
	size_t doGetNumDof() const override;

	/// \note The constraint violation being calculated based on a quaternion interpolation (slerp), and this
	/// type of interpolation being highly non-linear, the classical way of using the implementation one after the
	/// other one won't work.
	/// Therefore, the RotationVectorConstraint will use the vector mlcp->b to retrieve both representation's
	/// rotation vector, then calculate the proper slerp and set the violation back in mlcp->b
	void doBuild(double dt,
		const ConstraintData& data,
		const std::shared_ptr<Localization>& localization,
		MlcpPhysicsProblem* mlcp,
		size_t indexOfRepresentation,
		size_t indexOfConstraint,
		ConstraintSideSign sign) override;
};

}; // namespace Physics

}; // namespace SurgSim

#endif // SURGSIM_PHYSICS_FEMCONSTRAINTFIXEDROTATIONVECTOR_H
