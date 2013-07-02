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
/// Tests for the OsgTexture2d class.

#include <SurgSim/Framework/ApplicationData.h>
#include <SurgSim/Graphics/OsgTexture2d.h>

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

namespace SurgSim
{
namespace Graphics
{

TEST(OsgTexture2dTests, InitTest)
{
	OsgTexture2d texture;

	EXPECT_NE(nullptr, texture.getOsgTexture());

	EXPECT_EQ(nullptr, texture.getOsgTexture()->getImage(0u));
}

TEST(OsgTexture2dTests, SetSizeTest)
{
	OsgTexture2d texture;

	texture.setSize(256, 512);

	int width, height;
	texture.getSize(&width, &height);

	EXPECT_EQ(256, width);
	EXPECT_EQ(512, height);
}

TEST(OsgTexture2dTests, LoadAndClearImageTest)
{
	ASSERT_TRUE(boost::filesystem::exists("Data"));

	std::vector<std::string> paths;
	paths.push_back("Data/OsgTextureTests");
	SurgSim::Framework::ApplicationData data(paths);

	std::string imagePath = data.findFile("CheckerBoard.png");

	ASSERT_NE("", imagePath) << "Could not find image file!";

	// Load the image
	std::shared_ptr<OsgTexture2d> osgTexture = std::make_shared<OsgTexture2d>();
	std::shared_ptr<Texture> texture = osgTexture;

	EXPECT_TRUE(texture->loadImage(imagePath)) << "Failed to load image!";

	EXPECT_EQ(1u, osgTexture->getOsgTexture()->getNumImages());

	EXPECT_NE(nullptr, osgTexture->getOsgTexture()->getImage(0u)) << "Texture should have an image!";

	// Make sure the image has the expected size
	int width, height;
	osgTexture->getSize(&width, &height);
	EXPECT_EQ(512, width);
	EXPECT_EQ(512, height);

	osg::Image* image = osgTexture->getOsgTexture()->getImage(0u);
	EXPECT_EQ(512, image->s());
	EXPECT_EQ(512, image->t());
	EXPECT_EQ(1, image->r());

	// Remove the image
	texture->clearImage();

	EXPECT_EQ(nullptr, osgTexture->getOsgTexture()->getImage(0u)) << "Texture image should have been cleared!";

	// Try to load an image that does not exist
	EXPECT_FALSE(texture->loadImage("NotHere.png")) << "Should not have been able to load image - it does not exist!";
	EXPECT_EQ(nullptr, osgTexture->getOsgTexture()->getImage(0u));
}

}  // namespace Graphics
}  // namespace SurgSim