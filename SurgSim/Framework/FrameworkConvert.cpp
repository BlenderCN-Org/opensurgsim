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

#include "SurgSim/Framework/FrameworkConvert.h"
#include "SurgSim/Framework/Component.h"

#include <vector>

#include <boost/uuid/uuid_io.hpp>

namespace {
	const std::string NamePropertyName = "Name";
	const std::string IdPropertyName = "Id";
}

namespace YAML
{
Node convert<std::shared_ptr<SurgSim::Framework::Component>>::encode(
	const std::shared_ptr<SurgSim::Framework::Component> rhs)
{
	Node data;
	data[IdPropertyName] = to_string(rhs->getUuid());
	data[NamePropertyName] = rhs->getName();
	Node result;
	result[rhs->getClassName()] = data;

	return result;
}

bool convert<std::shared_ptr<SurgSim::Framework::Component>>::decode(const Node& node,
	std::shared_ptr<SurgSim::Framework::Component>& rhs)
{
	bool result = false;

	if (!node.IsMap())
	{
		return false;
	}

	Node data = node.begin()->second;
	std::string className = node.begin()->first.as<std::string>();

	if (data.IsMap() &&
		data[IdPropertyName].IsDefined() &&
		data[NamePropertyName].IsDefined())
	{
		if (rhs == nullptr)
		{
			std::string id = data[IdPropertyName].as<std::string>();
			RegistryType& registry = getRegistry();
			auto sharedComponent = registry.find(id);
			if ( sharedComponent != registry.end())
			{
				SURGSIM_ASSERT(data[NamePropertyName].as<std::string>() == sharedComponent->second->getName() &&
						className == sharedComponent->second->getClassName()) <<
						"The current node: " << std::endl << node << "has the same id as an instance " <<
						"already registered, but the name and/or the className are different. This is " <<
						"likely a problem with a manually assigned id.";
				rhs = sharedComponent->second;
			}
			else
			{
				SurgSim::Framework::Component::FactoryType& factory =
					SurgSim::Framework::Component::getFactory();

				if (factory.isRegistered(className))
				{
					rhs = factory.create(className, data[NamePropertyName].as<std::string>());
					getRegistry()[id] = rhs;
				}
				else
				{
					SURGSIM_FAILURE() << "Class " << className << " is not registered in the factory.";
				}
			}
		}
		rhs->decode(data);
		result = true;
	}
	return result;
}

convert<std::shared_ptr<SurgSim::Framework::Component>>::RegistryType&
	convert<std::shared_ptr<SurgSim::Framework::Component>>::getRegistry()
{
	static RegistryType registry;
	return registry;
}

Node convert<SurgSim::Framework::Component>::encode(const SurgSim::Framework::Component& rhs)
{
	YAML::Node result(YAML::NodeType::Map);
	YAML::Node data(rhs.encode());
	data[IdPropertyName] = to_string(rhs.getUuid());
	data[NamePropertyName] = rhs.getName();
	result[rhs.getClassName()] = data;
	return result;
}

}