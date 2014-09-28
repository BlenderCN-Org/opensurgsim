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

#include "SurgSim/Devices/Nimble/NimbleScaffold.h"

#include <algorithm>
#include <list>
#include <locale>
#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include "SurgSim/Devices/Nimble/NimbleDevice.h"
#include "SurgSim/Devices/Nimble/NimbleThread.h"
#include "SurgSim/DataStructures/DataGroup.h"
#include "SurgSim/DataStructures/DataGroupBuilder.h"
#include "SurgSim/DataStructures/TokenStream.h"
#include "SurgSim/Framework/Assert.h"
#include "SurgSim/Framework/Log.h"
#include "SurgSim/Framework/SharedInstance.h"
#include "SurgSim/Math/Vector.h"
#include "SurgSim/Math/Matrix.h"
#include "SurgSim/Math/RigidTransform.h"

using SurgSim::Math::Vector3d;
using SurgSim::Math::RigidTransform3d;
using SurgSim::Math::Quaterniond;

namespace
{

/// Data structure to hold the data from the Nimble hand tracking SDK, for a single hand.
struct HandTrackingData
{
	/// Number of hands tracked (left, right).
	static const size_t N_HANDS = 2;
	/// Number of joints in one hand.
	static const size_t N_JOINTS = 17;
	/// Number of fingers tracked in one hand.
	static const size_t N_FINGERS = 5;
	/// Number of predefined poses for each hand.
	static const size_t N_POSES = 7;
	/// Number of DOFs for each finger.
	static const size_t N_FINGER_DOFS_PER_HAND = 16;

	/// Position of the hand (w.r.t JointFrameIndex.ROOT_JOINT).
	Vector3d position;
	/// Orientation of the hand (w.r.t JointFrameIndex.ROOT_JOINT).
	Quaterniond quaternion;
	/// Number of times the hand was clicked. 0, 1 or 2.
	int clickCount;
	/// Value between 0 and 1 to specify the confidence of the poses. Currently, either 0 or 1.
	double confidenceEstimate;
	/// Orientation of each of the joints.
	std::array<Quaterniond, N_JOINTS> jointQuaternions;
	/// Position of each of the joints.
	std::array<Vector3d, N_JOINTS> jointPositions;
	/// Position of each of the finger tips.
	std::array<Vector3d, N_FINGERS> fingerTips;
	/// Value between 0 and 1 to specify the confidence of the hand being in one of the N_POSES poses. Sums to 1.
	std::array<double, N_POSES> handPoseConfidences;
	/// The angle of each of the finger joints.
	std::array<double, N_FINGER_DOFS_PER_HAND> fingerDofs;
};

/// A class to parse std::stringstream into requested data type.
/// The class is initialized with a std::stringstream and can parse it into requested datatype.
class NimbleStream : public SurgSim::DataStructures::TokenStream
{
public:
	/// Constructor.
	/// \param stream The string stream from which the values are parsed.
	explicit NimbleStream(std::stringstream* stream)
		: SurgSim::DataStructures::TokenStream(stream)
	{}

	/// Parse the values in the stream into the HandTracking Data structure.
	/// \param [out] handData The data structure where the parsed values are written to.
	/// \return True, if parsing is successful.
	bool parsePose(HandTrackingData* handData)
	{
		bool success = true;

		for (size_t i = 0; success && i < HandTrackingData::N_HANDS; ++i)
		{
			success &= parse(&handData[i].position);
			success &= parse(&handData[i].quaternion);
			success &= parse(&handData[i].clickCount);
		}

		for (size_t i = 0; success && i < HandTrackingData::N_HANDS; ++i)
		{
			success &= parse(&handData[i].confidenceEstimate);
			success &= handData[i].confidenceEstimate >= 0.0 && handData[i].confidenceEstimate <= 1.0;

			for (size_t j = 0; success && j < HandTrackingData::N_JOINTS; ++j)
			{
				success &= parse(&handData[i].jointQuaternions[j]);
				success &= parse(&handData[i].jointPositions[j]);
			}

			for (size_t j = 0; success && j < HandTrackingData::N_FINGERS; ++j)
			{
				success &= parse(&handData[i].fingerTips[j]);
			}
		}

		for (size_t i = 0; success && i < HandTrackingData::N_HANDS; ++i)
		{
			for (size_t j = 0; success && j < HandTrackingData::N_POSES; ++j)
			{
				success &= parse(&handData[i].handPoseConfidences[j]);
			}
		}

		for (size_t i = 0; success && i < HandTrackingData::N_HANDS; ++i)
		{
			for (size_t j = 0; success && j < HandTrackingData::N_FINGER_DOFS_PER_HAND; ++j)
			{
				success &= parse(&handData[i].fingerDofs[j]);
			}
		}

		return success;
	}
};

/// Parse the values in the stream based on its message type (the first few characters).
/// Only certain message types are parsed (POSE).
/// The rest of them are for other uses (CALIBRATION, PINCHING, WELCOME, USER).
/// \param stream The stream read from the socket.
/// \param [out] handData The data structure where the parsed values are written to.
/// \param [out] parseSuccess True, if the parsing to handData was a success.
/// \return True if the message was parsed, instead of being ignored.
bool processNimbleMessage(std::stringstream* stream, HandTrackingData* handData, bool* parseSuccess)
{
	bool messageParsed = false;
	NimbleStream tokenStream(stream);
	std::string messageType;

	if (tokenStream.parse(&messageType) && messageType == "POSE")
	{
		*parseSuccess = tokenStream.parsePose(handData);
		messageParsed = true;
	}

	return messageParsed;
}

}

namespace SurgSim
{
namespace Device
{

struct NimbleScaffold::StateData
{
public:
	/// Initialize the state.
	StateData()
	{
	}

	/// Processing thread.
	std::unique_ptr<NimbleThread> thread;

	/// The socket used for connecting to the Nimble server.
	std::shared_ptr<boost::asio::ip::tcp::iostream> socketStream;

	/// The hand tracking data.
	HandTrackingData handData[2];

	/// The list of active devices.
	std::vector<NimbleDevice*> activeDevices;

	/// The mutex that protects the active device.
	boost::mutex mutex;

private:
	// Prohibit copy construction and assignment
	StateData(const StateData&);
	StateData& operator=(const StateData&);
};

NimbleScaffold::NimbleScaffold(std::shared_ptr<SurgSim::Framework::Logger> logger) :
	m_logger(logger), m_state(new StateData()), m_serverIpAddress("127.0.0.1"), m_serverPort("1988"),
	m_serverSocketOpen(false)
{
	if (m_logger == nullptr)
	{
		m_logger = SurgSim::Framework::Logger::getLogger("Nimble device");
	}
	SURGSIM_LOG_DEBUG(m_logger) << "Nimble: Shared scaffold created.";
}

NimbleScaffold::~NimbleScaffold()
{
	{
		boost::lock_guard<boost::mutex> lock(m_state->mutex);

		if (m_state->activeDevices.size() > 0)
		{
			SURGSIM_LOG_SEVERE(m_logger) << "Nimble: Destroying scaffold while devices are active!?!";
		}
	}

	if (m_state->thread)
	{
		destroyThread();
	}

	SURGSIM_LOG_DEBUG(m_logger) << "Nimble: Shared scaffold destroyed.";
}

bool NimbleScaffold::registerDevice(NimbleDevice* device)
{
	bool success = true;

	{
		boost::lock_guard<boost::mutex> lock(m_state->mutex);

		auto found = std::find_if(m_state->activeDevices.begin(), m_state->activeDevices.end(),
			[device](const NimbleDevice* it) { return it->getName() == device->getName(); });
		
		if (found == m_state->activeDevices.end())
		{
			m_state->activeDevices.push_back(device);
			SURGSIM_LOG_INFO(m_logger) << "Nimble: Device registered in Scaffold.";
		}
		else
		{
			SURGSIM_LOG_SEVERE(m_logger) << "Nimble: Attempt to register device with the same name again.";
			success = false;
		}
	}

	if (success && !m_state->thread)
	{
		createThread();
	}

	return success;
}

bool NimbleScaffold::unregisterDevice(const NimbleDevice* device)
{
	bool success = true;

	{
		boost::lock_guard<boost::mutex> lock(m_state->mutex);

		auto found = std::find(m_state->activeDevices.begin(), m_state->activeDevices.end(), device);

		if (found != m_state->activeDevices.end())
		{
			m_state->activeDevices.erase(found);
			SURGSIM_LOG_INFO(m_logger) << "Nimble: Device unregistered from Scaffold.";
		}
		else
		{
			SURGSIM_LOG_WARNING(m_logger)
				<< "Nimble: Attempted to unregister a device from Scaffold which is not registered.";
			success = false;
		}
	}

	if (m_state->activeDevices.size() == 0 && m_state->thread)
	{
		destroyThread();
	}

	return success;
}

bool NimbleScaffold::initialize()
{
	// Connect to the Nimble hand tracking server.
	m_serverSocketOpen = true;

	m_state->socketStream = std::make_shared<boost::asio::ip::tcp::iostream>(m_serverIpAddress, m_serverPort);

	if (!(*m_state->socketStream))
	{
		SURGSIM_LOG_SEVERE(m_logger) << "Nimble: Error while opening a socket to the server: "
			<< m_state->socketStream->error().message() << ")";
		m_serverSocketOpen = false;
	}

	return m_serverSocketOpen;
}

bool NimbleScaffold::update()
{
	bool success = true;

	if (!(*m_state->socketStream))
	{
		SURGSIM_LOG_SEVERE(m_logger) << "Nimble: Socket stream no longer good: "
			<< m_state->socketStream->error().message() << ")";
		success = false;
	}
	else
	{
		bool handDataParsed = false;
		std::string iosBuf;
		std::getline(*m_state->socketStream, iosBuf);
		std::stringstream serverMessage(iosBuf);

		if (processNimbleMessage(&serverMessage, m_state->handData, &handDataParsed))
		{
			if (handDataParsed)
			{
				updateDeviceData();
			}
			else
			{
				SURGSIM_LOG_WARNING(m_logger)
					<< "Nimble: Hand data not parsed correctly.";
				resetDeviceData();
			}
		}
	}

	return success;
}

void NimbleScaffold::finalize()
{
	// The m_state->thread would be killed soon, so the socket is closed here.
	if (m_serverSocketOpen)
	{
		m_state->socketStream->close();
		if (m_state->socketStream->fail())
		{
			SURGSIM_LOG_SEVERE(m_logger) << "Nimble: Error when shutting down socket: "
				<< m_state->socketStream->error().message() << ")";
		}
	}
}

void NimbleScaffold::updateDeviceData()
{
	boost::lock_guard<boost::mutex> lock(m_state->mutex);

	for (auto it = m_state->activeDevices.begin(); it != m_state->activeDevices.end(); ++it)
	{
		size_t index = (*it)->m_trackedHandDataIndex;

		RigidTransform3d pose = SurgSim::Math::makeRigidTransform(m_state->handData[index].quaternion,
									m_state->handData[index].position);

		SurgSim::DataStructures::DataGroup& inputData = (*it)->getInputData();
		inputData.poses().set(SurgSim::DataStructures::Names::POSE, pose);
		(*it)->pushInput();
	}
}

void NimbleScaffold::resetDeviceData()
{
	boost::lock_guard<boost::mutex> lock(m_state->mutex);

	for (auto it = m_state->activeDevices.begin(); it != m_state->activeDevices.end(); ++it)
	{
		SurgSim::DataStructures::DataGroup& inputData = (*it)->getInputData();
		inputData.resetAll();
	}
}

bool NimbleScaffold::createThread()
{
	SURGSIM_ASSERT(!m_state->thread) << "Nimble: Attempt to create a thread when there is already one.";

	std::unique_ptr<NimbleThread> thread(new NimbleThread(this));
	thread->start();
	m_state->thread = std::move(thread);

	return true;
}

bool NimbleScaffold::destroyThread()
{
	SURGSIM_ASSERT(m_state->thread) << "Nimble: Attempt to destroy thread when there is none.";

	std::unique_ptr<NimbleThread> thread = std::move(m_state->thread);
	thread->stop();
	thread.release();

	return true;
}

SurgSim::DataStructures::DataGroup NimbleScaffold::buildDeviceInputData()
{
	SurgSim::DataStructures::DataGroupBuilder builder;
	builder.addPose(SurgSim::DataStructures::Names::POSE);
	return builder.createData();
}

std::shared_ptr<SurgSim::Framework::Logger> NimbleScaffold::getLogger() const
{
	return m_logger;
}

std::shared_ptr<NimbleScaffold> NimbleScaffold::getOrCreateSharedInstance()
{
	// Using an explicit creation function gets around problems with accessing the private constructor.
	static auto creator =
		[]() { return std::shared_ptr<NimbleScaffold>(new NimbleScaffold()); };  // NOLINT(readability/braces)
	static SurgSim::Framework::SharedInstance<NimbleScaffold> sharedInstance(creator);
	return sharedInstance.get();
}

}  // namespace Device
}  // namespace SurgSim