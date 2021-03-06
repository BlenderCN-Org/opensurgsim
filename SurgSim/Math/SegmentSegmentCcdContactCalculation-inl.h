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

#ifndef SURGSIM_MATH_SEGMENTSEGMENTCCDCONTACTCALCULATION_INL_H
#define SURGSIM_MATH_SEGMENTSEGMENTCCDCONTACTCALCULATION_INL_H

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "SurgSim/Math/CubicSolver.h"
#include "SurgSim/Math/Scalar.h"

namespace SurgSim
{
namespace Math
{

/// Check if 2 segments intersect at a given time of their motion
/// \tparam T		Accuracy of the calculation, can usually be inferred.
/// \tparam MOpt	Eigen Matrix options, can usually be inferred.
/// \param time The time of coplanarity of the 4 points (A(t), B(t), C(t), D(t) are expected to be coplanar)
/// \param A, B The 1st segment motion (each point's motion is from first to second)
/// \param C, D The 2nd segment motion (each point's motion is from first to second)
/// \param[out] barycentricCoordinates The barycentric coordinates of the intersection in AB(t) and CD(t)
/// i.e. P(t) = A + barycentricCoordinates[0].AB(t) = C + barycentricCoordinates[1].CD
/// \return True if AB(t) is intersecting CD(t), False otherwise
template <class T, int MOpt>
bool areSegmentsIntersecting(
	T time,
	const std::pair<Eigen::Matrix<T, 3, 1, MOpt>, Eigen::Matrix<T, 3, 1, MOpt>>& A,
	const std::pair<Eigen::Matrix<T, 3, 1, MOpt>, Eigen::Matrix<T, 3, 1, MOpt>>& B,
	const std::pair<Eigen::Matrix<T, 3, 1, MOpt>, Eigen::Matrix<T, 3, 1, MOpt>>& C,
	const std::pair<Eigen::Matrix<T, 3, 1, MOpt>, Eigen::Matrix<T, 3, 1, MOpt>>& D,
	Eigen::Matrix<T, 2, 1, MOpt>* barycentricCoordinates)
{
	Eigen::Matrix<T, 3, 1, MOpt> At = interpolate(A.first, A.second, time);
	Eigen::Matrix<T, 3, 1, MOpt> Bt = interpolate(B.first, B.second, time);
	Eigen::Matrix<T, 3, 1, MOpt> Ct = interpolate(C.first, C.second, time);
	Eigen::Matrix<T, 3, 1, MOpt> Dt = interpolate(D.first, D.second, time);

	// P = A + alpha.AB and P = C + beta.CD
	// A + alpha.AB = C + beta.CD
	// (AB -CD).(alpha) = (AC) which is a 3x2 linear system A.x = b
	//          (beta )
	// Let's solve it using (A^t.A)^-1.(A^t.A) = Id2x2
	// (A^t.A)^-1.A^t.(A.x) = (A^t.A)^-1.A^t.b
	// x = (A^t.A)^-1.A^t.b

	Eigen::Matrix<T, 3, 2> matrixA;
	matrixA.col(0) = Bt - At;
	matrixA.col(1) = -(Dt - Ct);

	Eigen::Matrix<T, 3, 1, MOpt> b = Ct - At;
	Eigen::Matrix<T, 2, 2> inv;
	bool invertible;
	(matrixA.transpose() * matrixA).computeInverseWithCheck(inv, invertible);
	if (!invertible)
	{
		return false;
	}

	*barycentricCoordinates = inv * matrixA.transpose() * b;

	for (int i = 0; i < 2; i++)
	{
		if (std::abs((*barycentricCoordinates)[i]) < Math::Geometry::ScalarEpsilon)
		{
			(*barycentricCoordinates)[i] = 0.0;
		}
		if (std::abs(1.0 - (*barycentricCoordinates)[i]) < Math::Geometry::ScalarEpsilon)
		{
			(*barycentricCoordinates)[i] = 1.0;
		}
	}

	return
		(*barycentricCoordinates)[0] >= 0.0 && (*barycentricCoordinates)[0] <= 1.0 &&
		(*barycentricCoordinates)[1] >= 0.0 && (*barycentricCoordinates)[1] <= 1.0;
}

/// Continuous collision detection between two segments AB and CD
/// \tparam T		Accuracy of the calculation, can usually be inferred.
/// \tparam MOpt	Eigen Matrix options, can usually be inferred.
/// \param A, B The 1st segment motion (each point's motion is from first to second)
/// \param C, D The 2nd segment motion (each point's motion is from first to second)
/// \param[out] timeOfImpact The time of impact within [0..1] if an collision is found
/// \param[out] s0p1Factor, s1p1Factor The barycentric coordinate of the contact point on the 2 segments
/// i.e. ContactPoint(timeOfImpact) = A(timeOfImpact) + s0p1Factor.AB(timeOfImpact)
/// i.e. ContactPoint(timeOfImpact) = C(timeOfImpact) + s1p1Factor.CD(timeOfImpact)
/// \return True if the given segment/segment motions intersect, False otherwise
/// \note Simple cubic-solver https://graphics.stanford.edu/courses/cs468-02-winter/Papers/Collisions_vetements.pdf
/// \note Optimized method http://www.robotics.sei.ecnu.edu.cn/CCDLY/GMOD15.pdf
/// \note Optimized method https://www.cs.ubc.ca/~rbridson/docs/brochu-siggraph2012-ccd.pdf
template <class T, int MOpt> inline
bool calculateCcdContactSegmentSegment(
	const std::pair<Eigen::Matrix<T, 3, 1, MOpt>, Eigen::Matrix<T, 3, 1, MOpt>>& A,
	const std::pair<Eigen::Matrix<T, 3, 1, MOpt>, Eigen::Matrix<T, 3, 1, MOpt>>& B,
	const std::pair<Eigen::Matrix<T, 3, 1, MOpt>, Eigen::Matrix<T, 3, 1, MOpt>>& C,
	const std::pair<Eigen::Matrix<T, 3, 1, MOpt>, Eigen::Matrix<T, 3, 1, MOpt>>& D,
	T* timeOfImpact, T* s0p1Factor, T* s1p1Factor)
{
	std::array<T, 3> roots;
	int numberOfRoots = timesOfCoplanarityInRange01(A, B, C, D, &roots);

	// The roots are all in [0..1] and ordered ascendingly
	for (int rootId = 0; rootId < numberOfRoots; ++rootId)
	{
		Eigen::Matrix<T, 2, 1, MOpt> barycentricCoordinates;
		if (areSegmentsIntersecting(roots[rootId], A, B, C, D, &barycentricCoordinates))
		{
			// The segments AB and CD are coplanar at time t, and they intersect
			*timeOfImpact = roots[rootId];
			*s0p1Factor = barycentricCoordinates[0];
			*s1p1Factor = barycentricCoordinates[1];

			// None of these assertion should be necessary, but just to double check
			SURGSIM_ASSERT(*timeOfImpact >= 0.0 && *timeOfImpact <= 1.0);
			SURGSIM_ASSERT(*s0p1Factor >= 0.0 && *s0p1Factor <= 1.0);
			SURGSIM_ASSERT(*s1p1Factor >= 0.0 && *s1p1Factor <= 1.0);

			return true;
		}
	}

	return false;
}

}; // namespace Math
}; // namespace SurgSim

#endif // SURGSIM_MATH_SEGMENTSEGMENTCCDCONTACTCALCULATION_INL_H
