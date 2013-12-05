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

#include "SurgSim/Devices/Keyboard/KeyboardDevice.h"
#include "SurgSim/Devices/Keyboard/KeyboardHandler.h"

#include "SurgSim/Graphics/OsgViewElement.h"
#include "SurgSim/Graphics/OsgConversions.h"

#include "SurgSim/Graphics/OsgView.h"
#include "SurgSim/Graphics/OsgTrackballZoomManipulator.h"

using SurgSim::Graphics::OsgView;
using SurgSim::Graphics::OsgViewElement;

OsgViewElement::OsgViewElement(const std::string& name) :
	SurgSim::Graphics::ViewElement(name, std::make_shared<OsgView>(name + " View")),
	m_manipulatorPosition(SurgSim::Math::Vector3d(3.0,3.0,3.0)),
	m_manipulatorLookat(SurgSim::Math::Vector3d(0.0,0.0,0.0)),
	m_keyboardEnabled(false)
{
}

OsgViewElement::~OsgViewElement()
{
}

bool OsgViewElement::setView(std::shared_ptr<SurgSim::Graphics::View> view)
{
	std::shared_ptr<OsgView> osgView = std::dynamic_pointer_cast<OsgView>(view);
	if (osgView && ViewElement::setView(view))
	{
		// After change 'view', need to enable keyboard device in the new 'view' if keyboard is enabled.
		return enableKeyboardDevice(m_keyboardEnabled);
	}
	else
	{
		return false;
	}
}

void SurgSim::Graphics::OsgViewElement::enableManipulator(bool val)
{
	if (m_manipulator == nullptr)
	{
		m_manipulator = new OsgTrackballZoomManipulator();
		// Set a default position
		m_manipulator->setTransformation(
			SurgSim::Graphics::toOsg(m_manipulatorPosition),
			SurgSim::Graphics::toOsg(m_manipulatorLookat),
			osg::Vec3d(0.0f,1.0f,0.0f));
	}

	std::shared_ptr<OsgView> view = std::dynamic_pointer_cast<OsgView>(getView());
	if (view != nullptr)
	{
		if (val)
		{
			view->getOsgView()->setCameraManipulator(m_manipulator);

		}
		else
		{
			view->getOsgView()->setCameraManipulator(nullptr);
		}
	}
}

bool SurgSim::Graphics::OsgViewElement::enableKeyboardDevice(bool val)
{
	std::shared_ptr<OsgView> view = std::dynamic_pointer_cast<OsgView>(getView());
	if (nullptr != view)
	{
		std::shared_ptr<SurgSim::Device::KeyboardDevice> keyboardDevice = getKeyboardDevice();
		osg::ref_ptr<osgGA::GUIEventHandler> keyboardHandle = keyboardDevice->getKeyboardHandler();
		if (val && ! m_keyboardEnabled)
		{
			view->getOsgView()->addEventHandler(keyboardHandle);
			m_keyboardEnabled = true;
		}
		else if (! val && m_keyboardEnabled)
		{
			view->getOsgView()->removeEventHandler(keyboardHandle);
			m_keyboardEnabled = false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

void SurgSim::Graphics::OsgViewElement::setManipulatorParameters(
	SurgSim::Math::Vector3d position,
	SurgSim::Math::Vector3d lookat)
{
	m_manipulatorPosition = position;
	m_manipulatorLookat = lookat;

	if (m_manipulator != nullptr)
	{
		m_manipulator->setTransformation(
			SurgSim::Graphics::toOsg(m_manipulatorPosition),
			SurgSim::Graphics::toOsg(m_manipulatorLookat),
			osg::Vec3d(0.0f,1.0f,0.0f));
	}
}
