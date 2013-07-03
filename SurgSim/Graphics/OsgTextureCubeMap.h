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

#ifndef SURGSIM_GRAPHICS_OSGTEXTURECUBEMAP_H
#define SURGSIM_GRAPHICS_OSGTEXTURECUBEMAP_H

#include <SurgSim/Graphics/OsgTexture.h>

#include <osg/TextureCubeMap>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4250)
#endif

namespace SurgSim
{

namespace Graphics
{

/// OSG implementation of a Cube Map Texture
///
/// Wraps an osg::TextureCubeMap
class OsgTextureCubeMap : public OsgTexture
{
public:
	/// Constructor
	/// \post	No image is loaded in the texture.
	OsgTextureCubeMap();

	/// Sets the size of the texture
	/// \param	width	Width of the texture
	/// \param	height	Height of the texture
	/// \note	Use this to setup a texture as a render target rather than loading from file.
	virtual void setSize(int width, int height);

	/// Gets the size of the texture
	/// \param[out]	width	Width of the texture
	/// \param[out]	height	Height of the texture
	virtual void getSize(int* width, int* height) const;

	/// Loads an image into the texture from a file
	/// \param	filePath	Path to the image file
	/// \return	True if the image is successfully loaded, otherwise false
	/// \note
	/// The image should contain the cube map unwrapped such that each face is width/3 x height/4 and the corners
	/// for the faces are arranged as follows, with (0, 0) as the bottom-left corner and (width, height) as the
	/// top-right corner:
	/// (-Z): (width * 1/3, 0           ) to (width * 2/3, height * 1/4)
	/// (-Y): (width * 1/3, height * 1/4) to (width * 2/3, height * 1/2)
	/// (-X): (0,           height * 1/2) to (width * 1/3, height * 3/4)
	/// (+Z): (width * 1/3, height * 1/2) to (width * 2/3, height * 3/4)
	/// (+X): (width * 2/3, height * 1/2) to (width,       height * 3/4)
	/// (+Y): (width * 1/3, height * 3/4) to (width * 2/3, height      )
	virtual bool loadImage(const std::string& filePath);

	/// Loads images from files into the faces of the cube map
	/// \param	negativeX	Path to the image for the (-X) face
	/// \param	positiveX	Path to the image for the (+X) face
	/// \param	negativeY	Path to the image for the (-Y) face
	/// \param	positiveY	Path to the image for the (+Y) face
	/// \param	negativeZ	Path to the image for the (-Z) face
	/// \param	positiveZ	Path to the image for the (+Z) face
	/// \return	True if the image is successfully loaded, otherwise false
	virtual bool loadImageFaces(const std::string& negativeX, const std::string& positiveX,
		const std::string& negativeY, const std::string& positiveY,
		const std::string& negativeZ, const std::string& positiveZ);

	/// Returns the osg::TextureCubeMap
	osg::ref_ptr<osg::TextureCubeMap> getOsgTextureCubeMap() const
	{
		return static_cast<osg::TextureCubeMap*>(getOsgTexture().get());
	}

protected:
	/// Makes a copy of an image block
	/// \param	source	Source image to copy from
	/// \param	column	First column of block in the source image
	/// \param	row	First row of block in the source image
	/// \param	width	Width of the block
	/// \param	height	Height of the block
	/// \return	Copy of the image block
	osg::ref_ptr<osg::Image> copyImageBlock(const osg::Image& source, int column, int row, int width, int height);
};

};  // namespace Graphics

};  // namespace SurgSim

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // SURGSIM_GRAPHICS_OSGTEXTURECUBEMAP_H
