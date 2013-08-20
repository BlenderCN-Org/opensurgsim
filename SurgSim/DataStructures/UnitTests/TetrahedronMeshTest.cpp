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
/// Tests for the Tetrahedron class.

#include "gtest/gtest.h"

#include "SurgSim/DataStructures/MeshVertex.h"
#include "SurgSim/DataStructures/MeshElement.h"
#include "SurgSim/DataStructures/UnitTests/MockObjects.h"

#include <random>

using SurgSim::DataStructures::TetrahedronMesh;
using SurgSim::Math::Vector3d;

class TetrahedronMeshTest : public ::testing::Test
{
public:
	void SetUp()
	{
		// Set to true to print the test positions.
		bool printPositions = true;
		// Set to true to print the test normals.
		bool printNormals = true;
		// Set to true to print the test triangles.
		bool printTriangles = true;
		// Set to true to print the test tetrahedrons.
		bool printTetrahedrons = true;
		// Set to true to print the test edges.
		bool printEdges = true;
		// Set the number of test vertices
		unsigned int numVertices = 10;
		// Set the number of test triangles
		unsigned int numTriangles = 20;
		// Set the number of test tetrahedrons
		unsigned int numTetrahedrons = 15;

		std::default_random_engine generator;
		std::uniform_real_distribution<double> positionDistribution(-10.0, 10.0);
		std::uniform_real_distribution<double> normalDistribution(-1.0, 1.0);
		std::uniform_int_distribution<unsigned int> vertexIdDistribution(0, numVertices);

		if (printPositions)
		{
			printf("Test Vertex Positions:\n");
		}

		/// Generate random positions for each vertex
		for (unsigned int i = 0; i < numVertices; ++i)
		{
			Vector3d position(positionDistribution(generator), positionDistribution(generator),
				positionDistribution(generator));
			testPositions.push_back(position);

			if (printPositions)
			{
				printf("\t%d: (%g, %g, %g)\n", i, position.x(), position.y(), position.z());
			}
		}

		if (printNormals)
		{
			printf("Test Vertex Normals:\n");
		}

		/// Generate random normals for each vertex
		for (unsigned int i = 0; i < numVertices; ++i)
		{
			Vector3d normal(normalDistribution(generator), normalDistribution(generator),
				normalDistribution(generator));
			normal.normalize();
			testNormals.push_back(normal);

			if (printNormals)
			{
				printf("\t%d: (%g, %g, %g)\n", i, normal.x(), normal.y(), normal.z());
			}
		}

		if (printTetrahedrons)
		{
			printf("Test Tetrahedrons:\n");
		}

		/// Generate random vertex IDs within [0, numVertices) in quadruplets for mesh tetrahedrons
		for (unsigned int i = 0; i < numTetrahedrons; ++i)
		{
			std::array<unsigned int, 4> tetrahedronVertices = {{ vertexIdDistribution(generator),
				vertexIdDistribution(generator), vertexIdDistribution(generator), vertexIdDistribution(generator) }};
			testTetrahedronsVerticesId.push_back(tetrahedronVertices);

			/// Create 6 vertex ID pairs for each tetrahedron edge (not worrying about duplicates for these tests)
			std::array<unsigned int, 6> tetrahedronEdges;
			int edgeIDs[6][2] = { {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3} };
			for (int j = 0; j < 6; ++j)
			{
				std::array<unsigned int, 2> edgeVertices = {{ tetrahedronVertices[edgeIDs[j][0]], tetrahedronVertices[edgeIDs[j][1]] }};
				testEdgesVerticesId.push_back(edgeVertices);

				tetrahedronEdges[j] = testEdgesVerticesId.size() - 1;
			}
			testTetrahedronsEdgesId.push_back(tetrahedronEdges);

			/// Create 4 vertex ID pairs for each tetrahedron triangle (not worrying about duplicates for these tests)
			int vertexIDs[4][3] = { {0, 1, 2}, {0, 1, 3}, {0, 2, 3}, {1, 2, 3} };
			int tetTriangleEdgeIds[4][3] = { {0, 1, 3}, {0, 2, 4}, {1, 2, 5}, {3, 4, 5} };
			std::array<unsigned int, 4> tetrahedronTriangles;
			for (int j = 0; j < 4; ++j)
			{
				std::array<unsigned int, 3> triangleVertices = {{ tetrahedronVertices[vertexIDs[j][0]], tetrahedronVertices[vertexIDs[j][1]], tetrahedronVertices[vertexIDs[j][2]] }};
				testTrianglesVerticesId.push_back(triangleVertices);

				tetrahedronTriangles[j] = testTrianglesVerticesId.size() - 1;

				std::array<unsigned int, 3> triangleEdges;
				for (int k = 0; k < 3; k++)
				{
					triangleEdges[k] = tetrahedronEdges[tetTriangleEdgeIds[j][k]];
				}
				testTrianglesEdgesId.push_back(triangleEdges);
			}
			testTetrahedronsTrianglesId.push_back(tetrahedronTriangles);

			if (printTetrahedrons)
			{
				printf("\t%d: Vertices (%d, %d, %d, %d), Edges (%d, %d, %d, %d, %d, %d), Triangles (%d %d %d %d)\n", i,
					tetrahedronVertices[0], tetrahedronVertices[1], tetrahedronVertices[2], tetrahedronVertices[3],
					tetrahedronEdges[0], tetrahedronEdges[1], tetrahedronEdges[2], tetrahedronEdges[3], tetrahedronEdges[4], tetrahedronEdges[5],
					tetrahedronTriangles[0], tetrahedronTriangles[1], tetrahedronTriangles[2], tetrahedronTriangles[3]
					);
			}
		}

		if (printTriangles)
		{
			printf("Test Triangles:\n");

			for (unsigned int i = 0; i < testTrianglesVerticesId.size(); ++i)
			{
				const std::array<unsigned int, 3>& triangleVertices = testTrianglesVerticesId[i];
				const std::array<unsigned int, 3>& triangleEdges = testTrianglesEdgesId[i];
				printf("\t%d: Vertices (%d, %d, %d) - Edges (%d %d %d)\n", i,
					triangleVertices[0], triangleVertices[1], triangleVertices[2],
					triangleEdges[0], triangleEdges[1], triangleEdges[2]);
			}
		}

		if (printEdges)
		{
			printf("Test Edges:\n");

			for (unsigned int i = 0; i < testEdgesVerticesId.size(); ++i)
			{
				const std::array<unsigned int, 2>& edgeVertices = testEdgesVerticesId[i];
				printf("\t%d: (%d, %d)\n", i, edgeVertices[0], edgeVertices[1]);
			}
		}
	}

	void TearDown()
	{

	}

	/// Positions of test vertices
	std::vector<Vector3d> testPositions;
	/// Normals of test vertices
	std::vector<Vector3d> testNormals;

	/// Vertices Id for all edges
	std::vector<std::array<unsigned int, 2>> testEdgesVerticesId;

	/// Vertices Id for all triangles
	std::vector<std::array<unsigned int, 3>> testTrianglesVerticesId;
	/// Edges Id for all triangles
	std::vector<std::array<unsigned int, 3>> testTrianglesEdgesId;

	/// Vertices Id for all tetrahedrons
	std::vector<std::array<unsigned int, 4>> testTetrahedronsVerticesId;
	/// Edges Id for all tetrahedrons
	std::vector<std::array<unsigned int, 6>> testTetrahedronsEdgesId;
	/// Triangles Id for all tetrahedrons
	std::vector<std::array<unsigned int, 4>> testTetrahedronsTrianglesId;
};


TEST_F(TetrahedronMeshTest, InitTest)
{
	ASSERT_NO_THROW({MockTetrahedronMesh mesh;});

	/// Make sure we can create triangle meshes with each possible combination of void data.
	/// One void entry
	typedef TetrahedronMesh<void, MockEdgeData, MockTriangleData, MockTetrahedronData> TetrahedronMeshNoVertexData;
	typedef TetrahedronMesh<MockVertexData, void, MockTriangleData, MockTetrahedronData> TetrahedronMeshNoEdgeData;
	typedef TetrahedronMesh<MockVertexData, MockEdgeData, void, MockTetrahedronData> TetrahedronMeshNoTriangleData;
	typedef TetrahedronMesh<MockVertexData, MockEdgeData, MockTriangleData, void> TetrahedronMeshNoTetrahedronData;
	ASSERT_NO_THROW({TetrahedronMeshNoVertexData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshNoEdgeData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshNoTriangleData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshNoTetrahedronData mesh;});
	
	/// Two void entries
	typedef TetrahedronMesh<void, void, MockTriangleData, MockTetrahedronData> TetrahedronMeshNoVertexOrEdgeData;
	typedef TetrahedronMesh<void, MockEdgeData, void, MockTetrahedronData> TetrahedronMeshNoVertexOrTriangleData;
	typedef TetrahedronMesh<void, MockEdgeData, MockTriangleData, void> TetrahedronMeshNoVertexOrTetrahedronData;
	typedef TetrahedronMesh<MockVertexData, void, void, MockTetrahedronData> TetrahedronMeshNoEdgeOrTriangleData;
	typedef TetrahedronMesh<MockVertexData, void, MockTriangleData, void> TetrahedronMeshNoEdgeOrTetrahedronData;
	typedef TetrahedronMesh<MockVertexData, MockEdgeData, void, void> TetrahedronMeshNoTriangleOrTetrahedronData;
	ASSERT_NO_THROW({TetrahedronMeshNoVertexOrEdgeData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshNoVertexOrTriangleData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshNoVertexOrTetrahedronData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshNoEdgeOrTriangleData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshNoEdgeOrTetrahedronData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshNoTriangleOrTetrahedronData mesh;});

	/// Three void entries
	typedef TetrahedronMesh<void, void, void, MockTetrahedronData> TetrahedronMeshOnlyTetrahedronData;
	typedef TetrahedronMesh<void, void, MockTriangleData, void> TetrahedronMeshOnlyTriangleData;
	typedef TetrahedronMesh<void, MockEdgeData, void, void> TetrahedronMeshOnlyEdgeData;
	typedef TetrahedronMesh<MockVertexData, void, void, void> TetrahedronMeshOnlyVertexData;
	ASSERT_NO_THROW({TetrahedronMeshOnlyTetrahedronData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshOnlyTriangleData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshOnlyEdgeData mesh;});
	ASSERT_NO_THROW({TetrahedronMeshOnlyVertexData mesh;});

	/// Four void entries
	typedef TetrahedronMesh<void, void, void, void> TetrahedronMeshNoData;
	ASSERT_NO_THROW({TetrahedronMeshNoData mesh;});
}

TEST_F(TetrahedronMeshTest, CreateVerticesTest)
{
	MockTetrahedronMesh mesh;

	EXPECT_EQ(0u, mesh.getNumVertices());
	EXPECT_EQ(0u, mesh.getVertices().size());
	EXPECT_EQ(0u, mesh.getNumEdges());
	EXPECT_EQ(0u, mesh.getEdges().size());
	EXPECT_EQ(0u, mesh.getNumTriangles());
	EXPECT_EQ(0u, mesh.getTriangles().size());
	EXPECT_EQ(0u, mesh.getNumTetrahedrons());
	EXPECT_EQ(0u, mesh.getTetrahedrons().size());
	
	EXPECT_EQ(0, mesh.getNumUpdates());

	/// Create the test vertices
	for (unsigned int i = 0; i < testPositions.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createVertex(testPositions[i], testNormals[i]));
		EXPECT_EQ(i + 1, mesh.getNumVertices());

		const std::vector<MockTriangleMesh::Vertex>& vertices = mesh.getVertices();
		EXPECT_EQ(i + 1, vertices.size());

		/// Make sure each vertex is set properly
		for (unsigned int j = 0; j < mesh.getNumVertices(); ++j)
		{
			EXPECT_EQ(testPositions[j], vertices[j].position);

			const MockVertexData& data = vertices[j].data;
			EXPECT_EQ(j, data.getId());
			EXPECT_EQ(testNormals[j], data.getNormal());
		}
	}

	/// Create the test edges
	for (unsigned int i = 0; i < testEdgesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createEdge(testEdgesVerticesId[i]));
		EXPECT_EQ(i + 1, mesh.getNumEdges());

		const std::vector<MockTriangleMesh::Edge>& edges = mesh.getEdges();
		EXPECT_EQ(i + 1, edges.size());

		/// Make sure each vertex is set properly
		for (unsigned int j = 0; j < mesh.getNumEdges(); ++j)
		{
			EXPECT_EQ(testEdgesVerticesId[j], edges[j].vertices);

			const MockEdgeData& data = edges[j].data;
			EXPECT_EQ(j, data.getId());
		}
	}

	/// Create the test triangles
	for (unsigned int i = 0; i < testTrianglesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createTriangle(testTrianglesVerticesId[i], testTrianglesEdgesId[i]));
		EXPECT_EQ(i + 1, mesh.getNumTriangles());

		const std::vector<MockTriangleMesh::Triangle>& triangles = mesh.getTriangles();
		EXPECT_EQ(i + 1, triangles.size());

		/// Make sure each vertex is set properly
		for (unsigned int j = 0; j < mesh.getNumTriangles(); ++j)
		{
			EXPECT_EQ(testTrianglesVerticesId[j], triangles[j].vertices);

			const MockTriangleData& data = triangles[j].data;
			EXPECT_EQ(j, data.getId());
			EXPECT_EQ(testTrianglesEdgesId[j], data.getEdges());
		}
	}

	/// Create the test tetrahedrons
	for (unsigned int i = 0; i < testTetrahedronsVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createTetrahedron(testTetrahedronsVerticesId[i], testTetrahedronsEdgesId[i], testTetrahedronsTrianglesId[i]));
		EXPECT_EQ(i + 1, mesh.getNumTetrahedrons());

		const std::vector<MockTetrahedronMesh::Tetrahedron>& tetrahedrons = mesh.getTetrahedrons();
		EXPECT_EQ(i + 1, tetrahedrons.size());

		/// Make sure each tetrahedron is set properly
		for (unsigned int j = 0; j < mesh.getNumTetrahedrons(); ++j)
		{
			EXPECT_EQ(testTetrahedronsVerticesId[j], tetrahedrons[j].vertices);

			const MockTetrahedronData& data = tetrahedrons[j].data;
			EXPECT_EQ(j, data.getId());
			EXPECT_EQ(testTetrahedronsEdgesId[j], data.getEdges());
			EXPECT_EQ(testTetrahedronsTrianglesId[j], data.getTriangles());
		}
	}
}

TEST_F(TetrahedronMeshTest, SetVertexPositionsTest)
{
	MockTetrahedronMesh mesh;

	/// Create vertices with test normals, but all positions at (0,0,0)
	for (unsigned int i = 0; i < testPositions.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createVertex(Vector3d(0.0, 0.0, 0.0), testNormals[i]));
		EXPECT_EQ(i + 1, mesh.getNumVertices());
	}

	mesh.setVertexPositions(testPositions);

	EXPECT_EQ(1, mesh.getNumUpdates());
	EXPECT_EQ(testPositions.size(), mesh.getNumVertices());

	const std::vector<MockMesh::Vertex>& vertices = mesh.getVertices();
	EXPECT_EQ(testPositions.size(), vertices.size());

	/// Make sure each vertex is set properly
	for (unsigned int i = 0; i < mesh.getNumVertices(); ++i)
	{
		EXPECT_EQ(testPositions[i], vertices[i].position);

		const MockVertexData& data = vertices[i].data;
		EXPECT_EQ(testNormals[i], data.getNormal());
	}

	mesh.setVertexPositions(testPositions, false);

	EXPECT_EQ(1, mesh.getNumUpdates());
	EXPECT_EQ(testPositions.size(), mesh.getNumVertices());
	EXPECT_EQ(testPositions.size(), mesh.getVertices().size());

	mesh.setVertexPositions(testPositions, true);

	EXPECT_EQ(2, mesh.getNumUpdates());
	EXPECT_EQ(testPositions.size(), mesh.getNumVertices());
	EXPECT_EQ(testPositions.size(), mesh.getVertices().size());

	/// Test the individual set/get methods
	mesh.setVertexPosition(5, Vector3d(0.0, 0.0, 0.0));

	/// Make sure each vertex is set properly
	for (unsigned int i = 0; i < mesh.getNumVertices(); ++i)
	{
		if (i == 5)
		{
			EXPECT_EQ(Vector3d(0.0, 0.0, 0.0), mesh.getVertexPosition(i));
			EXPECT_EQ(testNormals[i],  mesh.getVertexNormal(i));
		}
		else
		{
			EXPECT_EQ(testPositions[i], mesh.getVertexPosition(i));
			EXPECT_EQ(testNormals[i],  mesh.getVertexNormal(i));
		}
	}

	/// Try setting with wrong number of vertices
	mesh.createVertex(Vector3d(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0)); // create one more vertex

	EXPECT_ANY_THROW(mesh.setVertexPositions(testPositions));
}

TEST_F(TetrahedronMeshTest, ClearTest)
{
	MockTetrahedronMesh mesh;

	EXPECT_EQ(0, mesh.getNumUpdates());

	EXPECT_EQ(0u, mesh.getNumVertices());
	EXPECT_EQ(0u, mesh.getVertices().size());

	EXPECT_EQ(0u, mesh.getNumEdges());
	EXPECT_EQ(0u, mesh.getEdges().size());

	EXPECT_EQ(0u, mesh.getNumTriangles());
	EXPECT_EQ(0u, mesh.getTriangles().size());

	EXPECT_EQ(0u, mesh.getNumTetrahedrons());
	EXPECT_EQ(0u, mesh.getTetrahedrons().size());

	/// Create mesh using test data
	for (unsigned int i = 0; i < testPositions.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createVertex(Vector3d(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0)));
		EXPECT_EQ(i + 1, mesh.getNumVertices());
	}
	for (unsigned int i = 0; i < testEdgesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createEdge(testEdgesVerticesId[i]));
		EXPECT_EQ(i + 1, mesh.getNumEdges());
	}
	for (unsigned int i = 0; i < testTrianglesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createTriangle(testTrianglesVerticesId[i], testTrianglesEdgesId[i]));
		EXPECT_EQ(i + 1, mesh.getNumTriangles());
	}
	for (unsigned int i = 0; i < testTetrahedronsVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createTetrahedron(testTetrahedronsVerticesId[i], testTetrahedronsEdgesId[i], testTetrahedronsTrianglesId[i]));
		EXPECT_EQ(i + 1, mesh.getNumTetrahedrons());
	}

	EXPECT_EQ(testPositions.size(), mesh.getNumVertices());
	EXPECT_EQ(testPositions.size(), mesh.getVertices().size());

	EXPECT_EQ(testEdgesVerticesId.size(), mesh.getNumEdges());
	EXPECT_EQ(testEdgesVerticesId.size(), mesh.getEdges().size());

	EXPECT_EQ(testTrianglesVerticesId.size(), mesh.getNumTriangles());
	EXPECT_EQ(testTrianglesVerticesId.size(), mesh.getTriangles().size());

	EXPECT_EQ(testTetrahedronsVerticesId.size(), mesh.getNumTetrahedrons());
	EXPECT_EQ(testTetrahedronsVerticesId.size(), mesh.getTetrahedrons().size());

	/// Clear mesh
	mesh.clear();

	EXPECT_EQ(0u, mesh.getNumVertices());
	EXPECT_EQ(0u, mesh.getVertices().size());

	EXPECT_EQ(0u, mesh.getNumEdges());
	EXPECT_EQ(0u, mesh.getEdges().size());

	EXPECT_EQ(0u, mesh.getNumTriangles());
	EXPECT_EQ(0u, mesh.getTriangles().size());

	EXPECT_EQ(0u, mesh.getNumTetrahedrons());
	EXPECT_EQ(0u, mesh.getTetrahedrons().size());
}

TEST_F(TetrahedronMeshTest, UpdateTest)
{
	MockTetrahedronMesh mesh;

	EXPECT_EQ(0, mesh.getNumUpdates());

	for (int i = 0; i < 10; ++i)
	{
		mesh.update();
		EXPECT_EQ(i + 1, mesh.getNumUpdates());
	}
}

TEST_F(TetrahedronMeshTest, ComparisonTest)
{
	/// Create mesh using test data
	MockTetrahedronMesh mesh;
	for (unsigned int i = 0; i < testPositions.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createVertex(testPositions[i], testNormals[i]));
	}
	for (unsigned int i = 0; i < testEdgesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createEdge(testEdgesVerticesId[i]));
	}
	for (unsigned int i = 0; i < testTrianglesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createTriangle(testTrianglesVerticesId[i], testTrianglesEdgesId[i]));
	}
	for (unsigned int i = 0; i < testTetrahedronsVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, mesh.createTetrahedron(testTetrahedronsVerticesId[i], testTetrahedronsEdgesId[i], testTetrahedronsTrianglesId[i]));
	}

	/// Create same mesh again
	MockTetrahedronMesh sameMesh;

	for (unsigned int i = 0; i < testPositions.size(); ++i)
	{
		EXPECT_EQ(i, sameMesh.createVertex(testPositions[i], testNormals[i]));
	}
	for (unsigned int i = 0; i < testEdgesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, sameMesh.createEdge(testEdgesVerticesId[i]));
	}
	for (unsigned int i = 0; i < testTrianglesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, sameMesh.createTriangle(testTrianglesVerticesId[i], testTrianglesEdgesId[i]));
	}
	for (unsigned int i = 0; i < testTetrahedronsVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, sameMesh.createTetrahedron(testTetrahedronsVerticesId[i], testTetrahedronsEdgesId[i], testTetrahedronsTrianglesId[i]));
	}

	/// Create mesh with test data, but each vertex has position and normal of (0,0,0) to make them different
	MockTetrahedronMesh meshWithDifferentVertices;
	for (unsigned int i = 0; i < testPositions.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentVertices.createVertex(Vector3d(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0)));
	}
	for (unsigned int i = 0; i < testEdgesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentVertices.createEdge(testEdgesVerticesId[i]));
	}
	for (unsigned int i = 0; i < testTrianglesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentVertices.createTriangle(testTrianglesVerticesId[i], testTrianglesEdgesId[i]));
	}
	for (unsigned int i = 0; i < testTetrahedronsVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentVertices.createTetrahedron(testTetrahedronsVerticesId[i], testTetrahedronsEdgesId[i], testTetrahedronsTrianglesId[i]));
	}

	/// Create mesh with test data, but reverse each edge's vertex order to make them different
	MockTetrahedronMesh meshWithDifferentEdges;
	for (unsigned int i = 0; i < testPositions.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentEdges.createVertex(testPositions[i], testNormals[i]));
	}
	for (unsigned int i = 0; i < testEdgesVerticesId.size(); ++i)
	{
		std::array<unsigned int, 2> edge = {{ testEdgesVerticesId[i][1], testEdgesVerticesId[i][0] }};
		EXPECT_EQ(i, meshWithDifferentEdges.createEdge(edge));
	}
	for (unsigned int i = 0; i < testTrianglesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentEdges.createTriangle(testTrianglesVerticesId[i], testTrianglesEdgesId[i]));
	}
	for (unsigned int i = 0; i < testTetrahedronsVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentEdges.createTetrahedron(testTetrahedronsVerticesId[i], testTetrahedronsEdgesId[i], testTetrahedronsTrianglesId[i]));
	}

	/// Create mesh with test data, but only create half of the triangles to make the list different.
	MockTetrahedronMesh meshWithDifferentTriangles;
	for (unsigned int i = 0; i < testPositions.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentTriangles.createVertex(testPositions[i], testNormals[i]));
	}
	for (unsigned int i = 0; i < testEdgesVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentTriangles.createEdge(testEdgesVerticesId[i]));
	}
	for (unsigned int i = 0; i < testTrianglesVerticesId.size() / 2; ++i)
	{
		EXPECT_EQ(i, meshWithDifferentTriangles.createTriangle(testTrianglesVerticesId[i], testTrianglesEdgesId[i]));
	}
	for (unsigned int i = 0; i < testTetrahedronsVerticesId.size(); ++i)
	{
		EXPECT_EQ(i, meshWithDifferentTriangles.createTetrahedron(testTetrahedronsVerticesId[i], testTetrahedronsEdgesId[i], testTetrahedronsTrianglesId[i]));
	}

	/// Test comparisons
	EXPECT_TRUE(mesh == sameMesh);
	EXPECT_FALSE(mesh != sameMesh);

	EXPECT_FALSE(mesh == meshWithDifferentVertices);
	EXPECT_TRUE(mesh != meshWithDifferentVertices);

	EXPECT_FALSE(mesh == meshWithDifferentEdges);
	EXPECT_TRUE(mesh != meshWithDifferentEdges);

	EXPECT_FALSE(mesh == meshWithDifferentTriangles);
	EXPECT_TRUE(mesh != meshWithDifferentTriangles);
}
