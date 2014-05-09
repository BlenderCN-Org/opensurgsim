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

#include "SurgSim/Graphics/Camera.h"
#include "SurgSim/Math/MathConvert.h"

namespace SurgSim
{
namespace Graphics
{

Camera::Camera(const std::string& name) : Representation(name)
{
	SURGSIM_ADD_SERIALIZABLE_PROPERTY(Camera, SurgSim::Math::Matrix44d, ProjectionMatrix,
									  getProjectionMatrix, setProjectionMatrix);
	SURGSIM_ADD_SERIALIZABLE_PROPERTY(Camera, std::string, RenderGroupReference,
									  getRenderGroupReference, setRenderGroupReference);

	SURGSIM_ADD_RO_PROPERTY(Camera, SurgSim::Math::Matrix44d, ViewMatrix, getViewMatrix);
	SURGSIM_ADD_RO_PROPERTY(Camera, SurgSim::Math::Matrix44f, FloatViewMatrix, getViewMatrix);
	SURGSIM_ADD_RO_PROPERTY(Camera, SurgSim::Math::Matrix44f, FloatProjectionMatrix, getProjectionMatrix);
	SURGSIM_ADD_RO_PROPERTY(Camera, SurgSim::Math::Matrix44f, FloatInverseViewMatrix, getInverseViewMatrix);
}

void Camera::setRenderGroupReference(const std::string& name)
{
	removeGroupReference(name);
	m_renderGroupReference = name;
}

std::string Camera::getRenderGroupReference() const
{
	return m_renderGroupReference;
}

bool Camera::setRenderGroup(std::shared_ptr<Group> group)
{
	m_group = group;
	return true;
}

std::shared_ptr<Group> Camera::getRenderGroup() const
{
	return m_group;
}

bool Camera::addGroupReference(const std::string& name)
{
	bool result = false;
	if (name != m_renderGroupReference)
	{
		result = Representation::addGroupReference(name);
	}
	return result;
}

bool Camera::doInitialize()
{
	SURGSIM_ASSERT(m_renderGroupReference != "")
			<< "Can't have a camera without a RenderGroupReference.";
	return true;
}

}
}

