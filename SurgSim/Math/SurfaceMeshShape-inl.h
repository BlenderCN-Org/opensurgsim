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
//

#ifndef SURGSIM_MATH_SURFACEMESHSHAPE_INL_H
#define SURGSIM_MATH_SURFACEMESHSHAPE_INL_H

namespace SurgSim
{
namespace Math
{

template <class VertexData, class EdgeData, class TriangleData>
SurfaceMeshShape::SurfaceMeshShape(
	const SurgSim::DataStructures::TriangleMeshBase<VertexData, EdgeData, TriangleData>& mesh,
	double thickness) : m_thickness(thickness)
{
	SURGSIM_ASSERT(mesh.isValid()) << "Invalid mesh";

	m_mesh = std::make_shared<SurgSim::DataStructures::TriangleMesh>(mesh);

	computeVolumeIntegrals();
}

}; // namespace Math
}; // namespace SurgSim

#endif // SURGSIM_MATH_SURFACEMESHSHAPE_INL_H
