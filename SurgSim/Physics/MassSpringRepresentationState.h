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

#ifndef SURGSIM_PHYSICS_MASSSPRINGREPRESENTATIONSTATE_H
#define SURGSIM_PHYSICS_MASSSPRINGREPRESENTATIONSTATE_H

#include <SurgSim/DataStructures/TetrahedronMesh.h>
#include <SurgSim/Math/Vector.h>
#include <SurgSim/Math/Matrix.h>

using SurgSim::DataStructures::TetrahedronMesh;
using SurgSim::Math::Vector3d;
using SurgSim::Math::Matrix33d;

namespace SurgSim
{

namespace Physics
{

// Class to handle the mass for each node of a MassSpring (contains mass and velocity).
// The position is held by the Vertices class, this is the extra information per vertex
class MassParameter
{
public:
	/// Constructor
	MassParameter() :
	  m_mass(0.0), m_velocity(0.0, 0.0, 0.0)
	{
	}

	/// Set the mass
	/// \param mass The mass to be stored (in Kg)
	void setMass(double mass)
	{
		m_mass = mass;
	}
	/// Get the mass
	/// \return The mass stored (in Kg)
	double getMass() const
	{
		return m_mass;
	}

	/// Set the velocity
	/// \param velocity The velocity to be stored (in m.s-1)
	void setVelocity(const Vector3d& velocity)
	{
		m_velocity = velocity;
	}
	/// Get the velocity
	/// \return The velocity stored (in m.s-1)
	const Vector3d& getVelocity() const
	{
		return m_velocity;
	}

	/// Comparison operator (equality)
	/// \param m MassParameter to compare it to
	/// \return True if the 2 MassParameter contains the same information, false otherwise
	bool operator ==(const MassParameter& m) const
	{
		return (m_mass == m.m_mass && m_velocity == m.m_velocity);
	}
	/// Comparison operator (inequality)
	/// \param m MassParameter to compare it to
	/// \return False if the 2 MassParameter contains the same information, True otherwise
	bool operator !=(const MassParameter& m) const
	{
		return !((*this) == m);
	}

protected:
	/// Mass (in kg)
	double m_mass;
	/// Velocity (in m.s-1)
	Vector3d m_velocity;
};

// Class to handle the spring for each node of a MassSpring (contains spring parameters).
// The force and stiffness generated by this spring is computed on the fly, not stored.
class LinearSpringParameter
{
public:
	/// Constructor
	LinearSpringParameter() :
	  m_l0(0.0), m_stiffness(0.0), m_damping(0.0)
	{
	}

	/// Set the spring stiffness parameter
	/// \param stiffness The stiffness to assign to the spring (in N.m-1)
	void setStiffness(double stiffness)
	{
		m_stiffness = stiffness;
	}
	/// Get the spring stiffness parameter
	/// \return The stiffness assigned to the spring (in N.m-1)
	double getStiffness() const
	{
		return m_stiffness;
	}

	/// Set the spring damping parameter
	/// \param damping The damping to assign to the spring (in N.s.m-1)
	void setDamping(double damping)
	{
		m_damping = damping;
	}
	/// Get the spring damping parameter
	/// \return The damping assigned to the spring (in N.s.m-1)
	double getDamping() const
	{
		return m_damping;
	}

	/// Set the rest length of the spring
	/// \param l0 The rest length to assign to the spring (in m)
	void setInitialLength(double l0)
	{
		m_l0 = l0;
	}
	/// Get the rest length of the spring
	/// \return The rest length assigned to the spring (in m)
	double getInitialLength() const
	{
		return m_l0;
	}

	/// Get the spring force given a state
	/// \param xA, xB The positions of the 2 nodes
	/// \param vA, vB The velocities of the 2 nodes
	/// \return The Force on the 1st node. The force on the 2nd node is the opposite.
	template <class M>
	const Vector3d getF(
		const Eigen::VectorBlock<M>& xA,
		const Eigen::VectorBlock<M>& xB,
		const Eigen::VectorBlock<M>& vA,
		const Eigen::VectorBlock<M>& vB) const
	{
		Vector3d u = xB - xA;
		double m_l = u.norm();
		u /= m_l;

		return u * (m_l - m_l0) * m_stiffness;
	}

	/// Comparison operator (equality)
	/// \param s LinearSpringParameter to compare it to
	/// \return True if the 2 LinearSpringParameter contains the same information, false otherwise
	bool operator ==(const LinearSpringParameter& s) const
	{
		return m_l0 == s.m_l0 && m_stiffness == s.m_stiffness && m_damping == s.m_damping;
	}

	/// Comparison operator (inequality)
	/// \param s LinearSpringParameter to compare it to
	/// \return False if the 2 LinearSpringParameter contains the same information, true otherwise
	bool operator !=(const LinearSpringParameter& m) const
	{
		return !((*this) == m);
	}

private:
	/// Rest length (in m)
	double m_l0;
	/// Stiffness parameters (in N.m-1)
	double m_stiffness;
	/// Damping parameters (in N.s.m-1)
	double m_damping;
};

/// MassSpringRepresentationState is simply a TetMesh with masses on vertices and springs on edges.
typedef TetrahedronMesh<MassParameter, LinearSpringParameter, void, void> MassSpringRepresentationState;

} // namespace Physics

} // namespace SurgSim

#endif // SURGSIM_PHYSICS_MASSSPRINGREPRESENTATIONSTATE_H