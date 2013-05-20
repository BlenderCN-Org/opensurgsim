// This file is a part of the OpenSurgSim project.
// Copyright 2012-2013, SimQuest Solutions Inc.
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

/// \file
/// Tests for the Graphics Manager class.

#include "SurgSim/Framework/Runtime.h"
#include "SurgSim/Framework/Scene.h"
#include "SurgSim/Graphics/ViewElement.h"
#include "SurgSim/Graphics/UnitTests/MockObjects.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <random>

using SurgSim::Framework::ComponentManager;
using SurgSim::Framework::Representation;
using SurgSim::Framework::Runtime;
using SurgSim::Framework::Scene;
using SurgSim::Framework::SceneElement;
using SurgSim::Graphics::Actor;
using SurgSim::Graphics::Camera;
using SurgSim::Graphics::ViewElement;

TEST(ManagerTests, InitTest)
{
	ASSERT_NO_THROW({std::shared_ptr<MockManager> manager = std::make_shared<MockManager>();});
}

TEST(ManagerTests, StartUpTest)
{
	std::shared_ptr<Runtime> runtime = std::make_shared<Runtime>();
	std::shared_ptr<MockManager> manager = std::make_shared<MockManager>();

	runtime->addManager(manager);
	EXPECT_EQ(0, manager->getNumUpdates());
	EXPECT_EQ(0.0, manager->getSumDt());

	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	runtime->setScene(scene);

	/// Add a graphics component to the scene
	std::shared_ptr<MockView> view = std::make_shared<MockView>("test component");
	std::shared_ptr<ViewElement> viewElement = std::make_shared<ViewElement>("test element", view);
	scene->addSceneElement(viewElement);

	/// Run the thread for a moment
	runtime->start();
	EXPECT_TRUE(manager->isInitialized());
	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	runtime->stop();

	/// Check that the manager did update when the thread was running
	EXPECT_GT(0, manager->getNumUpdates());
	EXPECT_GT(0.0, manager->getSumDt());

	EXPECT_TRUE(view->isInitialized());
	EXPECT_TRUE(view->isAwoken());
	EXPECT_GT(0, view->getNumUpdates());
	EXPECT_EQ(manager->getNumUpdates(), view->getNumUpdates());
}

TEST(ManagerTests, AddRemoveTest)
{
	std::shared_ptr<MockManager> graphicsManager = std::make_shared<MockManager>();
	/// Perform add and remove from a pointer to a ComponentManager to check that the intended polymorphism is working.
	std::shared_ptr<ComponentManager> componentManager = graphicsManager;

	std::shared_ptr<Actor> actor1 = std::make_shared<MockActor>("test actor 1");
	std::shared_ptr<Actor> actor2 = std::make_shared<MockActor>("test actor 2");
	std::shared_ptr<MockGroup> group1 = std::make_shared<MockGroup>("test group 1");
	std::shared_ptr<MockGroup> group2 = std::make_shared<MockGroup>("test group 2");
	std::shared_ptr<MockView> view1 = std::make_shared<MockView>("test view 1");
	std::shared_ptr<MockView> view2 = std::make_shared<MockView>("test view 2");
	std::shared_ptr<Representation> nonGraphicsComponent = std::make_shared<Representation>("non-graphics component");

	EXPECT_EQ(0, graphicsManager->getActors().size());
	EXPECT_EQ(0, graphicsManager->getGroups().size());
	EXPECT_EQ(0, graphicsManager->getViews().size());

	/// Add an actor
	EXPECT_TRUE(graphicsManager->addComponent(actor1));
	EXPECT_EQ(1, graphicsManager->getActors().size());
	EXPECT_NE(graphicsManager->getActors().end(), std::find(graphicsManager->getActors().begin(),
		graphicsManager->getActors().end(), actor1));

	/// Add a group
	EXPECT_TRUE(graphicsManager->addComponent(group1));
	EXPECT_EQ(1, graphicsManager->getGroups().size());
	EXPECT_NE(graphicsManager->getGroups().end(), std::find(graphicsManager->getGroups().begin(),
		graphicsManager->getGroups().end(), group1));

	/// Add a view
	EXPECT_TRUE(graphicsManager->addComponent(view1));
	EXPECT_EQ(1, graphicsManager->getViews().size());
	EXPECT_NE(graphicsManager->getViews().end(), std::find(graphicsManager->getViews().begin(),
		graphicsManager->getViews().end(), view1));


	/// Add another view
	EXPECT_TRUE(graphicsManager->addComponent(view2));
	EXPECT_EQ(2, graphicsManager->getViews().size());
	EXPECT_NE(graphicsManager->getViews().end(), std::find(graphicsManager->getViews().begin(),
		graphicsManager->getViews().end(), view2));

	/// Add another group
	EXPECT_TRUE(graphicsManager->addComponent(group2));
	EXPECT_EQ(2, graphicsManager->getGroups().size());
	EXPECT_NE(graphicsManager->getGroups().end(), std::find(graphicsManager->getGroups().begin(),
		graphicsManager->getGroups().end(), group2));

	/// Add another actor
	EXPECT_TRUE(graphicsManager->addComponent(actor2));
	EXPECT_EQ(2, graphicsManager->getActors().size());
	EXPECT_NE(graphicsManager->getActors().end(), std::find(graphicsManager->getActors().begin(),
		graphicsManager->getActors().end(), actor2));


	/// Try to add a duplicate actor
	EXPECT_FALSE(componentManager->addComponent(actor1));
	EXPECT_EQ(2, graphicsManager->getActors().size());

	/// Try to add a duplicate group
	EXPECT_FALSE(componentManager->addComponent(group2));
	EXPECT_EQ(2, graphicsManager->getGroups().size());

	/// Try to add a duplicate view
	EXPECT_FALSE(componentManager->addComponent(view1));
	EXPECT_EQ(2, graphicsManager->getViews().size());

	/// Try to add a component that is not graphics-related
	EXPECT_TRUE(componentManager->addComponent(nonGraphicsComponent)) <<
		"Adding a component that this manager is not concerned with should return true";


	/// Remove a group
	EXPECT_TRUE(componentManager->removeComponent(group2));
	EXPECT_EQ(graphicsManager->getGroups().end(), std::find(graphicsManager->getGroups().begin(),
		graphicsManager->getGroups().end(), group2));

	/// Remove a view
	EXPECT_TRUE(componentManager->removeComponent(view2));
	EXPECT_EQ(graphicsManager->getViews().end(), std::find(graphicsManager->getViews().begin(),
		graphicsManager->getViews().end(), view2));

	/// Remove an actor
	EXPECT_TRUE(componentManager->removeComponent(actor1));
	EXPECT_EQ(graphicsManager->getActors().end(), std::find(graphicsManager->getActors().begin(),
		graphicsManager->getActors().end(), actor1));

	/// Try to remove a group that is not in the manager
	EXPECT_FALSE(componentManager->removeComponent(group2));
	EXPECT_EQ(graphicsManager->getGroups().end(), std::find(graphicsManager->getGroups().begin(),
		graphicsManager->getGroups().end(), group2));

	/// Try to remove an actor that is not in the manager
	EXPECT_FALSE(componentManager->removeComponent(actor1));
	EXPECT_EQ(graphicsManager->getActors().end(), std::find(graphicsManager->getActors().begin(),
		graphicsManager->getActors().end(), actor1));

	/// Try to remove a view that is not in the manager
	EXPECT_FALSE(componentManager->removeComponent(view2));
	EXPECT_EQ(graphicsManager->getViews().end(), std::find(graphicsManager->getViews().begin(),
		graphicsManager->getViews().end(), view2));


	/// Try to remove a component that is not graphics-related
	EXPECT_TRUE(componentManager->removeComponent(nonGraphicsComponent)) <<
		"Removing a component that this manager is not concerned with should return true";
}