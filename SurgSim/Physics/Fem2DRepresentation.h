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

#ifndef SURGSIM_PHYSICS_FEM2DREPRESENTATION_H
#define SURGSIM_PHYSICS_FEM2DREPRESENTATION_H

#include <memory>
#include <string>

#include "SurgSim/Math/Matrix.h"
#include "SurgSim/Physics/FemRepresentation.h"

namespace SurgSim
{

namespace Physics
{
class Fem2DRepresentationPlyReaderDelegate;

/// Finite Element Model 2D is a fem built with 2D FemElement
class Fem2DRepresentation : public FemRepresentation
{
public:
	/// Constructor
	/// \param name The name of the Fem2DRepresentation
	explicit Fem2DRepresentation(const std::string& name);

	/// Destructor
	virtual ~Fem2DRepresentation();

	virtual RepresentationType getType() const override;

protected:
	virtual void transformState(std::shared_ptr<SurgSim::Math::OdeState> state,
								const SurgSim::Math::RigidTransform3d& transform) override;

private:
	virtual std::shared_ptr<FemRepresentationPlyReaderDelegate> getDelegate() override;
};

} // namespace Physics

} // namespace SurgSim

#endif // SURGSIM_PHYSICS_FEM2DREPRESENTATION_H
