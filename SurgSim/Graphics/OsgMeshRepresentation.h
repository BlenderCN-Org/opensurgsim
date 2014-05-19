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

#ifndef SURGSIM_GRAPHICS_OSGMESHREPRESENTATION_H
#define SURGSIM_GRAPHICS_OSGMESHREPRESENTATION_H

#include <memory>

#include <osg/Array>
#include <osg/ref_ptr>

#include "SurgSim/Framework/Macros.h"
#include "SurgSim/Graphics/OsgRepresentation.h"
#include "SurgSim/Graphics/MeshRepresentation.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4250)
#endif

namespace osg
{
class Geometry;
class DrawElementsUInt;
}

namespace SurgSim
{
namespace Graphics
{
class Mesh;

/// Implementation of a MeshRepresentation for rendering under osg.
class OsgMeshRepresentation : public OsgRepresentation, public MeshRepresentation
{
public:
	/// Constructor.
	/// \param	name	The name.
	explicit OsgMeshRepresentation(const std::string& name);

	/// Destructor
	~OsgMeshRepresentation();

	SURGSIM_CLASSNAME(SurgSim::Graphics::OsgMeshRepresentation);

	virtual std::shared_ptr<Mesh> getMesh() override;

	virtual void setDrawAsWireFrame(bool val) override;
	virtual bool drawAsWireFrame() const override;

	virtual void setUpdateOptions(int val) override;
	virtual int getUpdateOptions() const override;

	osg::ref_ptr<osg::Geometry> getOsgGeometry();

	virtual void setFilename(std::string filename) override;
	virtual std::string getFilename() const override;

protected:
	virtual void doUpdate(double dt) override;

private:
	/// Indicates if the mesh is rendered as a wireframe.
	bool m_drawAsWireFrame;

	/// Indicates which elements of the mesh should be updated on every frame
	int m_updateOptions;

	/// The mesh.
	std::shared_ptr<Mesh> m_mesh;

	/// File name of the external file which contains the mesh to be used by this class.
	std::string m_filename;

	///@{
	/// Osg structures
	osg::ref_ptr<osg::Geometry> m_geometry;
	osg::ref_ptr<osg::Vec3Array> m_vertices;
	osg::ref_ptr<osg::Vec4Array> m_colors;
	osg::ref_ptr<osg::Vec3Array> m_normals;
	osg::ref_ptr<osg::Vec2Array> m_textureCoordinates;
	osg::ref_ptr<osg::DrawElementsUInt> m_triangles;
	///@}

	/// Updates the internal arrays in accordance to the sizes given in the mesh
	/// \return updateOptions value that indicates which of the structures where updated in size and
	/// 		will have to be updated independent of the value set in setUpdateOptions()
	int updateOsgArrays();

	/// Copies the attributes for each mesh vertex in the appropriate osg structure, this will only be done
	/// for the data as is indicated by updateOptions
	/// \param updateOptions Set of flags indicating whether a specific vertex attribute should be updated
	void updateVertices(int updateOptions);

	/// Updates the normals.
	void updateNormals();

	/// Updates the triangles.
	void updateTriangles();

	/// Gets data variance for a given update option.
	/// \param	updateOption	The update option.
	/// \return	The data variance.
	osg::Object::DataVariance getDataVariance(int updateOption);
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}; // Graphics
}; // SurgSim

#endif // SURGSIM_GRAPHICS_OSGMESHREPRESENTATION_H