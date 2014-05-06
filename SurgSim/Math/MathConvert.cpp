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

#include "SurgSim/Math/MathConvert.h"

#include "SurgSim/Math/Shape.h"

namespace YAML
{

Node convert<std::shared_ptr<SurgSim::Math::Shape>>::encode(
	const std::shared_ptr<SurgSim::Math::Shape>& rhs)
{
	SURGSIM_ASSERT(nullptr != rhs) << "Trying to encode nullptr SurgSim::Math::Shape";
	Node result;
	result[rhs->getClassName()] = rhs->encode();

	return result;
}

bool convert<std::shared_ptr<SurgSim::Math::Shape>>::decode(
	const Node& node,
	std::shared_ptr<SurgSim::Math::Shape>& rhs)
{
	bool result = false;

	if (node.IsMap())
	{
		if (nullptr == rhs)
		{
			std::string className = node.begin()->first.as<std::string>();
			SurgSim::Math::Shape::FactoryType& factory = SurgSim::Math::Shape::getFactory();

			if (factory.isRegistered(className))
			{
				rhs = factory.create(className);
			}
			else
			{
				SURGSIM_FAILURE() << "Class " << className << " is not registered in the factory.";
			}
		}

		Node data = node.begin()->second;
		if (data.IsMap())
		{
			rhs->decode(data);
		}

		result = true;
	}

	return result;
}

}; // namespace YAML