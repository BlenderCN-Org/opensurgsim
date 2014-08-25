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

#include "SurgSim/Framework/Macros.h"
#include "SurgSim/Framework/ObjectFactory.h"
#include "SurgSim/Math/Vector.h"
#include "SurgSim/Math/Matrix.h"
#include "SurgSim/Physics/RigidRepresentationBase.h"

namespace SurgSim
{

namespace Physics
{
class RigidRepresentationState;
class Localization;

SURGSIM_STATIC_REGISTRATION(RigidRepresentation);

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

	SURGSIM_CLASSNAME(SurgSim::Physics::RigidRepresentation);

	virtual RepresentationType getType() const override;

	/// Set the current linear velocity of the rigid representation
	/// \param linearVelocity The linear velocity
	void setLinearVelocity(const SurgSim::Math::Vector3d& linearVelocity);

	/// Set the current angular velocity of the rigid representation
	/// \param angularVelocity The angular velocity
	void setAngularVelocity(const SurgSim::Math::Vector3d& angularVelocity);

	/// Add an external generalized force applied to the rigid representation's mass center
	/// Note this force is generalized (i.e. it's a 6D vector, containing both 3D force and 3D torque)
	/// Note the stiffness and damping are 6x6 matrices with coupling between the translational and rotation dof.
	/// Note this generalized force will be zeroed every afterUpdate call of the rigid representation
	/// \param generalizedForce The external generalized force to apply at the mass center
	/// \param K The stiffness matrix associated with the generalized force (jacobian of the force w.r.t position)
	/// \param D The damping matrix associated with the generalized force (jacobian of the force w.r.t velocity)
	void addExternalGeneralizedForce(const SurgSim::Math::Vector6d& generalizedForce,
									 const SurgSim::Math::Matrix66d& K = SurgSim::Math::Matrix66d::Zero(),
									 const SurgSim::Math::Matrix66d& D = SurgSim::Math::Matrix66d::Zero());

	/// Add an external generalized force applied to the rigid representation (anywhere)
	/// Note this force is generalized (i.e. it's a 6D vector, containing both 3D force and 3D torque)
	/// Note the stiffness and damping are 6x6 matrices with coupling between the translational and rotation dof.
	/// Note this generalized force will be zeroed every afterUpdate call of the rigid representation
	/// \param localization The application point
	/// \param generalizedForce The external generalized force
	/// \param K The stiffness matrix associated with the generalized force (jacobian of the force w.r.t position)
	/// \param D The damping matrix associated with the generalized force (jacobian of the force w.r.t velocity)
	void addExternalGeneralizedForce(std::shared_ptr<Localization> localization,
									 const SurgSim::Math::Vector6d& generalizedForce,
									 const SurgSim::Math::Matrix66d& K = SurgSim::Math::Matrix66d::Zero(),
									 const SurgSim::Math::Matrix66d& D = SurgSim::Math::Matrix66d::Zero());

	/// \return the current external generalized 6D force
	const SurgSim::Math::Vector6d& getExternalGeneralizedForce() const;

	/// \return the current external generalized stiffness 6x6 matrix
	const SurgSim::Math::Matrix66d& getExternalGeneralizedStiffness() const;

	/// \return the current external generalized damping 6x6 matrix
	const SurgSim::Math::Matrix66d& getExternalGeneralizedDamping() const;

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

	SurgSim::Math::Vector6d m_externalGeneralizedForce;
	SurgSim::Math::Matrix66d m_externalGeneralizedStiffness;
	SurgSim::Math::Matrix66d m_externalGeneralizedDamping;

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
