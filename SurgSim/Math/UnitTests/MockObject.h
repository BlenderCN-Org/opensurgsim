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

#ifndef SURGSIM_MATH_UNITTESTS_MOCKOBJECT_H
#define SURGSIM_MATH_UNITTESTS_MOCKOBJECT_H

#include <Eigen/Core>
#include "SurgSim/Math/OdeEquation.h"
#include "SurgSim/Math/OdeSolver.h"
#include "SurgSim/Math/OdeState.h"

namespace SurgSim
{

namespace Math
{

class MassPointState : public OdeState
{
public:
	MassPointState() : OdeState()
	{
		setNumDof(3, 1);
		getPositions().setLinSpaced(1.0, 1.3);
		getVelocities().setLinSpaced(0.4, -0.3);
	}
};

class MassPoint : public OdeEquation
{
public:
	/// Constructor
	/// \param viscosity The mass viscosity
	explicit MassPoint(double viscosity = 0.0) :
		m_mass(0.456),
		m_viscosity(viscosity),
		m_gravity(0.0, -9.81, 0.0)
	{
		m_f.resize(3);
		m_M.resize(3, 3);
		m_D.resize(3, 3);
		m_K.resize(3, 3);
		this->m_initialState = std::make_shared<MassPointState>();
	}

	void disableGravity()
	{
		m_gravity.setZero();
	}

	Matrix applyCompliance(const OdeState& state, const Matrix& b) override
	{
		SURGSIM_ASSERT(m_odeSolver) << "Ode solver not initialized, it should have been initialized on wake-up";

		Math::Matrix bTemp = b;
		for (auto condition : state.getBoundaryConditions())
		{
			Math::zeroRow(condition, &bTemp);
		}
		auto solution = m_odeSolver->getLinearSolver()->solve(bTemp);
		for (auto condition : state.getBoundaryConditions())
		{
			Math::zeroRow(condition, &solution);
		}
		return solution;
	}

	void setOdeSolver(std::shared_ptr<SurgSim::Math::OdeSolver> solver)
	{
		m_odeSolver = solver;
	}

	/// Ode solver (its type depends on the numerical integration scheme)
	std::shared_ptr<SurgSim::Math::OdeSolver> m_odeSolver;
	double m_mass, m_viscosity;
	Vector3d m_gravity;

protected:
	void computeF(const OdeState& state) override
	{
		m_f = m_mass * m_gravity - m_viscosity * state.getVelocities();
	}

	void computeM(const OdeState& state) override
	{
		m_M.setIdentity();
		m_M *= m_mass;
	}

	void computeD(const OdeState& state) override
	{
		m_D.setIdentity();
		m_D *= m_viscosity;
	}

	void computeK(const OdeState& state) override
	{
		m_K.setZero();
	}

	void computeFMDK(const OdeState& state) override
	{
		m_M.setIdentity();
		m_M *= m_mass;
		m_D.setIdentity();
		m_D *= m_viscosity;
		m_K.setZero();
		m_f = m_mass * m_gravity - m_viscosity * state.getVelocities();
	}
};



/// State class for static resolution
/// It contains 3 nodes with 3 dofs each, with positions (0 0 0) (1 0 0) (2 0 0) and null velocities/accelerations
class MassPointsStateForStatic : public OdeState
{
public:
	MassPointsStateForStatic() : OdeState()
	{
		setNumDof(3, 3);
		getPositions().segment<3>(3) = Vector3d(1.0, 0.0, 0.0);
		getPositions().segment<3>(6) = Vector3d(2.0, 0.0, 0.0);
	}
};

// Model of 3 nodes connected by springs with the 1st node fixed (no mass, no damping, only deformations)
class MassPointsForStatic : public SurgSim::Math::OdeEquation
{
public:
	/// Constructor
	MassPointsForStatic() :
		m_gravityForce(9)
	{
		m_f.resize(9);
		m_M.resize(9, 9);
		m_D.resize(9, 9);
		m_K.resize(9, 9);

		m_f.setZero();
		m_K.setZero();
		m_gravityForce.setZero();
		m_gravityForce.segment<3>(3) = Vector3d(0.0, 0.01 * -9.81, 0.0);
		m_gravityForce.segment<3>(6) = Vector3d(0.0, 0.01 * -9.81, 0.0);

		this->m_initialState = std::make_shared<MassPointsStateForStatic>();
	}

	const Vector& getExternalForces() const
	{
		return m_gravityForce;
	}

	Matrix applyCompliance(const OdeState& state, const Matrix& b) override
	{
		SURGSIM_ASSERT(m_odeSolver) << "Ode solver not initialized, it should have been initialized on wake-up";

		Math::Matrix bTemp = b;
		for (auto condition : state.getBoundaryConditions())
		{
			Math::zeroRow(condition, &bTemp);
		}
		auto solution = m_odeSolver->getLinearSolver()->solve(bTemp);
		for (auto condition : state.getBoundaryConditions())
		{
			Math::zeroRow(condition, &solution);
		}
		return solution;
	}

	void setOdeSolver(std::shared_ptr<SurgSim::Math::OdeSolver> solver)
	{
		m_odeSolver = solver;
	}

protected:
	void computeF(const OdeState& state) override
	{
		computeK(state);

		// Internal deformation forces
		m_f = -m_K * (state.getPositions() - m_initialState->getPositions());

		// Gravity pulling on the free nodes
		m_f += m_gravityForce;
	}

	void computeM(const OdeState& state) override
	{
		m_M.setZero();
	}

	void computeD(const OdeState& state) override
	{
		m_D.setZero();
	}

	void computeK(const OdeState& state) override
	{
		// A fake but valid stiffness matrix (node 0 fixed)
		// Desired matrix is:
		//
		// 1  0  0  0  0  0  0  0  0
		// 0  1  0  0  0  0  0  0  0
		// 0  0  1  0  0  0  0  0  0
		// 0  0  0 10  2  2  2  2  2
		// 0  0  0  2 10  2  2  2  2
		// 0  0  0  2  2 10  2  2  2
		// 0  0  0  2  2  2 10  2  2
		// 0  0  0  2  2  2  2 10  2
		// 0  0  0  2  2  2  2  2 10
		//
		// Generate it using sparse matrix notation.

		m_K.resize(9, 9);
		typedef Eigen::Triplet<double> T;
		std::vector<T> tripletList;
		tripletList.reserve(39);

		// Upper 3x3 identity block
		for (int counter = 0; counter < 3; ++counter)
		{
			tripletList.push_back(T(counter, counter, 1.0));
		}

		for (int row = 3; row < 9; ++row)
		{
			// Diagonal is 8 more than the rest of the 6x6 block
			tripletList.push_back(T(row, row, 8.0));

			// Now add in the 2's over the entire block
			for (int col = 3; col < 9; ++col)
			{
				tripletList.push_back(T(row, col, 2.0));
			}
		}
		m_K.setFromTriplets(tripletList.begin(), tripletList.end());
	}

	void computeFMDK(const OdeState& state) override
	{
		computeF(state);
		computeM(state);
		computeD(state);
		computeK(state);
	}

private:
	/// Ode solver (its type depends on the numerical integration scheme)
	std::shared_ptr<SurgSim::Math::OdeSolver> m_odeSolver;
	Vector m_gravityForce;
};

/// Class for the complex non-linear ODE a = x.v^2
class OdeComplexNonLinear : public OdeEquation
{
public:
	/// Constructor
	OdeComplexNonLinear()
	{
		m_f.resize(3);
		m_M.resize(3, 3);
		m_D.resize(3, 3);
		m_K.resize(3, 3);
		this->m_initialState = std::make_shared<MassPointState>();
	}

	Matrix applyCompliance(const OdeState& state, const Matrix& b) override
	{
		SURGSIM_ASSERT(m_odeSolver) << "Ode solver not initialized, it should have been initialized on wake-up";

		Math::Matrix bTemp = b;
		for (auto condition : state.getBoundaryConditions())
		{
			Math::zeroRow(condition, &bTemp);
		}
		auto solution = m_odeSolver->getLinearSolver()->solve(bTemp);
		for (auto condition : state.getBoundaryConditions())
		{
			Math::zeroRow(condition, &solution);
		}
		return solution;
	}

	void setOdeSolver(std::shared_ptr<SurgSim::Math::OdeSolver> solver)
	{
		m_odeSolver = solver;
	}

protected:
	void computeF(const OdeState& state) override
	{
		double v2 = state.getVelocities().dot(state.getVelocities());
		m_f = v2 * state.getPositions();
	}

	void computeM(const OdeState& state) override
	{
		m_M.setIdentity();
	}

	void computeD(const OdeState& state) override
	{
		m_D = 2.0 * state.getPositions() * state.getVelocities().transpose().sparseView();
	}

	void computeK(const OdeState& state) override
	{
		m_K.resize(static_cast<SparseMatrix::Index>(state.getNumDof()),
			static_cast<SparseMatrix::Index>(state.getNumDof()));
		m_K.setIdentity();
		m_K *= state.getVelocities().squaredNorm();
	}

	void computeFMDK(const OdeState& state) override
	{
		computeF(state);
		computeM(state);
		computeD(state);
		computeK(state);
	}

private:
	/// Ode solver (its type depends on the numerical integration scheme)
	std::shared_ptr<SurgSim::Math::OdeSolver> m_odeSolver;
};

}; // Math

}; // SurgSim

#endif // SURGSIM_MATH_UNITTESTS_MOCKOBJECT_H
