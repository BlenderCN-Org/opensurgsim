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

#ifndef SURGSIM_GRAPHICS_OSGOCTREEREPRESENTATION_H
#define SURGSIM_GRAPHICS_OSGOCTREEREPRESENTATION_H

#include <memory>
#include <string>
#include <unordered_map>

#include <osg/ref_ptr>
#include <osg/PositionAttitudeTransform>

#include "SurgSim/Graphics/OctreeRepresentation.h"
#include "SurgSim/Graphics/OsgRepresentation.h"
#include "SurgSim/Math/OctreeShape.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4250)
#endif


namespace SurgSim
{
namespace Graphics
{

class OsgUnitBox;

/// OSG octree representation, implements an OctreeRepresenation using OSG.
class OsgOctreeRepresentation : public OctreeRepresentation, public OsgRepresentation
{
public:
	/// Constructor
	/// \param name Name of OsgOctreeRepresentation
	explicit OsgOctreeRepresentation(const std::string& name);

	/// Destructor
	~OsgOctreeRepresentation();

	/// Executes the update operation
	/// \param	dt	The time step
	virtual void doUpdate(double dt) override;

	/// Get the Octree of this representation
	/// \return    The octree used by this representation.
	virtual std::shared_ptr<SurgSim::Math::OctreeShape::NodeType> getOctree() const override;

	/// Set the Octree of this representation
	/// \param The Octree to be used in this representation.
	virtual void setOctree(std::shared_ptr<SurgSim::Math::OctreeShape> octreeShape) override;

private:
	/// To draw the given Octree Node
	/// \param octreeNode Octree node to be drawn
	/// \return An osg::PositionAttitudeTransform containing the OSG representatoin of the Octree node
	void draw(osg::ref_ptr<osg::Group> thisTransform, SurgSim::Math::Vector3d parentCenter, std::shared_ptr<SurgSim::Math::OctreeShape::NodeType> octreeNode, unsigned level, unsigned parentIndex, unsigned index);


	void addNode(osg::ref_ptr<osg::Group> thisTransform, std::shared_ptr<SurgSim::Math::OctreeShape::NodeType> octreeNode, unsigned key, SurgSim::Math::Vector3d parentCenter);

	/// The Octree represented by this representation
	std::shared_ptr<SurgSim::Math::OctreeShape::NodeType> m_octree;

	/// Shared unit box, so that the geometry can be instanced rather than having multiple copies.
	std::shared_ptr<OsgUnitBox> m_sharedUnitBox;
	/// Returns the shared unit box
	static std::shared_ptr<OsgUnitBox> getSharedUnitBox();

	osg::ref_ptr<osg::Node> m_dummy;

	/// Determine if an OctreeNode with given ID has been added to the scene graph.
	std::unordered_map<unsigned, bool> m_nodeAdded;
	/// Determine the relative index of an OctreeNode in its parent's children list.
	/// E.g. The 2nd child of an OctreeNode may be added to this OctreeNode's corresponding OSG node as its 5th child.
	/// The difference here is that we have two trees: an Octree and a corresponding OSG tree.
	std::unordered_map<unsigned, unsigned> m_nodeIndex;
};

}; // Graphics
}; // SurgSim

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // SURGSIM_GRAPHICS_OSGOCTREEREPRESENTATION_H