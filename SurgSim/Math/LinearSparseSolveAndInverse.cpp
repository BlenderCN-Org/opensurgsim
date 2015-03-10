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

#include "SurgSim/Math/LinearSparseSolveAndInverse.h"

namespace SurgSim
{

namespace Math
{

void LinearSparseSolveAndInverseLU::operator()(const SparseMatrix& A, const Vector& b, Vector* x, Matrix* Ainv)
{
	SURGSIM_ASSERT(A.cols() == A.rows()) << "Cannot inverse a non square matrix";

	std::cout << "In Solver:" << std::endl;
	std::cout << A << std::endl;

	if (x != nullptr || Ainv != nullptr)
	{
		Eigen::SparseLU<SparseMatrix> lu;
		lu.compute(A);
		if (x != nullptr)
		{
			(*x) = lu.solve(b);
		}
		if (Ainv != nullptr)
		{
			if ((Ainv->rows() != A.rows()) || (Ainv->cols() != A.cols()))
			{
				Ainv->resize(A.rows(), A.cols());
			}
			Ainv->setIdentity();
			(*Ainv) = lu.solve((*Ainv));
		}
	}
}

}; // namespace Math

}; // namespace SurgSim
