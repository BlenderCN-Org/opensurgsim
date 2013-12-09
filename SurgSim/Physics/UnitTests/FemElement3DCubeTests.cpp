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

#include <gtest/gtest.h>

#include <memory>
#include <array>

#include "SurgSim/Physics/FemElement3DCube.h"
#include "SurgSim/Physics/DeformableRepresentationState.h"
#include "SurgSim/Math/Vector.h"
#include "SurgSim/Math/Matrix.h"

using SurgSim::Math::Matrix;
using SurgSim::Math::Vector;
using SurgSim::Math::Vector3d;
using SurgSim::Physics::DeformableRepresentationState;
using SurgSim::Physics::FemElement3DCube;

namespace
{
const double epsilon = 2.6e-9;
};

class MockFemElement3DCube : public FemElement3DCube
{
public:
	MockFemElement3DCube(std::array<unsigned int, 8> nodeIds, const DeformableRepresentationState& restState) :
		FemElement3DCube(nodeIds, restState)
	{
	}

	double getRestVolume() const
	{
		return m_restVolume;
	}

	double evaluateN(int i, double epsilon, double neta, double mu) const
	{
		return N(i, epsilon, neta, mu);
	}

	double evaluatedNidEpsilon(int i, double epsilon, double neta, double mu) const
	{
		return dNdepsilon(i, epsilon, neta, mu);
	}

	double evaluatedNidNeta(int i, double epsilon, double neta, double mu) const
	{
		return dNdneta(i, epsilon, neta, mu);
	}

	double evaluatedNidMu(int i, double epsilon, double neta, double mu) const
	{
		return dNdmu(i, epsilon, neta, mu);
	}

	const Eigen::Matrix<double, 24, 1, Eigen::DontAlign>& getInitialPosition() const
	{
		return m_x0;
	}
};

class FemElement3DCubeTests : public ::testing::Test
{
public:
	std::array<unsigned int, 8> m_nodeIds;
	DeformableRepresentationState m_restState;
	double m_expectedVolume;
	Eigen::Matrix<double, 24, 1, Eigen::DontAlign> m_expectedX0;
	double m_rho, m_E, m_nu;
	SurgSim::Math::Matrix m_expectedMassMatrix, m_expectedDampingMatrix, m_expectedStiffnessMatrix;
	SurgSim::Math::Vector m_vectorOnes;

	void computeExpectedStiffnessMatrix(std::vector<unsigned int> nodeIdsVectorForm)
	{
		using SurgSim::Math::getSubMatrix;
		using SurgSim::Math::addSubMatrix;

		Eigen::Matrix<double, 24, 24, Eigen::DontAlign> K;
		K.setZero();
		{
			// Expected stiffness matrix given in
			// "Physically-Based Simulation of Objects Represented by Surface Meshes"
			// Muller, Techner, Gross, CGI 2004
			// NOTE a bug in the paper, the sub matrix K44 should have a diagonal of 'd'
			double h = 1.0;
			double a = h * m_E * (1.0 - m_nu) / ((1.0 + m_nu) * (1.0 - 2.0 * m_nu));
			double b = h * m_E * (m_nu) / ((1.0 + m_nu) * (1.0 - 2.0 * m_nu));
			double c = h * m_E / (2.0 * (1.0 + m_nu));

			double d = (a + 2.0 * c) / 9.0;
			double e = (b + c) / 12.0;
			double n = -e;
			// Fill up the diagonal sub-matrices (3x3)
			getSubMatrix(K, 0, 0, 3, 3).setConstant(e);
			getSubMatrix(K, 0, 0, 3, 3).diagonal().setConstant(d);

			getSubMatrix(K, 1, 1, 3, 3).setConstant(n);
			getSubMatrix(K, 1, 1, 3, 3).diagonal().setConstant(d);
			getSubMatrix(K, 1, 1, 3, 3)(1,2) = e;
			getSubMatrix(K, 1, 1, 3, 3)(2,1) = e;

			getSubMatrix(K, 2, 2, 3, 3).setConstant(n);
			getSubMatrix(K, 2, 2, 3, 3).diagonal().setConstant(d);
			getSubMatrix(K, 2, 2, 3, 3)(0,1) = e;
			getSubMatrix(K, 2, 2, 3, 3)(1,0) = e;

			getSubMatrix(K, 3, 3, 3, 3).setConstant(n);
			getSubMatrix(K, 3, 3, 3, 3).diagonal().setConstant(d);
			getSubMatrix(K, 3, 3, 3, 3)(0,2) = e;
			getSubMatrix(K, 3, 3, 3, 3)(2,0) = e;

			getSubMatrix(K, 4, 4, 3, 3).setConstant(n);
			getSubMatrix(K, 4, 4, 3, 3).diagonal().setConstant(d);
			//getSubMatrix(K, 4, 4, 3, 3)(0,0) = e; // BUG IN THE PAPER !!!
			getSubMatrix(K, 4, 4, 3, 3)(0,1) = e;
			getSubMatrix(K, 4, 4, 3, 3)(1,0) = e;

			getSubMatrix(K, 5, 5, 3, 3).setConstant(n);
			getSubMatrix(K, 5, 5, 3, 3).diagonal().setConstant(d);
			getSubMatrix(K, 5, 5, 3, 3)(0,2) = e;
			getSubMatrix(K, 5, 5, 3, 3)(2,0) = e;

			getSubMatrix(K, 6, 6, 3, 3).setConstant(e);
			getSubMatrix(K, 6, 6, 3, 3).diagonal().setConstant(d);

			getSubMatrix(K, 7, 7, 3, 3).setConstant(n);
			getSubMatrix(K, 7, 7, 3, 3).diagonal().setConstant(d);
			getSubMatrix(K, 7, 7, 3, 3)(1,2) = e;
			getSubMatrix(K, 7, 7, 3, 3)(2,1) = e;

			// Edges
			{
				double d1 = (-a + c) / 9.0;
				double d2 = (a - c) / 18.0;
				double e1 = (b - c) / 12.0;
				double e2 = (b + c) / 24.0;
				double n1 = -e1;
				double n2 = -e2;

				// Edge in x-direction
				getSubMatrix(K, 0, 1, 3, 3)(0, 0) = d1;
				getSubMatrix(K, 0, 1, 3, 3)(0, 1) = e1;
				getSubMatrix(K, 0, 1, 3, 3)(0, 2) = e1;
				getSubMatrix(K, 0, 1, 3, 3)(1, 0) = n1;
				getSubMatrix(K, 0, 1, 3, 3)(1, 1) = d2;
				getSubMatrix(K, 0, 1, 3, 3)(1, 2) = e2;
				getSubMatrix(K, 0, 1, 3, 3)(2, 0) = n1;
				getSubMatrix(K, 0, 1, 3, 3)(2, 1) = e2;
				getSubMatrix(K, 0, 1, 3, 3)(2, 2) = d2;

				getSubMatrix(K, 2, 3, 3, 3) = getSubMatrix(K, 0, 1, 3, 3);
				getSubMatrix(K, 2, 3, 3, 3)(0, 2) = n1;
				getSubMatrix(K, 2, 3, 3, 3)(1, 2) = n2;
				getSubMatrix(K, 2, 3, 3, 3)(2, 0) = e1;
				getSubMatrix(K, 2, 3, 3, 3)(2, 1) = n2;

				getSubMatrix(K, 4, 5, 3, 3) = getSubMatrix(K, 2, 3, 3, 3);

				getSubMatrix(K, 6, 7, 3, 3) = getSubMatrix(K, 0, 1, 3, 3);

				// Edge in y-direction
				getSubMatrix(K, 0, 3, 3, 3)(0, 0) = d2;
				getSubMatrix(K, 0, 3, 3, 3)(0, 1) = n1;
				getSubMatrix(K, 0, 3, 3, 3)(0, 2) = e2;
				getSubMatrix(K, 0, 3, 3, 3)(1, 0) = e1;
				getSubMatrix(K, 0, 3, 3, 3)(1, 1) = d1;
				getSubMatrix(K, 0, 3, 3, 3)(1, 2) = e1;
				getSubMatrix(K, 0, 3, 3, 3)(2, 0) = e2;
				getSubMatrix(K, 0, 3, 3, 3)(2, 1) = n1;
				getSubMatrix(K, 0, 3, 3, 3)(2, 2) = d2;

				getSubMatrix(K, 1, 2, 3, 3) = getSubMatrix(K, 0, 3, 3, 3);
				getSubMatrix(K, 1, 2, 3, 3)(0, 1) = e1;
				getSubMatrix(K, 1, 2, 3, 3)(0, 2) = n2;
				getSubMatrix(K, 1, 2, 3, 3)(1, 0) = n1;
				getSubMatrix(K, 1, 2, 3, 3)(2, 0) = n2;

				getSubMatrix(K, 4, 7, 3, 3) = getSubMatrix(K, 1, 2, 3, 3);
				getSubMatrix(K, 4, 7, 3, 3)(0, 1) = n1;
				getSubMatrix(K, 4, 7, 3, 3)(1, 0) = e1;
				getSubMatrix(K, 4, 7, 3, 3)(1, 2) = n1;
				getSubMatrix(K, 4, 7, 3, 3)(2, 1) = e1;

				getSubMatrix(K, 5, 6, 3, 3) = getSubMatrix(K, 0, 3, 3, 3);
				getSubMatrix(K, 5, 6, 3, 3)(0, 1) = e1;
				getSubMatrix(K, 5, 6, 3, 3)(1, 0) = n1;
				getSubMatrix(K, 5, 6, 3, 3)(1, 2) = n1;
				getSubMatrix(K, 5, 6, 3, 3)(2, 1) = e1;

				// Edge in z-direction
				getSubMatrix(K, 0, 4, 3, 3)(0, 0) = d2;
				getSubMatrix(K, 0, 4, 3, 3)(0, 1) = e2;
				getSubMatrix(K, 0, 4, 3, 3)(0, 2) = n1;
				getSubMatrix(K, 0, 4, 3, 3)(1, 0) = e2;
				getSubMatrix(K, 0, 4, 3, 3)(1, 1) = d2;
				getSubMatrix(K, 0, 4, 3, 3)(1, 2) = n1;
				getSubMatrix(K, 0, 4, 3, 3)(2, 0) = e1;
				getSubMatrix(K, 0, 4, 3, 3)(2, 1) = e1;
				getSubMatrix(K, 0, 4, 3, 3)(2, 2) = d1;

				getSubMatrix(K, 1, 5, 3, 3) = getSubMatrix(K, 0, 4, 3, 3);
				getSubMatrix(K, 1, 5, 3, 3)(0, 1) = n2;
				getSubMatrix(K, 1, 5, 3, 3)(0, 2) = e1;
				getSubMatrix(K, 1, 5, 3, 3)(1, 0) = n2;
				getSubMatrix(K, 1, 5, 3, 3)(2, 0) = n1;

				getSubMatrix(K, 2, 6, 3, 3) = getSubMatrix(K, 0, 4, 3, 3);
				getSubMatrix(K, 2, 6, 3, 3)(0, 2) = e1;
				getSubMatrix(K, 2, 6, 3, 3)(1, 2) = e1;
				getSubMatrix(K, 2, 6, 3, 3)(2, 0) = n1;
				getSubMatrix(K, 2, 6, 3, 3)(2, 1) = n1;

				getSubMatrix(K, 3, 7, 3, 3) = getSubMatrix(K, 0, 4, 3, 3);
				getSubMatrix(K, 3, 7, 3, 3)(0, 1) = n2;
				getSubMatrix(K, 3, 7, 3, 3)(1, 0) = n2;
				getSubMatrix(K, 3, 7, 3, 3)(1, 2) = e1;
				getSubMatrix(K, 3, 7, 3, 3)(2, 1) = n1;
			}

			// Faces diagonals
			{
				double d1 = (-2.0 * a - c) / 36.0;
				double d2 = (a - 4.0 * c) / 36.0;
				double e1 = (b + c) / 12.0;
				double e2 = (b - c) / 24.0;
				double n1 = -e1;
				double n2 = -e2;

				getSubMatrix(K, 0, 5, 3, 3)(0, 0) = d1;
				getSubMatrix(K, 0, 5, 3, 3)(0, 1) = e2;
				getSubMatrix(K, 0, 5, 3, 3)(0, 2) = n1;
				getSubMatrix(K, 0, 5, 3, 3)(1, 0) = n2;
				getSubMatrix(K, 0, 5, 3, 3)(1, 1) = d2;
				getSubMatrix(K, 0, 5, 3, 3)(1, 2) = n2;
				getSubMatrix(K, 0, 5, 3, 3)(2, 0) = n1;
				getSubMatrix(K, 0, 5, 3, 3)(2, 1) = e2;
				getSubMatrix(K, 0, 5, 3, 3)(2, 2) = d1;

				getSubMatrix(K, 1, 4, 3, 3) = getSubMatrix(K, 0, 5, 3, 3);
				getSubMatrix(K, 1, 4, 3, 3)(0, 1) = n2;
				getSubMatrix(K, 1, 4, 3, 3)(0, 2) = e1;
				getSubMatrix(K, 1, 4, 3, 3)(1, 0) = e2;
				getSubMatrix(K, 1, 4, 3, 3)(2, 0) = e1;

				getSubMatrix(K, 2, 7, 3, 3) = getSubMatrix(K, 0, 5, 3, 3);
				getSubMatrix(K, 2, 7, 3, 3)(0, 2) = e1;
				getSubMatrix(K, 2, 7, 3, 3)(1, 2) = e2;
				getSubMatrix(K, 2, 7, 3, 3)(2, 0) = e1;
				getSubMatrix(K, 2, 7, 3, 3)(2, 1) = n2;

				getSubMatrix(K, 3, 6, 3, 3) = getSubMatrix(K, 0, 5, 3, 3);
				getSubMatrix(K, 3, 6, 3, 3)(0, 1) = n2;
				getSubMatrix(K, 3, 6, 3, 3)(1, 0) = e2;
				getSubMatrix(K, 3, 6, 3, 3)(1, 2) = e2;
				getSubMatrix(K, 3, 6, 3, 3)(2, 1) = n2;

				getSubMatrix(K, 1, 6, 3, 3)(0, 0) = d2;
				getSubMatrix(K, 1, 6, 3, 3)(0, 1) = e2;
				getSubMatrix(K, 1, 6, 3, 3)(0, 2) = e2;
				getSubMatrix(K, 1, 6, 3, 3)(1, 0) = n2;
				getSubMatrix(K, 1, 6, 3, 3)(1, 1) = d1;
				getSubMatrix(K, 1, 6, 3, 3)(1, 2) = n1;
				getSubMatrix(K, 1, 6, 3, 3)(2, 0) = n2;
				getSubMatrix(K, 1, 6, 3, 3)(2, 1) = n1;
				getSubMatrix(K, 1, 6, 3, 3)(2, 2) = d1;

				getSubMatrix(K, 2, 5, 3, 3) = getSubMatrix(K, 1, 6, 3, 3);
				getSubMatrix(K, 2, 5, 3, 3)(0, 1) = n2;
				getSubMatrix(K, 2, 5, 3, 3)(1, 0) = e2;
				getSubMatrix(K, 2, 5, 3, 3)(1, 2) = e1;
				getSubMatrix(K, 2, 5, 3, 3)(2, 1) = e1;

				getSubMatrix(K, 0, 7, 3, 3) = getSubMatrix(K, 1, 6, 3, 3);
				getSubMatrix(K, 0, 7, 3, 3)(0, 1) = n2;
				getSubMatrix(K, 0, 7, 3, 3)(0, 2) = n2;
				getSubMatrix(K, 0, 7, 3, 3)(1, 0) = e2;
				getSubMatrix(K, 0, 7, 3, 3)(2, 0) = e2;

				getSubMatrix(K, 3, 4, 3, 3) = getSubMatrix(K, 1, 6, 3, 3);
				getSubMatrix(K, 3, 4, 3, 3)(0, 2) = n2;
				getSubMatrix(K, 3, 4, 3, 3)(1, 2) = e1;
				getSubMatrix(K, 3, 4, 3, 3)(2, 0) = e2;
				getSubMatrix(K, 3, 4, 3, 3)(2, 1) = e1;

				getSubMatrix(K, 0, 2, 3, 3)(0, 0) = d1;
				getSubMatrix(K, 0, 2, 3, 3)(0, 1) = n1;
				getSubMatrix(K, 0, 2, 3, 3)(0, 2) = e2;
				getSubMatrix(K, 0, 2, 3, 3)(1, 0) = n1;
				getSubMatrix(K, 0, 2, 3, 3)(1, 1) = d1;
				getSubMatrix(K, 0, 2, 3, 3)(1, 2) = e2;
				getSubMatrix(K, 0, 2, 3, 3)(2, 0) = n2;
				getSubMatrix(K, 0, 2, 3, 3)(2, 1) = n2;
				getSubMatrix(K, 0, 2, 3, 3)(2, 2) = d2;

				getSubMatrix(K, 1, 3, 3, 3) = getSubMatrix(K, 0, 2, 3, 3);
				getSubMatrix(K, 1, 3, 3, 3)(0, 1) = e1;
				getSubMatrix(K, 1, 3, 3, 3)(0, 2) = n2;
				getSubMatrix(K, 1, 3, 3, 3)(1, 0) = e1;
				getSubMatrix(K, 1, 3, 3, 3)(2, 0) = e2;

				getSubMatrix(K, 4, 6, 3, 3) = getSubMatrix(K, 0, 2, 3, 3);
				getSubMatrix(K, 4, 6, 3, 3)(0, 2) = n2;
				getSubMatrix(K, 4, 6, 3, 3)(1, 2) = n2;
				getSubMatrix(K, 4, 6, 3, 3)(2, 0) = e2;
				getSubMatrix(K, 4, 6, 3, 3)(2, 1) = e2;

				getSubMatrix(K, 5, 7, 3, 3) = getSubMatrix(K, 0, 2, 3, 3);
				getSubMatrix(K, 5, 7, 3, 3)(0, 1) = e1;
				getSubMatrix(K, 5, 7, 3, 3)(1, 0) = e1;
				getSubMatrix(K, 5, 7, 3, 3)(1, 2) = n2;
				getSubMatrix(K, 5, 7, 3, 3)(2, 1) = e2;
			}

			// Cube diagonals
			{
				double d = (-a - 2.0 * c) / 36.0;
				double e = (b + c) / 24.0;
				double n = -e;

				getSubMatrix(K, 0, 6, 3, 3).setConstant(n);
				getSubMatrix(K, 0, 6, 3, 3).diagonal().setConstant(d);

				getSubMatrix(K, 1, 7, 3, 3).setConstant(e);
				getSubMatrix(K, 1, 7, 3, 3).diagonal().setConstant(d);
				getSubMatrix(K, 1, 7, 3, 3)(1, 2) = n;
				getSubMatrix(K, 1, 7, 3, 3)(2, 1) = n;

				getSubMatrix(K, 2, 4, 3, 3).setConstant(e);
				getSubMatrix(K, 2, 4, 3, 3).diagonal().setConstant(d);
				getSubMatrix(K, 2, 4, 3, 3)(0, 1) = n;
				getSubMatrix(K, 2, 4, 3, 3)(1, 0) = n;

				getSubMatrix(K, 3, 5, 3, 3).setConstant(e);
				getSubMatrix(K, 3, 5, 3, 3).diagonal().setConstant(d);
				getSubMatrix(K, 3, 5, 3, 3)(0, 2) = n;
				getSubMatrix(K, 3, 5, 3, 3)(2, 0) = n;
			}

			// Use symmetry to complete the triangular inferior part of K
			for (int row = 0; row < 24; ++row)
			{
				for (int col = row + 1; col < 24; ++col)
				{
					K(col, row) = K(row, col);
				}
			}
		}
		addSubMatrix(K, nodeIdsVectorForm, 3 , &m_expectedStiffnessMatrix);
	}

	void computeExpectedMassMatrix(std::vector<unsigned int> nodeIdsVectorForm)
	{
		using SurgSim::Math::addSubMatrix;

		Eigen::Matrix<double, 24, 24, Eigen::DontAlign> M;
		M.setZero();

		// "Physically-Based Simulation of Objects Represented by Surface Meshes"
		// Muller, Techner, Gross, CGI 2004
		// Given the shape functions they defined on Appendix A for the cube we are testing
		// We can derive the mass matrix M = \int_V rho N^T.N dV
		// N being a matrix (3x24) of shape functions
		// cf documentation

		double a = 1.0 / 27.0;
		double b = a / 2.0;
		double c = a / 4.0;
		double d = a / 8.0;

		M.diagonal().setConstant(a);

		M.block(0, 3, 21, 21).diagonal().setConstant(b);
		M.block(3*3, 3*4, 3, 3).diagonal().setConstant(c); // block (3, 4)

		M.block(0, 6, 18, 18).diagonal().setConstant(c);
		M.block(3*2, 3*4, 6, 6).diagonal().setConstant(d); // block (2, 4) and block (3, 5)

		M.block(0, 9, 15, 15).diagonal().setConstant(c);
		M.block(3*0, 3*3, 3, 3).diagonal().setConstant(b); // block (0, 3)
		M.block(3*4, 3*7, 3, 3).diagonal().setConstant(b); // block (4, 7)

		M.block(0, 12, 12, 12).diagonal().setConstant(b);

		M.block(0, 15, 9, 9).diagonal().setConstant(c);

		M.block(0, 18, 6, 6).diagonal().setConstant(d);

		M.block(0, 21, 3, 3).diagonal().setConstant(c);

		// Symmetry
		for (size_t row = 0; row < 24; ++row)
		{
			for (size_t col = row+1; col < 24; ++col)
			{
				M(col, row) = M(row, col);
			}
		}

		M *= m_rho;
		addSubMatrix(M, nodeIdsVectorForm, 3 , &m_expectedMassMatrix);
	}

	virtual void SetUp() override
	{
		using SurgSim::Math::getSubVector;
		using SurgSim::Math::getSubMatrix;
		using SurgSim::Math::addSubMatrix;

		m_restState.setNumDof(3, 8);
		Vector& x0 = m_restState.getPositions();

		// Cube is aligned with the axis (X,Y,Z), centered on (0.0, 0.0, 0.0), of size 1
		//       2*-----------*3
		//       /           /|
		//    6*-----------*7 |      ^ y
		//     |           |  |      |
		//     |  0        |  *1     *->x
		//     |           | /      /
		//    4*-----------*5       z
		getSubVector(x0, 0, 3) = Vector3d(-0.5,-0.5,-0.5);
		getSubVector(x0, 1, 3) = Vector3d( 0.5,-0.5,-0.5);
		getSubVector(x0, 2, 3) = Vector3d(-0.5, 0.5,-0.5);
		getSubVector(x0, 3, 3) = Vector3d( 0.5, 0.5,-0.5);
		getSubVector(x0, 4, 3) = Vector3d(-0.5,-0.5, 0.5);
		getSubVector(x0, 5, 3) = Vector3d( 0.5,-0.5, 0.5);
		getSubVector(x0, 6, 3) = Vector3d(-0.5, 0.5, 0.5);
		getSubVector(x0, 7, 3) = Vector3d( 0.5, 0.5, 0.5);

		// Ordering following the description in
		// "Physically-Based Simulation of Objects Represented by Surface Meshes"
		// Muller, Techner, Gross, CGI 2004
		std::array<unsigned int, 8> tmpNodeIds = {{0, 1, 3, 2, 4, 5, 7, 6}};
		m_nodeIds = tmpNodeIds;
		std::vector<unsigned int> nodeIdsVectorForm; // Useful for assembly helper function
		for (size_t i = 0; i < 8; ++i)
		{
			nodeIdsVectorForm.push_back(m_nodeIds[i]);
		}

		// Build the expected x0 vector
		for (size_t i = 0; i < 8; i++)
		{
			getSubVector(m_expectedX0, i, 3) = getSubVector(x0, m_nodeIds[i], 3);
		}

		// The cube has a size of 1, so its volume is 1m^3
		m_expectedVolume = 1.0;

		m_rho = 1000.0;
		m_E = 1e6;
		m_nu = 0.45;

		m_expectedMassMatrix.resize(3*8, 3*8);
		m_expectedMassMatrix.setZero();
		m_expectedDampingMatrix.resize(3*8, 3*8);
		m_expectedDampingMatrix.setZero();
		m_expectedStiffnessMatrix.resize(3*8, 3*8);
		m_expectedStiffnessMatrix.setZero();
		m_vectorOnes.resize(3*8);
		m_vectorOnes.setConstant(1.0);

		computeExpectedMassMatrix(nodeIdsVectorForm);
		m_expectedDampingMatrix.setZero();
		computeExpectedStiffnessMatrix(nodeIdsVectorForm);
	}
};

extern void testSize(const Vector& v, int expectedSize);
extern void testSize(const Matrix& m, int expectedRows, int expectedCols);

TEST_F(FemElement3DCubeTests, ConstructorTest)
{
	ASSERT_NO_THROW({MockFemElement3DCube cube(m_nodeIds, m_restState);});
	ASSERT_NO_THROW({MockFemElement3DCube* cube = new MockFemElement3DCube(m_nodeIds, m_restState); delete cube;});
	ASSERT_NO_THROW({std::shared_ptr<MockFemElement3DCube> cube =
		std::make_shared<MockFemElement3DCube>(m_nodeIds, m_restState);});
}

TEST_F(FemElement3DCubeTests, NodeIdsTest)
{
	FemElement3DCube cube(m_nodeIds, m_restState);
	EXPECT_EQ(8u, cube.getNumNodes());
	EXPECT_EQ(8u, cube.getNodeIds().size());
	for (int i = 0; i < 8; i++)
	{
		EXPECT_EQ(m_nodeIds[i], cube.getNodeId(i));
		EXPECT_EQ(m_nodeIds[i], cube.getNodeIds()[i]);
	}
}

TEST_F(FemElement3DCubeTests, VolumeTest)
{
	MockFemElement3DCube cube(m_nodeIds, m_restState);
	EXPECT_NEAR(cube.getRestVolume(), m_expectedVolume, 1e-10);
	EXPECT_NEAR(cube.getVolume(m_restState), m_expectedVolume, 1e-10);
}

TEST_F(FemElement3DCubeTests, ShapeFunctionsTest)
{
	using SurgSim::Math::getSubVector;

	MockFemElement3DCube cube(m_nodeIds, m_restState);

	EXPECT_TRUE(cube.getInitialPosition().isApprox(m_expectedX0)) <<
		"x0 = " << cube.getInitialPosition().transpose() << std::endl << "x0 expected = " << m_expectedX0.transpose();

	// We should have by construction:
	// { N0(p0) = 1    N1(p0)=N2(p0)=N3(p0)=0
	// { N1(p1) = 1    N1(p1)=N2(p1)=N3(p1)=0
	// { N2(p2) = 1    N1(p2)=N2(p2)=N3(p2)=0
	// { N3(p3) = 1    N1(p3)=N2(p3)=N3(p3)=0
	Vector3d p[8];
	for (size_t nodeId = 0; nodeId < 8; ++nodeId)
	{
		// either retrieving the points from expectedX0  with indices 0..7
		// OR     retrieving the points from m_restState with indices m_nodeIds[0]..m_nodeIds[7]
		p[nodeId] = getSubVector(m_expectedX0, nodeId, 3);
	}
	double Ni_p0[8], Ni_p1[8], Ni_p2[8], Ni_p3[8], Ni_p4[8], Ni_p5[8], Ni_p6[8], Ni_p7[8];
	for (int i = 0; i < 8; i++)
	{
		Ni_p0[i] = cube.evaluateN(i, -1.0, -1.0, -1.0);
		Ni_p1[i] = cube.evaluateN(i, +1.0, -1.0, -1.0);
		Ni_p2[i] = cube.evaluateN(i, +1.0, +1.0, -1.0);
		Ni_p3[i] = cube.evaluateN(i, -1.0, +1.0, -1.0);
		Ni_p4[i] = cube.evaluateN(i, -1.0, -1.0, +1.0);
		Ni_p5[i] = cube.evaluateN(i, +1.0, -1.0, +1.0);
		Ni_p6[i] = cube.evaluateN(i, +1.0, +1.0, +1.0);
		Ni_p7[i] = cube.evaluateN(i, -1.0, +1.0, +1.0);
	}
	EXPECT_NEAR(Ni_p0[0], 1.0, 1e-12);
	for (size_t i = 0; i < 8; ++i)
	{
		if (i == 0) continue;
		EXPECT_NEAR(Ni_p0[i], 0.0, 1e-12);
	}

	EXPECT_NEAR(Ni_p1[1], 1.0, 1e-12);
	for (size_t i = 0; i < 8; ++i)
	{
		if (i == 1) continue;
		EXPECT_NEAR(Ni_p1[i], 0.0, 1e-12);
	}

	EXPECT_NEAR(Ni_p2[2], 1.0, 1e-12);
	for (size_t i = 0; i < 8; ++i)
	{
		if (i == 2) continue;
		EXPECT_NEAR(Ni_p2[i], 0.0, 1e-12);
	}

	EXPECT_NEAR(Ni_p3[3], 1.0, 1e-12);
	for (size_t i = 0; i < 8; ++i)
	{
		if (i == 3) continue;
		EXPECT_NEAR(Ni_p3[i], 0.0, 1e-12);
	}

	EXPECT_NEAR(Ni_p4[4], 1.0, 1e-12);
	for (size_t i = 0; i < 8; ++i)
	{
		if (i == 4) continue;
		EXPECT_NEAR(Ni_p4[i], 0.0, 1e-12);
	}

	EXPECT_NEAR(Ni_p5[5], 1.0, 1e-12);
	for (size_t i = 0; i < 8; ++i)
	{
		if (i == 5) continue;
		EXPECT_NEAR(Ni_p5[i], 0.0, 1e-12);
	}

	EXPECT_NEAR(Ni_p6[6], 1.0, 1e-12);
	for (size_t i = 0; i < 8; ++i)
	{
		if (i == 6) continue;
		EXPECT_NEAR(Ni_p6[i], 0.0, 1e-12);
	}

	EXPECT_NEAR(Ni_p7[7], 1.0, 1e-12);
	for (size_t i = 0; i < 8; ++i)
	{
		if (i == 7) continue;
		EXPECT_NEAR(Ni_p7[i], 0.0, 1e-12);
	}

	// We should have the relation sum(Ni(x,y,z) = 1) for all points in the volume
	// We verify that relation by sampling the tetrahedron volume
	for (double epsilon = -1.0; epsilon <= 1.0; epsilon+=0.1)
	{
		for (double neta = -1.0; neta <= 1.0; neta+=0.1)
		{
			for (double mu = -1.0; mu <= 1.0; mu+=0.1)
			{
				double Ni_p[8];
				double sum = 0.0;
				for (int i = 0; i < 8; i++)
				{
					Ni_p[i] = cube.evaluateN(i, epsilon, neta, mu);
					sum += Ni_p[i];
				}
				EXPECT_NEAR(sum, 1.0, 1e-10) <<
					" for epsilon = " << epsilon << ", neta = " << neta << ", mu = " << mu << std::endl <<
					" N0(epsilon,neta,mu) = " << Ni_p[0] << " N1(epsilon,neta,mu) = " << Ni_p[1] <<
					" N2(epsilon,neta,mu) = " << Ni_p[2] << " N3(epsilon,neta,mu) = " << Ni_p[3] <<
					" N4(epsilon,neta,mu) = " << Ni_p[4] << " N5(epsilon,neta,mu) = " << Ni_p[5] <<
					" N6(epsilon,neta,mu) = " << Ni_p[6] << " N7(epsilon,neta,mu) = " << Ni_p[7];
			}
		}
	}
}

TEST_F(FemElement3DCubeTests, ForceAndMatricesTest)
{
	using SurgSim::Math::getSubVector;

	MockFemElement3DCube cube(m_nodeIds, m_restState);

	// Test the various mode of failure related to the physical parameters
	// This has been already tested in FemElementTests, but this is to make sure this method is called properly
	// So the same behavior should be expected
	{
		// Mass density not set
		ASSERT_ANY_THROW(cube.initialize(m_restState));

		// Poisson Ratio not set
		cube.setMassDensity(-1234.56);
		ASSERT_ANY_THROW(cube.initialize(m_restState));

		// Young modulus not set
		cube.setPoissonRatio(0.55);
		ASSERT_ANY_THROW(cube.initialize(m_restState));

		// Invalid mass density
		cube.setYoungModulus(-4321.33);
		ASSERT_ANY_THROW(cube.initialize(m_restState));

		// Invalid Poisson ratio
		cube.setMassDensity(m_rho);
		ASSERT_ANY_THROW(cube.initialize(m_restState));

		// Invalid Young modulus
		cube.setPoissonRatio(m_nu);
		ASSERT_ANY_THROW(cube.initialize(m_restState));

		cube.setYoungModulus(m_E);
		ASSERT_NO_THROW(cube.initialize(m_restState));
	}

	SurgSim::Math::Vector forceVector(3*8);
	SurgSim::Math::Matrix massMatrix(3*8, 3*8);
	SurgSim::Math::Matrix dampingMatrix(3*8, 3*8);
	SurgSim::Math::Matrix stiffnessMatrix(3*8, 3*8);

	forceVector.setZero();
	massMatrix.setZero();
	dampingMatrix.setZero();
	stiffnessMatrix.setZero();

	// No force should be produced when in rest state (x = x0) => F = K.(x-x0) = 0
	cube.addForce(m_restState, &forceVector);
	EXPECT_TRUE(forceVector.isZero());

	cube.addMass(m_restState, &massMatrix);
	EXPECT_TRUE(massMatrix.isApprox(m_expectedMassMatrix)) <<
		"Expected mass matrix :" << std::endl << m_expectedMassMatrix  << std::endl << std::endl <<
		"Mass matrix :"  << std::endl << massMatrix << std::endl << std::endl <<
		"Error on the mass matrix is "  << std::endl << m_expectedMassMatrix - massMatrix << std::endl;

	cube.addDamping(m_restState, &dampingMatrix);
	EXPECT_TRUE(dampingMatrix.isApprox(m_expectedDampingMatrix));

	cube.addStiffness(m_restState, &stiffnessMatrix);
	EXPECT_TRUE(stiffnessMatrix.isApprox(m_expectedStiffnessMatrix)) <<
		"Error on the stiffness matrix is " << std::endl << m_expectedStiffnessMatrix - stiffnessMatrix << std::endl;

	forceVector.setZero();
	massMatrix.setZero();
	dampingMatrix.setZero();
	stiffnessMatrix.setZero();

	cube.addFMDK(m_restState, &forceVector, &massMatrix, &dampingMatrix, &stiffnessMatrix);
	EXPECT_TRUE(forceVector.isZero());
	EXPECT_TRUE(massMatrix.isApprox(m_expectedMassMatrix));
	EXPECT_TRUE(dampingMatrix.isApprox(m_expectedDampingMatrix));
	EXPECT_TRUE(stiffnessMatrix.isApprox(m_expectedStiffnessMatrix));

	// Test addMatVec API with Mass component only
	forceVector.setZero();
	cube.addMatVec(m_restState, 1.0, 0.0, 0.0, m_vectorOnes, &forceVector);
	for (int rowId = 0; rowId < 3 * 8; rowId++)
	{
		EXPECT_NEAR(m_expectedMassMatrix.row(rowId).sum(), forceVector[rowId], epsilon);
	}
	// Test addMatVec API with Damping component only
	forceVector.setZero();
	cube.addMatVec(m_restState, 0.0, 1.0, 0.0, m_vectorOnes, &forceVector);
	for (int rowId = 0; rowId < 3 * 8; rowId++)
	{
		EXPECT_NEAR(m_expectedDampingMatrix.row(rowId).sum(), forceVector[rowId], epsilon);
	}
	// Test addMatVec API with Stiffness component only
	forceVector.setZero();
	cube.addMatVec(m_restState, 0.0, 0.0, 1.0, m_vectorOnes, &forceVector);
	for (int rowId = 0; rowId < 3 * 8; rowId++)
	{
		EXPECT_NEAR(m_expectedStiffnessMatrix.row(rowId).sum(), forceVector[rowId], epsilon);
	}
	// Test addMatVec API with mix Mass/Damping/Stiffness components
	forceVector.setZero();
	cube.addMatVec(m_restState, 1.0, 2.0, 3.0, m_vectorOnes, &forceVector);
	for (int rowId = 0; rowId < 3 * 8; rowId++)
	{
		double expectedCoef = 1.0 * m_expectedMassMatrix.row(rowId).sum() +
			2.0 * m_expectedDampingMatrix.row(rowId).sum() +
			3.0 * m_expectedStiffnessMatrix.row(rowId).sum();
		EXPECT_NEAR(expectedCoef, forceVector[rowId], epsilon);
	}
}
