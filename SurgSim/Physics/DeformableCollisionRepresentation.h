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

#ifndef SURGSIM_PHYSICS_DEFORMABLECOLLISIONREPRESENTATION_H
#define SURGSIM_PHYSICS_DEFORMABLECOLLISIONREPRESENTATION_H

#include "SurgSim/Collision/Representation.h"

#include <string>
#include <memory>

namespace SurgSim
{
namespace DataStructures
{
class TriangleMesh;
}

namespace Math
{
class Shape;
class MeshShape;
}

namespace Physics
{
class DeformableRepresentationState;
class DeformableRepresentationBase;

/// A collision representation that can be attached to a deformable, when this contains a mesh with the same number
/// of vertices as the deformable has nodes, the mesh vertices will move to match the positions of the nodes in
/// the deformable.
class DeformableCollisionRepresentation : public SurgSim::Collision::Representation
{
public:

	/// Constructor
	/// \param name Name of the Representation
	explicit DeformableCollisionRepresentation(const std::string& name);

	/// Destructor
	virtual ~DeformableCollisionRepresentation();

	SURGSIM_CLASSNAME(SurgSim::Physics::DeformableCollisionRepresentation);

	/// Set the mesh to be used in this collision representation
	/// the vertices in the mesh need to be the same number as the vertices in the
	/// deformable representation
	/// \param mesh The mesh to be used for the collision calculation and updates
	void setMesh(std::shared_ptr<SurgSim::DataStructures::TriangleMesh> mesh);

	/// \return The mesh that is part of this representation
	std::shared_ptr<SurgSim::DataStructures::TriangleMesh> getMesh() const;

	/// Set the shape for this collision representation, has to me a meshShape
	/// \param shape The shape to be used.
	void setShape(std::shared_ptr<SurgSim::Math::Shape> shape);

	virtual const std::shared_ptr<SurgSim::Math::Shape> getShape() const override;

	/// Sets the deformable to which this collision representation is connected
	/// \param representation The deformable that will be used to update the contained mesh
	void setDeformableRepresentation(std::shared_ptr<SurgSim::Physics::DeformableRepresentationBase> representation);

	/// \return The deformable that is used to update the contained mesh
	const std::shared_ptr<SurgSim::Physics::DeformableRepresentationBase> getDeformableRepresentation() const;

	virtual int getShapeType() const override;

	virtual void update(const double& dt) override;

	virtual bool doInitialize() override;

	virtual void setInitialPose(const SurgSim::Math::RigidTransform3d& pose) override;

	virtual const SurgSim::Math::RigidTransform3d& getInitialPose() const override;

	virtual void setPose(const SurgSim::Math::RigidTransform3d& pose) override;

	virtual const SurgSim::Math::RigidTransform3d& getPose() const override;

private:

	/// Shape used for collision detection
	std::shared_ptr<SurgSim::Math::MeshShape> m_shape;

	/// Mesh used for collision detection
	std::shared_ptr<SurgSim::DataStructures::TriangleMesh> m_mesh;

	/// Reference to the deformable driving changes to this mesh
	std::weak_ptr<SurgSim::Physics::DeformableRepresentationBase> m_deformable;
};

}
}

#endif