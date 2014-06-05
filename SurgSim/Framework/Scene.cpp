// This file is a part of the OpenSurgSim project.
// Copyright 2013, SimQuest LLC.
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

#include "SurgSim/Framework/Scene.h"

#include "SurgSim/Framework/FrameworkConvert.h"
#include "SurgSim/Framework/Runtime.h"
#include "SurgSim/Framework/SceneElement.h"
#include "SurgSim/Framework/Component.h"
#include "SurgSim/Framework/Log.h"

#include <utility>
#include <set>
#include <vector>
#include <boost/thread/locks.hpp>

#include <yaml-cpp/yaml.h>

namespace SurgSim
{
namespace Framework
{

Scene::Scene(std::weak_ptr<Runtime> runtime) :
	m_runtime(runtime)
{
	SURGSIM_ASSERT(!m_runtime.expired()) << "Can't create scene with empty runtime.";
}

Scene::~Scene()
{

}

void Scene::addSceneElement(std::shared_ptr<SceneElement> element)
{
	SURGSIM_ASSERT(!m_runtime.expired()) << "Runtime pointer is expired, cannot add SceneElement to Scene.";

	std::string name = element->getName();
	element->setScene(getSharedPtr());

	std::shared_ptr<Runtime> runtime = m_runtime.lock();
	element->setRuntime(runtime);

	if (element->initialize())
	{
		{
			boost::lock_guard<boost::mutex> lock(m_sceneElementsMutex);
			m_elements.push_back(element);
		}
		runtime->addSceneElement(element);
	}

}

std::shared_ptr<Runtime> Scene::getRuntime()
{
	return m_runtime.lock();
}

const std::vector <std::shared_ptr<SceneElement>>& Scene::getSceneElements() const
{
	return m_elements;
}

std::shared_ptr<Scene> Scene::getSharedPtr()
{
	std::shared_ptr<Scene> result;
	try
	{
		result = shared_from_this();
	}
	catch (const std::exception&)
	{
		SURGSIM_FAILURE() << "Scene was not created as a shared_ptr";
	}
	return result;
}

YAML::Node Scene::encode() const
{
	YAML::Node result(YAML::NodeType::Map);
	YAML::Node data(YAML::NodeType::Map);
	for (auto sceneElement = m_elements.begin(); sceneElement != m_elements.end(); ++sceneElement)
	{
		data["SceneElements"].push_back(*(*sceneElement));
	}

	result["SurgSim::Framework::Scene"] = data;

	return result;
}

bool Scene::decode(const YAML::Node& node)
{
	bool result = false;
	if (node.IsMap())
	{
		YAML::Node data = node["SurgSim::Framework::Scene"];

		if (data["SceneElements"].IsDefined())
		{
			auto sceneElements = data["SceneElements"].as<std::vector<std::shared_ptr<SceneElement>>>();

			std::for_each(sceneElements.begin(), sceneElements.end(),
						  [&](std::shared_ptr<SceneElement> element)
			{
				addSceneElement(element);
			});
		}
		result = true;
	}
	return result;
}
}; // namespace Framework
}; // namespace SurgSim

