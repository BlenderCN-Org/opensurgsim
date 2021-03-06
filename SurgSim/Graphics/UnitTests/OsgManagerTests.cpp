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
/// Tests for the OsgManager class.

#include "SurgSim/Graphics/UnitTests/MockObjects.h"
#include "SurgSim/Graphics/UnitTests/MockOsgObjects.h"

#include "SurgSim/Framework/Runtime.h"

#include "SurgSim/Graphics/OsgCamera.h"
#include "SurgSim/Graphics/OsgGroup.h"
#include "SurgSim/Graphics/OsgManager.h"
#include "SurgSim/Graphics/OsgRepresentation.h"
#include "SurgSim/Graphics/OsgView.h"
#include "SurgSim/Graphics/OsgViewElement.h"

#include <gtest/gtest.h>

#include <memory>
#include <algorithm>
#include <random>

using SurgSim::Framework::Runtime;
using SurgSim::Framework::ComponentManager;
using SurgSim::Framework::Representation;
using SurgSim::Framework::Component;


namespace SurgSim
{
namespace Graphics
{


class OsgManagerTest : public ::testing::Test
{
public:
	virtual void SetUp()
	{
		runtime = std::make_shared<Runtime>();
		graphicsManager = std::make_shared<OsgManager>();

		runtime->addManager(graphicsManager);
	}

	virtual void TearDown()
	{
		runtime->stop();
	}


	bool testDoAddComponent(const std::shared_ptr<Component>& component)
	{
		return graphicsManager->executeAdditions(component);
	}

	bool testDoRemoveComponent(const std::shared_ptr<Component>& component)
	{
		return graphicsManager->executeRemovals(component);
	}

	void doProcessComponents()
	{
		graphicsManager->processComponents();
	}

	std::shared_ptr<Runtime> runtime;
	std::shared_ptr<OsgManager> graphicsManager;
};

TEST_F(OsgManagerTest, InitTest)
{
	ASSERT_NO_THROW({std::shared_ptr<OsgManager> manager = std::make_shared<OsgManager>();});
}

bool hasView(osgViewer::CompositeViewer* compositeViewer, osg::View* view)
{
	bool foundView = false;
	for (unsigned int i = 0; i < compositeViewer->getNumViews(); ++i)
	{
		if (compositeViewer->getView(i) == view)
		{
			foundView = true;
		}
	}
	return foundView;
}

TEST_F(OsgManagerTest, AddRemoveTest)
{
	osgViewer::CompositeViewer* compositeViewer = graphicsManager->getOsgCompositeViewer();
	/// Perform add and remove from a pointer to a ComponentManager to check that the intended polymorphism is working.
	std::shared_ptr<ComponentManager> componentManager = graphicsManager;

	std::shared_ptr<OsgRepresentation> representation1 =
		std::make_shared<MockOsgRepresentation>("test representation 1");
	std::shared_ptr<OsgRepresentation> representation2 =
		std::make_shared<MockOsgRepresentation>("test representation 2");
	std::shared_ptr<OsgGroup> group1 = std::make_shared<OsgGroup>("test group 1");
	std::shared_ptr<OsgGroup> group2 = std::make_shared<OsgGroup>("test group 2");

	std::shared_ptr<OsgCamera> camera = std::make_shared<OsgCamera>("test camera");

	std::shared_ptr<OsgView> view1 = std::make_shared<OsgView>("test view 1");
	view1->setCamera(camera);
	std::shared_ptr<OsgView> view2 = std::make_shared<OsgView>("test view 2");
	view2->setCamera(camera);

	std::shared_ptr<MockRepresentation> nonOsgRepresentation
		= std::make_shared<MockRepresentation>("non-osg representation");
	std::shared_ptr<MockGroup> nonOsgGroup = std::make_shared<MockGroup>("non-osg group");
	std::shared_ptr<MockView> nonOsgView = std::make_shared<MockView>("non-osg view");
	using SurgSim::Framework::Representation;
	std::shared_ptr<Representation> nonGraphicsComponent = std::make_shared<NonGraphicsRepresentation>(
				"non-graphics component");

	EXPECT_EQ(0u, graphicsManager->getRepresentations().size());
	EXPECT_EQ(0u, graphicsManager->getGroups().size());
	EXPECT_EQ(0u, graphicsManager->getViews().size());

	/// Add an representation
	EXPECT_TRUE(componentManager->enqueueAddComponent(representation1));
	doProcessComponents();
	EXPECT_EQ(1u, graphicsManager->getRepresentations().size());
	EXPECT_EQ(1u, graphicsManager->getGroups().size());
	EXPECT_NE(graphicsManager->getRepresentations().end(), std::find(graphicsManager->getRepresentations().begin(),
			  graphicsManager->getRepresentations().end(), representation1));

	/// Add a view
	EXPECT_TRUE(componentManager->enqueueAddComponent(view1));
	doProcessComponents();
	EXPECT_EQ(1u, graphicsManager->getViews().size());
	EXPECT_NE(graphicsManager->getViews().end(), std::find(graphicsManager->getViews().begin(),
			  graphicsManager->getViews().end(), view1));
	EXPECT_TRUE(hasView(compositeViewer, view1->getOsgView()));

	/// Add another view
	EXPECT_TRUE(componentManager->enqueueAddComponent(view2));
	doProcessComponents();
	EXPECT_EQ(2u, graphicsManager->getViews().size());
	EXPECT_NE(graphicsManager->getViews().end(), std::find(graphicsManager->getViews().begin(),
			  graphicsManager->getViews().end(), view2));

	/// Add another representation
	EXPECT_TRUE(componentManager->enqueueAddComponent(representation2));
	doProcessComponents();
	EXPECT_EQ(2u, graphicsManager->getRepresentations().size());
	EXPECT_NE(graphicsManager->getRepresentations().end(), std::find(graphicsManager->getRepresentations().begin(),
			  graphicsManager->getRepresentations().end(), representation2));


	/// Try to add a duplicate representation
	EXPECT_TRUE(componentManager->enqueueAddComponent(representation1));
	doProcessComponents();
	EXPECT_EQ(2u, graphicsManager->getRepresentations().size());

	/// Try to add a duplicate view
	EXPECT_TRUE(componentManager->enqueueAddComponent(view1));
	doProcessComponents();
	EXPECT_EQ(2u, graphicsManager->getViews().size());

	/// Try to add an representation that is not a subclass of OsgRepresentation
	EXPECT_TRUE(componentManager->enqueueAddComponent(nonOsgRepresentation)) <<
			"Adding an Representation that is not a subclass of OsgRepresentation should fail and return false";
	doProcessComponents();
	EXPECT_EQ(2u, graphicsManager->getRepresentations().size());

	/// Try to add a view that is not a subclass of OsgView
	EXPECT_TRUE(componentManager->enqueueAddComponent(nonOsgView)) <<
			"Adding a View that is not a subclass of OsgView should fail and return false";
	doProcessComponents();
	EXPECT_EQ(2u, graphicsManager->getViews().size());

	/// Try to add a component that is not graphics-related
	EXPECT_TRUE(componentManager->enqueueAddComponent(nonGraphicsComponent)) <<
			"Adding a component that this manager is not concerned with should return true";

	/// Remove a view
	EXPECT_TRUE(componentManager->enqueueRemoveComponent(view2));
	doProcessComponents();
	EXPECT_EQ(graphicsManager->getViews().end(), std::find(graphicsManager->getViews().begin(),
			  graphicsManager->getViews().end(), view2));

	/// Remove an representation
	EXPECT_TRUE(componentManager->enqueueRemoveComponent(representation1));
	doProcessComponents();
	EXPECT_EQ(graphicsManager->getRepresentations().end(), std::find(graphicsManager->getRepresentations().begin(),
			  graphicsManager->getRepresentations().end(), representation1));

	/// Try to remove an representation that is not in the manager
	EXPECT_TRUE(componentManager->enqueueRemoveComponent(representation1));
	doProcessComponents();
	EXPECT_EQ(graphicsManager->getRepresentations().end(), std::find(graphicsManager->getRepresentations().begin(),
			  graphicsManager->getRepresentations().end(), representation1));

	/// Try to remove a view that is not in the manager
	EXPECT_TRUE(componentManager->enqueueRemoveComponent(view2));
	doProcessComponents();
	EXPECT_EQ(graphicsManager->getViews().end(), std::find(graphicsManager->getViews().begin(),
			  graphicsManager->getViews().end(), view2));


	/// Try to remove a component that is not graphics-related
	EXPECT_TRUE(componentManager->enqueueRemoveComponent(nonGraphicsComponent)) <<
			"Removing a component that this manager is not concerned with should return true";
}

TEST_F(OsgManagerTest, LazyGroupsTest)
{
	std::shared_ptr<OsgRepresentation> representation1 =
		std::make_shared<MockOsgRepresentation>("TestRepresentation_1");
	std::shared_ptr<OsgRepresentation> representation2 =
		std::make_shared<MockOsgRepresentation>("TestRepresentation_2");
	std::shared_ptr<OsgRepresentation> representation3 =
		std::make_shared<MockOsgRepresentation>("TestRepresentation_3");

	representation1->addGroupReference("TestGroup_1");
	representation1->addGroupReference("TestGroup_2");
	representation1->addGroupReference("TestGroup_3");
	representation2->addGroupReference("TestGroup_2");
	representation2->addGroupReference("TestGroup_3");
	representation3->addGroupReference("TestGroup_3");

	graphicsManager->enqueueAddComponent(representation1);
	graphicsManager->enqueueAddComponent(representation2);
	doProcessComponents();

	auto defaultGroup = graphicsManager->getGroups().at(SurgSim::Graphics::Representation::DefaultGroupName);
	auto group1 = graphicsManager->getGroups().at("TestGroup_1");
	auto group2 = graphicsManager->getGroups().at("TestGroup_2");


	EXPECT_EQ(2U, defaultGroup->getMembers().size());
	EXPECT_EQ(1U, group1->getMembers().size());
	EXPECT_EQ(2U, group2->getMembers().size());

	graphicsManager->enqueueAddComponent(representation3);
	doProcessComponents();

	auto group3 = graphicsManager->getGroups().at("TestGroup_3");

	EXPECT_EQ(3U, defaultGroup->getMembers().size());
	EXPECT_EQ(1U, group1->getMembers().size());
	EXPECT_EQ(2U, group2->getMembers().size());
	EXPECT_EQ(3U, group3->getMembers().size());

}

}; // namespace Graphics
}; // namespace SurgSim


