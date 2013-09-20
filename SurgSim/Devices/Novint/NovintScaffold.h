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

#ifndef SURGSIM_DEVICES_NOVINT_NOVINTSCAFFOLD_H
#define SURGSIM_DEVICES_NOVINT_NOVINTSCAFFOLD_H

#include <memory>

#include <SurgSim/Framework/Logger.h>
#include <SurgSim/DataStructures/DataGroup.h>

namespace SurgSim
{
namespace Device
{

class NovintDevice;

/// A class that manages Novint Falcon devices.
///
/// This should support any device that can communicate using the Novint HDAL SDK toolkit, such as the
/// off-the-shelf Novint Falcon gaming controller and the Novint Falcon with the Open Surgery Grip.
///
/// \sa SurgSim::Device::NovintDevice
class NovintScaffold
{
public:
	/// Constructor.
	/// \param logger (optional) The logger to be used for the scaffold object and the devices it manages.
	/// 			  If unspecified or empty, a console logger will be created and used.
	explicit NovintScaffold(std::shared_ptr<SurgSim::Framework::Logger> logger = nullptr);

	/// Destructor.
	~NovintScaffold();

	/// Gets the logger used by this object and the devices it manages.
	/// \return The logger.
	std::shared_ptr<SurgSim::Framework::Logger> getLogger() const
	{
		return m_logger;
	}

	/// Gets or creates the scaffold shared by all NovintDevice instances.
	/// The scaffold is managed using a SingleInstance object, so it will be destroyed when all devices are released.
	/// \return the scaffold object.
	static std::shared_ptr<NovintScaffold> getOrCreateSharedInstance();

	/// Sets the default log level.
	/// Has no effect unless called before a scaffold is created (i.e. before the first device).
	/// \param logLevel The log level.
	static void setDefaultLogLevel(SurgSim::Framework::LogLevel logLevel);

private:

	/// Internal shared state data type.
	struct StateData;
	/// Interal per-device information.
	struct DeviceData;
	/// Wrapper for the haptic loop callback.
	class Callback;
	/// Wrapper for the HDAL device handle.
	class Handle;

	friend class NovintDevice;

	/// Registers the specified device object.
	/// If successful, the device object will become connected to an unused controller.
	///
	/// \param device The device object to be used, which should have a unique name.
	/// \return True if the initialization succeeds, false if it fails.
	bool registerDevice(NovintDevice* device);

	/// Unregisters the specified device object.
	/// The corresponding controller will become unused, and can be re-registered later.
	///
	/// \param device The device object.
	/// \return true on success, false on failure.
	bool unregisterDevice(const NovintDevice* device);

	/// Initializes a single device, creating the necessary HDAL resources.
	/// \param [in,out] info	The device data.
	/// \return	true on success.
	bool initializeDeviceState(DeviceData* info);

	/// Finalizes a single device, destroying the necessary HDAL resources.
	/// \param [in,out] info	The device data.
	/// \return	true on success.
	bool finalizeDeviceState(DeviceData* info);

	/// Updates the device information for a single device.
	/// \param info	The device data.
	/// \return	true on success.
	bool updateDevice(DeviceData* info);

	/// Initializes the HDAL SDK.
	/// \return true on success.
	bool initializeSdk();

	/// Finalizes (de-initializes) the HDAL SDK.
	/// \return true on success.
	bool finalizeSdk();

	/// Executes the operations for a single haptic frame.
	/// Should only be called from the context of a HDAL callback.
	/// \return true on success.
	bool runHapticFrame();

	/// Creates the haptic loop callback.
	/// \return true on success.
	bool createHapticLoop();

	/// Destroys the haptic loop callback.
	/// Should be called while NOT holding the internal device list mutex, to prevent deadlock.
	/// \return true on success.
	bool destroyHapticLoop();

	/// Starts the HDAL scheduler.
	/// \return	true on success.
	bool startScheduler();

	/// Stops the HDAL scheduler.
	/// \return	true on success.
	bool stopScheduler();

	/// Gets the gravity compensation flag for the current device.
	/// \param info	The device data.
	/// \param [out] gravityCompensationState	State of the gravity compensation flag.
	/// \return	true if it succeeds, false if it fails.
	bool getGravityCompensation(const DeviceData* info, bool* gravityCompensationState);

	/// Attempts to force the gravity compensation flag for the current device to the specified value.
	///
	/// Sets the flag repeatedly, until it reports the desired value, in order to work around the problem where
	/// attempts to set the gravity compensation do not always change the actual gravity compensation flag in the
	/// device.
	///
	/// Logs a message if the state is known to have been changed.
	///
	/// \param info	The device data.
	/// \param gravityCompensationState	Desired state of the gravity compensation flag.
	/// \return	true if it succeeds, false if it fails.
	bool enforceGravityCompensation(const DeviceData* info, bool gravityCompensationState);

	/// Sets the gravity compensation flag for the current device, unless it's already set as desired.
	/// \param info	The device data.
	/// \param gravityCompensationState	Desired state of the gravity compensation flag.
	/// \return	true if it succeeds, false if it fails.
	bool setGravityCompensation(const DeviceData* info, bool gravityCompensationState);

	/// Check for HDAL errors, display them, and signal fatal errors.
	/// Exactly equivalent to <code>checkForFatalError(false, message)</code>.
	/// \param message An additional descriptive message.
	/// \return true if there was a fatal error; false if everything is OK.
	bool checkForFatalError(const char* message);

	/// Check for HDAL errors, display them, and signal fatal errors.
	/// Exactly equivalent to <code>checkForFatalError(message) || previousError</code>, but less nasty to read.
	/// \param previousError	True if a previous error has occurred.
	/// \param message	An additional descriptive message.
	/// \return	true if there was a fatal error or if previousError is true; false if everything is OK.
	bool checkForFatalError(bool previousError, const char* message)
	{
		bool newError = checkForFatalError(message);
		return previousError || newError;
	}

	/// Builds the data layout for the application input (i.e. device output).
	static SurgSim::DataStructures::DataGroup buildDeviceInputData();



	/// Logger used by the scaffold and all devices.
	std::shared_ptr<SurgSim::Framework::Logger> m_logger;
	/// Internal scaffold state.
	std::unique_ptr<StateData> m_state;

	/// The default logging level.
	static SurgSim::Framework::LogLevel m_defaultLogLevel;
};

};  // namespace Device
};  // namespace SurgSim

#endif  // SURGSIM_DEVICES_NOVINT_NOVINTSCAFFOLD_H