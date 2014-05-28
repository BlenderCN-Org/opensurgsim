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

#ifndef SURGSIM_COLLISION_REPRESENTATION_H
#define SURGSIM_COLLISION_REPRESENTATION_H

#include <list>
#include <memory>
#include <unordered_map>

#include "SurgSim/DataStructures/BufferedValue.h"
#include "SurgSim/Framework/Representation.h"

namespace SurgSim
{

namespace Math
{
class Shape;
};

namespace Physics
{
class Representation;
};

namespace Collision
{
struct Contact;
class Representation;

typedef std::unordered_map<std::shared_ptr<SurgSim::Collision::Representation>,
							std::list<std::shared_ptr<SurgSim::Collision::Contact>>> ContactMapType;

/// Wrapper class to use for the collision operation, handles its enclosed shaped
/// and a possible local to global coordinate system transform, if the physics representation
/// is a nullptr or a has gone out of scope ASSERT's will be triggered.
/// Collision with other representations will be updated by CollisionPair::addContact() and
/// be cleared every time DcdCollision::updatePair() makes a new CollisionPair.
class Representation : public SurgSim::Framework::Representation
{
public:
	/// Constructor
	/// \param name Name of this collision representation
	explicit Representation(const std::string& name);

	/// Destructor
	virtual ~Representation();

	/// Get the shape type id
	/// \return The unique type of the shape, used to determine which calculation to use.
	virtual int getShapeType() const = 0;

	/// Get the shape
	/// \return The actual shape used for collision.
	virtual const std::shared_ptr<SurgSim::Math::Shape> getShape() const = 0;

	/// A map between collision representations and contacts.
	/// For each collision representation, it gives the list of contacts registered against this instance.
	/// The map is stored in a BufferedValue.  If accessing from the PhysicsManager thread, use a ReadAccessor,
	/// otherwise use a SafeReadAccessor.  Write access is not provided, instead use addCollisionWith and
	/// clearCollisions.
	/// \return A map with collision representations as keys and lists of contacts as the associated value.
	std::shared_ptr<SurgSim::DataStructures::BufferedValue<ContactMapType>> getCollisions() const;

	/// Add a contact against a given collision representation.
	/// \param collisionRepresentation The collision representation to which this collision representation collides.
	/// \param contact The contact information.
	/// \note The Contact object added to the map follows the convention of pointing the contact normal toward
	/// this representation. And the first penetration point is on this representation and the second is on
	/// collisionRepresentation.
	void addCollisionWith(const std::shared_ptr<SurgSim::Collision::Representation>& collisionRepresentation,
						  const std::shared_ptr<SurgSim::Collision::Contact>& contact);

	/// Clear all the collisions.
	void clearCollisions();

	/// Update the representation.
	/// \param dt the time passed from the last update.
	virtual void update(const double& dt);

	/// Publish the buffered collision map.
	virtual void publishCollisions();

protected:
	/// A map which associates a list of contacts with each collision representation.
	/// Every contact added to this map follows the convention of pointing the contact normal toward this
	/// representation. And the first penetration point is on this representation.
	std::shared_ptr<SurgSim::DataStructures::BufferedValue<ContactMapType>> m_collisions;

	/// The write accessor to the collisions.
	SurgSim::DataStructures::ReadWriteAccessor<ContactMapType> m_writeCollisions;
};


}; // namespace Collision
}; // namespace SurgSim

#endif
