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

#ifndef SURGSIM_FRAMEWORK_ASSET_H
#define SURGSIM_FRAMEWORK_ASSET_H

#include <string>

namespace SurgSim
{
namespace Framework
{
class Accessible;
class ApplicationData;
class AssetTest;

/// This class is used to facilitate file loading. It uses the static ApplicationData
/// in SurgSim::Framework::Runtime to load file.
/// Classes not in SurgSim::Framework::Component hierarchy should inherit this class in
/// order to load a file.
class Asset
{
	friend AssetTest;
public:
	/// Constructor
	Asset();

	/// Destructor
	virtual ~Asset();

	/// Check the existence of file 'fileName' using 'data'.
	/// If 'fileName' exists, this method calls 'doLoad()' to load the file.
	/// It asserts on loading the file successfully.
	/// \note As a side effect, the name of the file will be recorded in
	/// \note Asset::m_fileName and can be retrieved by Asset::getFileName().
	/// \param fileName Name of the file to be loaded.
	/// \param data ApplicationData which provides the runtime look up path(s).
	void load(const std::string& fileName, const SurgSim::Framework::ApplicationData& data);

	/// Overloaded function using SurgSim::Framework::Runtime::getApplicationData() as look up path(s).
	/// \param fileName Name of the file to be loaded.
	void load(const std::string& fileName);

	/// Return the name of file loaded by this class.
	/// \return Name of the file loaded by this class.
	std::string getFileName() const;

protected:
	/// Derived classes will overwrite this method to do actual loading.
	/// \note This method is not required to do any check on the validity or the existence of the file.
	/// \param filePath Absolute path to the file.
	/// \return True if loading is successful; Otherwise, false.
	virtual bool doLoad(const std::string& filePath) = 0;

	/// Derived classes (which also inherit from SurgSim::Framework::Accessible) should call this function 
	/// with 'this' pointer as the parameter in their constructors to register file name property for serialization.
	/// \param accessible 'this' pointer of derived class.
	void serializeFileName(SurgSim::Framework::Accessible* accesible);

private:
	/// Name of the file to be loaded.
	std::string m_fileName;
};

} // namespace Framework
} // namespace SurgSim

#endif // SURGSIM_FRAMEWORK_ASSET_H