// This file is a part of the OpenSurgSim project.
// Copyright 2012-2013, SimQuest Solutions Inc.
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

/// \file
/// Tests for the Mesh class.

#include "gtest/gtest.h"

#include "SurgSim/DataStructures/Vertex.h"
#include "SurgSim/DataStructures/UnitTests/MockObjects.h"
#include "SurgSim/Math/Vector.h"

using SurgSim::DataStructures::Vertex;
using SurgSim::Math::Vector3d;

/// Vertex with ID and normal data
typedef SurgSim::DataStructures::Vertex<MockVertexData> MockVertex;
/// Vertex with no data
typedef SurgSim::DataStructures::Vertex<void> MockVertexNoData;

TEST(MeshVertexTest, InitTest)
{
	Vector3d position(0.0, 0.0, 0.0);
	MockVertexData data(0, Vector3d(1.0, 0.0, 0.0));
	ASSERT_NO_THROW({MockVertex vertex(position, data);});

	/// Check that we can also create a vertex with no data
	ASSERT_NO_THROW({MockVertexNoData vertex(position);});
}

TEST(MeshVertexTest, VertexTest)
{
	Vector3d position(1.0, 2.0, 3.0);
	MockVertexData data(2, Vector3d(1.0, 0.0, 0.0));
	MockVertex vertex(position, data);

	EXPECT_EQ(position, vertex.position);
	EXPECT_EQ(data, vertex.data);

	{
		const MockVertexData& data = vertex.data;
		EXPECT_EQ(2u, data.getId());
		EXPECT_EQ(Vector3d(1.0, 0.0, 0.0), data.getNormal());
	}
}

TEST(MeshVertexTest, VertexNoDataTest)
{
	Vector3d position(1.0, 2.0, 3.0);
	MockVertexNoData vertex(position);

	EXPECT_EQ(position, vertex.position);
}