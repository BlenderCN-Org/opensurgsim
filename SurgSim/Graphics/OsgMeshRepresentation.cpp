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

#include "SurgSim/Graphics/OsgMeshRepresentation.h"

#include "SurgSim/Graphics/OsgConversions.h"
#include "SurgSim/Graphics/OsgTexture.h"
#include "SurgSim/Graphics/Texture.h"
#include "SurgSim/Graphics/TriangleNormalGenerator.h"

#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Switch>
#include <osg/Vec3f>
#include <osg/PolygonMode>
#include <osg/TriangleIndexFunctor>
#include <osg/PositionAttitudeTransform>

#include <osgUtil/SmoothingVisitor>


namespace SurgSim
{
namespace Graphics
{

OsgMeshRepresentation::OsgMeshRepresentation(const std::string& name) :
	Representation(name),
	OsgRepresentation(name),
	MeshRepresentation(name),
	m_mesh(std::make_shared<Mesh>()),
	m_updateOptions(UPDATE_OPTION_VERTICES)
{
	// The actual size of the mesh is not known at this time, just allocate the
	// osg structures that are needed and add them to the geometry, and the node

	m_geometry = new osg::Geometry();

	// Set up vertices array
	m_vertices = new osg::Vec3Array();
	m_vertices->setDataVariance(osg::Object::DYNAMIC);
	m_geometry->setVertexArray(m_vertices);

	// Set up color array with default color
	m_colors = new osg::Vec4Array(1);
	(*m_colors)[0]= osg::Vec4(1.0f,1.0f,1.0f,1.0f);
	m_geometry->setColorArray(m_colors, osg::Array::BIND_OVERALL);


	// Set up textureCoordinates array, texture coords are optional, don't add them to the
	// geometry yet
	m_textureCoordinates = new osg::Vec2Array(0);
	m_textureCoordinates->setDataVariance(osg::Object::DYNAMIC);


	// Set up primitive set for triangles
	m_triangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
	m_triangles->setDataVariance(osg::Object::DYNAMIC);
	m_geometry->addPrimitiveSet(m_triangles);

	// Create normals, currently per triangle
	m_normals = new osg::Vec3Array();
	m_normals->setDataVariance(osg::Object::DYNAMIC);
	m_geometry->setNormalArray(m_normals,osg::Array::BIND_PER_VERTEX);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(m_geometry);

	m_transform->addChild(geode);
}

OsgMeshRepresentation::~OsgMeshRepresentation()
{
}

std::shared_ptr<Mesh> OsgMeshRepresentation::getMesh()
{
	return m_mesh;
}

void OsgMeshRepresentation::setDrawAsWireFrame(bool val)
{
	osg::StateSet* state = m_switch->getOrCreateStateSet();

	osg::ref_ptr<osg::PolygonMode> polygonMode;
	if (val)
	{
		 polygonMode = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	}
	else
	{
		polygonMode = new osg::PolygonMode(osg::PolygonMode::FRONT, osg::PolygonMode::FILL);
	}

	state->setAttributeAndModes(polygonMode, osg::StateAttribute::ON);
}

void OsgMeshRepresentation::update(double dt)
{
	SURGSIM_ASSERT(m_mesh->isValid()) << "The mesh in the OsgMeshRepresentation " << getName() << " is invalid.";

	int updateOptions = updateOsgArrays();
	updateOptions |= m_updateOptions;

	if ((updateOptions & (UPDATE_OPTION_VERTICES | UPDATE_OPTION_TEXTURES | UPDATE_OPTION_COLORS)) != 0)
	{
		updateVertices(updateOptions);
		m_geometry->dirtyDisplayList();
		m_geometry->dirtyBound();
	}

	if ((updateOptions & UPDATE_OPTION_TRIANGLES) != 0)
	{
		updateTriangles();
		m_triangles->dirty();
	}
}

void OsgMeshRepresentation::updateVertices(int updateOptions)
{
	static osg::Vec4d defaultColor(0.8, 0.2, 0.2, 1.0);
	static osg::Vec2d defaultTextureCoord(0.0, 0.0);

	bool updateColors = (updateOptions & UPDATE_OPTION_COLORS) != 0;
	bool updateTextures = (updateOptions & UPDATE_OPTION_TEXTURES) != 0;
	bool updateVertices = (updateOptions & UPDATE_OPTION_VERTICES) != 0;
	size_t vertexCount = m_mesh->getNumVertices();

	for (size_t i=0; i < vertexCount; ++i)
	{
		Mesh::VertexType vertex = m_mesh->getVertex(i);
		if (updateVertices)
		{
			(*m_vertices)[i].set(toOsg(vertex.position));
		}
		if (updateColors)
		{
			(*m_colors)[i] =
				(vertex.data.color.hasValue()) ? toOsg(vertex.data.color.getValue()) : defaultColor;
		}
		if (updateTextures)
		{
			(*m_textureCoordinates)[i] =
				(vertex.data.texture.hasValue()) ? toOsg(vertex.data.texture.getValue()) : defaultTextureCoord;
		}
	}

	if (updateVertices)
	{
		updateNormals();
	}
}

void OsgMeshRepresentation::updateNormals()
{
	// Generate normals from geometry
	auto normalGenerator = createNormalGenerator(m_vertices, m_normals);
	m_geometry->accept(normalGenerator);
	normalGenerator.normalize();
}

void OsgMeshRepresentation::updateTriangles()
{
	std::vector<Mesh::TriangleType> triangles = m_mesh->getTriangles();
	auto endIt = std::end(triangles);
	int i = 0;
	for (auto it = std::begin(triangles); it != endIt; ++it)
	{
		(*m_triangles)[i++] = it->verticesId[0];
		(*m_triangles)[i++] = it->verticesId[1];
		(*m_triangles)[i++] = it->verticesId[2];
	}
}

int OsgMeshRepresentation::updateOsgArrays()
{
	int result = 0;

	size_t numVertices = m_mesh->getNumVertices();
	if (numVertices > m_vertices->size())
	{
		m_vertices->resize(numVertices);
		m_normals->resize(numVertices);

		m_vertices->setDataVariance(getDataVariance(UPDATE_OPTION_VERTICES));
		m_normals->setDataVariance(getDataVariance(UPDATE_OPTION_VERTICES));

		result |= UPDATE_OPTION_VERTICES;
	}

	// The first vertex determines what values the mesh should have ...
	Mesh::VertexType vertex = m_mesh->getVertex(0);

	if (vertex.data.color.hasValue() && numVertices > m_colors->size())
	{
		if (m_colors->size() > 1)
		{
			m_colors->setDataVariance(getDataVariance(UPDATE_OPTION_COLORS));
			m_geometry->setColorArray(m_colors, osg::Array::BIND_PER_VERTEX);
		}
		m_colors->resize(numVertices);
		result |= UPDATE_OPTION_COLORS;
	}

	if (vertex.data.texture.hasValue() && numVertices > m_textureCoordinates->size())
	{
		bool setTextureArray = m_textureCoordinates->size() == 0;
		m_textureCoordinates->resize(numVertices);
		if (setTextureArray)
		{
			m_geometry->setTexCoordArray(0, m_textureCoordinates, osg::Array::BIND_PER_VERTEX);
			m_textureCoordinates->setDataVariance(getDataVariance(UPDATE_OPTION_TEXTURES));
		}
		result |= UPDATE_OPTION_TEXTURES;
	}

	if (m_mesh->getNumTriangles()*3 > m_triangles->size())
	{
		m_triangles->resize(m_mesh->getNumTriangles()*3);
		m_triangles->setDataVariance(getDataVariance(UPDATE_OPTION_TRIANGLES));
		result |= UPDATE_OPTION_TRIANGLES;
	}
	return result;
}

void OsgMeshRepresentation::setUpdateOptions(int val)
{
	if (val <= UPDATE_OPTION_ALL && val >= UPDATE_OPTION_NONE)
	{
		m_updateOptions = val;
	}
}

int OsgMeshRepresentation::getUpdateOptions() const
{
	return m_updateOptions;
}

osg::ref_ptr<osg::Geometry> OsgMeshRepresentation::getOsgGeometry()
{
	return m_geometry;
}

osg::Object::DataVariance OsgMeshRepresentation::getDataVariance(int updateOption)
{
	return ((m_updateOptions & updateOption) != 0) ? osg::Object::DYNAMIC : osg::Object::STATIC;
}




}; // Graphics

}; // SurgSim