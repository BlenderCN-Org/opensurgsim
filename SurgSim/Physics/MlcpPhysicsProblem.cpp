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

#include "SurgSim/Physics/MlcpPhysicsProblem.h"

namespace SurgSim
{

namespace Physics
{

MlcpPhysicsProblem::~MlcpPhysicsProblem()
{
}

void MlcpPhysicsProblem::setZero(size_t numDof, size_t numConstraintDof, size_t numConstraints)
{
	MlcpProblem::setZero(numDof, numConstraintDof, numConstraints);

	H.resize(numConstraintDof, numDof);
	CHt.setZero(numDof, numConstraintDof);
}

MlcpPhysicsProblem MlcpPhysicsProblem::Zero(size_t numDof, size_t numConstraintDof, size_t numConstraints)
{
	MlcpPhysicsProblem result;
	result.setZero(numDof, numConstraintDof, numConstraints);

	return result;
}

}; // namespace Physics

}; // namespace SurgSim
