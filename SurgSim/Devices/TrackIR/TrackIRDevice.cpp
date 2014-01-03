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

#include "SurgSim/Devices/TrackIR/TrackIRDevice.h"

#include "SurgSim/Framework/Log.h"
#include "SurgSim/Devices/TrackIR/TrackIRScaffold.h"

namespace SurgSim
{
namespace Device
{

TrackIRDevice::TrackIRDevice(const std::string& uniqueName) :
	SurgSim::Input::CommonDevice(uniqueName, TrackIRScaffold::buildDeviceInputData())
{
}

TrackIRDevice::~TrackIRDevice()
{
	if (isInitialized())
	{
		finalize();
	}
}


bool TrackIRDevice::initialize()
{
	SURGSIM_ASSERT(! isInitialized());
	std::shared_ptr<TrackIRScaffold> scaffold = TrackIRScaffold::getOrCreateSharedInstance();
	SURGSIM_ASSERT(scaffold);

	if (! scaffold->registerDevice(this))
	{
		return false;
	}

	m_scaffold = std::move(scaffold);
	SURGSIM_LOG_INFO(m_scaffold->getLogger()) << "Device " << getName() << ": " << "Initialized.";
	return true;
}

bool TrackIRDevice::finalize()
{
	SURGSIM_ASSERT(isInitialized());
	SURGSIM_LOG_INFO(m_scaffold->getLogger()) << "Device " << getName() << ": " << "Finalizing.";
	bool ok = m_scaffold->unregisterDevice(this);
	m_scaffold.reset();
	return ok;
}

bool TrackIRDevice::isInitialized() const
{
	return (nullptr != m_scaffold);
}


void TrackIRDevice::setPositionScale(double scale)
{
	m_positionScale = scale;
	if (m_scaffold)
	{
		m_scaffold->setPositionScale(this, m_positionScale);
	}
}

double TrackIRDevice::getPositionScale() const
{
	return m_positionScale;
}


void TrackIRDevice::setOrientationScale(double scale)
{
	m_orientationScale = scale;
	if (m_scaffold)
	{
		m_scaffold->setOrientationScale(this, m_orientationScale);
	}
}

double TrackIRDevice::getOrientationScale() const
{
	return m_orientationScale;
}


double TrackIRDevice::defaultPositionScale()
{
	return 0.5;
}

double TrackIRDevice::defaultOrientationScale()
{
	return 0.03;
}

};  // namespace Device
};  // namespace SurgSim
