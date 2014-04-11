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

#ifndef SURGSIM_PHYSICS_RIGIDREPRESENTATION_H
#define SURGSIM_PHYSICS_RIGIDREPRESENTATION_H

#include "SurgSim/Physics/RigidRepresentationBase.h"

#include "SurgSim/Math/Vector.h"
#include "SurgSim/Math/Matrix.h"
#include "SurgSim/Math/RigidTransform.h"

namespace SurgSim
{

namespace Physics
{
class RigidRepresentationState;

/// The RigidRepresentation class defines the dynamic rigid body representation
/// Note that the rigid representation is velocity-based, therefore its degrees of
/// freedom are the linear and angular velocities: 6 Dof
class RigidRepresentation : public RigidRepresentationBase
{
public:

	/// Constructor
	/// \param name The rigid representation's name
	explicit RigidRepresentation(const std::string& name);

	/// Destructor
	virtual ~RigidRepresentation();

	virtual RepresentationType getType() const override;

	/// Set the current pose of the rigid representation
	/// \param pose The current pose (translation + rotation)
	/// \note Does not apply to this representation (the pose is fully controlled by the
	/// physics simulation).
	virtual void setPose(const SurgSim::Math::RigidTransform3d& pose) override;

	/// Set the current linear velocity of the rigid representation
	/// \param linearVelocity The linear velocity
	void setLinearVelocity(const SurgSim::Math::Vector3d& linearVelocity);

	/// Set the current angular velocity of the rigid representation
	/// \param angularVelocity The angular velocity
	void setAngularVelocity(const SurgSim::Math::Vector3d& angularVelocity);

	/// Set the external force being applied to the rigid representation
	/// Note this force will be zeroed every update of the rigid representation
	/// \param force The external force
	/// \param K The stiffness matrix associated with the force (jacobian of the force w.r.t position)
	/// \param D The damping matrix associated with the force (jacobian of the force w.r.t velocity)
	void addExternalForce(const SurgSim::Math::Vector3d& force,
						  const SurgSim::Math::Matrix33d& K = SurgSim::Math::Matrix33d::Zero(),
						  const SurgSim::Math::Matrix33d& D = SurgSim::Math::Matrix33d::Zero());

	/// Set the external torque being applied to the rigid representation
	/// Note this torque will be zeroed every update of the rigid representation
	/// \param torque The external torque
	/// \param K The angular stiffness matrix associated with the torque (jacobian of the torque w.r.t position)
	/// \param D The angular damping matrix associated with the torque (jacobian of the torque w.r.t velocity)
	void addExternalTorque(const SurgSim::Math::Vector3d& torque,
						   const SurgSim::Math::Matrix33d& K = SurgSim::Math::Matrix33d::Zero(),
						   const SurgSim::Math::Matrix33d& D = SurgSim::Math::Matrix33d::Zero());

	virtual void beforeUpdate(double dt) override;

	virtual	void update(double dt) override;

	virtual	void afterUpdate(double dt) override;

	void applyCorrection(double dt, const Eigen::VectorBlock<SurgSim::Math::Vector>& deltaVelocity) override;

	virtual void resetParameters() override;

	/// Retrieve the rigid body 6x6 compliance matrix
	/// \return the 6x6 compliance matrix
	const SurgSim::Math::Matrix66d& getComplianceMatrix() const;

protected:
	/// Inertia matrices in global coordinates
	SurgSim::Math::Matrix33d m_globalInertia;
	/// Inverse of inertia matrix in global coordinates
	SurgSim::Math::Matrix33d m_invGlobalInertia;

	/// Current force applied on the rigid representation (in N)
	SurgSim::Math::Vector3d m_force;
	/// Current torque applied on the rigid representation (in N.m)
	SurgSim::Math::Vector3d m_torque;

	/// Compliance matrix (size of the number of Dof = 6)
	SurgSim::Math::Matrix66d m_C;

	SurgSim::Math::Vector3d m_externalForce;
	SurgSim::Math::Vector3d m_externalTorque;
	SurgSim::Math::Matrix66d m_externalStiffnessMatrix;
	SurgSim::Math::Matrix66d m_externalDampingMatrix;

private:
	virtual bool doInitialize() override;

	/// Compute compliance matrix (internal data structure)
	/// \param dt The time step in use
	void computeComplianceMatrix(double dt);

	/// Update global inertia matrices (internal data structure)
	/// \param state The state of the rigid representation to use for the update
	virtual void updateGlobalInertiaMatrices(const RigidRepresentationState& state) override;


};

}; // Physics

}; // SurgSim

#endif // SURGSIM_PHYSICS_RIGIDREPRESENTATION_H
