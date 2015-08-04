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

#ifndef SURGSIM_GRAPHICS_CURVE_H
#define SURGSIM_GRAPHICS_CURVE_H

#include "SurgSim/Graphics/Representation.h"
#include "SurgSim/Framework/LockedContainer.h"
#include "SurgSim/DataStructures/Vertices.h"

namespace SurgSim
{

namespace Graphics
{

class CurveRepresentation : public virtual Representation

{
public:
	/// Constructor
	CurveRepresentation(const std::string& name);

	virtual void setSubdivisions(size_t num) = 0;

	virtual size_t getSubdivisions() const = 0;

	virtual void setTension(double tension) = 0;

	virtual double getTension() const = 0;

	virtual void setColor(const SurgSim::Math::Vector4d& color)  = 0;

	virtual Math::Vector4d getColor() const = 0;

	void updateControlPoints(const DataStructures::VerticesPlain& vertices);

	void updateControlPoints(DataStructures::VerticesPlain&& vertices);

protected:

	Framework::LockedContainer<DataStructures::VerticesPlain> m_locker;

};

}
}

#endif
