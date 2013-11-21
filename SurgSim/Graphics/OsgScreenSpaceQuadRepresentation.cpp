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

#include <memory>

#include <SurgSim/Graphics/OsgScreenSpaceQuadRepresentation.h>
#include <SurgSim/Graphics/OsgRigidTransformConversions.h>
#include <SurgSim/Graphics/View.h>
#include <SurgSim/Graphics/OsgUniform.h>
#include <SurgSim/Graphics/OsgUniformBase.h>
#include <SurgSim/Graphics/OsgMaterial.h>
#include <SurgSim/Graphics/Texture2d.h>
#include <SurgSim/Graphics/TextureRectangle.h>

#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Projection>
#include <osg/StateAttribute>
#include <osg/Switch>

namespace SurgSim
{
namespace Graphics
{

OsgScreenSpaceQuadRepresentation::OsgScreenSpaceQuadRepresentation(
	const std::string& name,
	std::shared_ptr<View> view) :
	Representation(name),
	OsgRepresentation(name),
	ScreenSpaceQuadRepresentation(name,view),
	m_view(view),
	m_scale(1.0,1.0,1.0)
{
	m_switch = new osg::Switch;
	m_switch->setName(name + " Switch");

	m_transform = new osg::PositionAttitudeTransform();
	m_transform->setName(name + " Transform");

	m_view->getDimensions(&m_displayWidth, &m_displayHeight);

	m_geode = new osg::Geode;

	// Make the quad
	float depth = 0.0;
	m_geometry = osg::createTexturedQuadGeometry(
		osg::Vec3(0.0, 0.0, depth),
		osg::Vec3(1.0, 0.0, depth),
		osg::Vec3(0.0, 1.0, depth));

	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
	m_geometry->setColorArray(colors);
	m_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	m_geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	//osg::StateSet* stateset = geom->getOrCreateStateSet();
	//stateset->setMode(GL_BLEND,osg::StateAttribute::ON);

	m_geode->addDrawable(m_geometry);

	m_transform->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	m_transform->setCullingActive(false);
	m_transform->addChild(m_geode);

	// Need to keep this around to modify the ortho projection when the view changes
	m_projection = new osg::Projection;
	m_projection->setMatrix(osg::Matrix::ortho2D(0,m_displayWidth,0,m_displayHeight));
	m_projection->addChild(m_transform);


	m_switch->addChild(m_projection);
}

OsgScreenSpaceQuadRepresentation::~OsgScreenSpaceQuadRepresentation()
{

}

void OsgScreenSpaceQuadRepresentation::doUpdate(double dt)
{
	int width, height;
	m_view->getDimensions(&width, &height);
	if (width != m_displayWidth || height != m_displayHeight)
	{
		m_displayWidth = width;
		m_displayHeight = height;
		m_projection->setMatrix(osg::Matrix::ortho2D(0, m_displayWidth, 0, m_displayHeight));
	}
}

void OsgScreenSpaceQuadRepresentation::setSize(double width, double height)
{
	m_scale.x() = width;
	m_scale.y() = height;
	m_transform->setScale(m_scale);
}

void OsgScreenSpaceQuadRepresentation::getSize(double* width, double* height) const
{
	SURGSIM_ASSERT(width != nullptr && height  != nullptr) << "Cannot use a nullptr as an output parameter";
	*width = m_scale.x();
	*height = m_scale.y();
}

void OsgScreenSpaceQuadRepresentation::setPose(const SurgSim::Math::RigidTransform3d& transform)
{
	// HS-2013-jun-28 This function should probably be protected by a mutes, but I can see
	// the assumption could be that this is only called from on thread.
	// #threadsafety
	m_pose = transform;
	std::pair<osg::Quat, osg::Vec3d> pose = toOsg(m_pose);
	m_transform->setPosition(pose.second);
}

bool OsgScreenSpaceQuadRepresentation::setTexture(std::shared_ptr<Texture> texture)
{
	SURGSIM_ASSERT(texture != nullptr) << "Null texture passed to setTexture";
	std::shared_ptr<OsgTexture2d> osgTexture2d = std::dynamic_pointer_cast<OsgTexture2d>(texture);
	if (osgTexture2d != nullptr)
	{
		return setTexture(osgTexture2d);
	}

	std::shared_ptr<OsgTextureRectangle> osgTextureRectangle = std::dynamic_pointer_cast<OsgTextureRectangle>(texture);
	if (osgTextureRectangle != nullptr)
	{
		return setTexture(osgTextureRectangle);
	}

	return false;

}

bool OsgScreenSpaceQuadRepresentation::setTexture(std::shared_ptr<OsgTexture2d> osgTexture)
{
	bool result = false;
	auto newUniform = std::make_shared<OsgUniform<std::shared_ptr<OsgTexture2d>>>("diffuseMap");
	newUniform->set(osgTexture);
	if (replaceUniform("diffuseMap", newUniform))
	{
		setTextureCoordinates(0.0,0.0,1.0,1.0);
		result = true;
	}
	return result;
}

bool OsgScreenSpaceQuadRepresentation::setTexture(std::shared_ptr<OsgTextureRectangle> osgTexture)
{
	bool result = false;
	auto newUniform = std::make_shared<OsgUniform<std::shared_ptr<OsgTextureRectangle>>>("diffuseMap");
	newUniform->set(osgTexture);
	if (replaceUniform("diffuseMap", newUniform))
	{
		int width, height;
		osgTexture->getSize(&width, &height);
		setTextureCoordinates(0.0,0.0,static_cast<float>(width),static_cast<float>(height));
		result = true;
	}
	return result;
}



bool OsgScreenSpaceQuadRepresentation::replaceUniform(const std::string& name, std::shared_ptr<UniformBase> newUniform)
{
	std::shared_ptr<OsgMaterial> material = std::dynamic_pointer_cast<OsgMaterial>(getMaterial());
	if (material == nullptr)
	{
		material = std::make_shared<OsgMaterial>();
		setMaterial(material);
	}

	std::shared_ptr<UniformBase> oldUniform = material->getUniform(name);

	if (oldUniform != nullptr)
	{
		material->removeUniform(oldUniform);
	}

	bool result = material->addUniform(newUniform);

	// if the add failed try to add the old one back on but report failure
	if (! result && oldUniform != nullptr)
	{
		material->addUniform(oldUniform);
	}

	return result;
}

void OsgScreenSpaceQuadRepresentation::setTextureCoordinates(float left, float bottom, float right, float top)
{
	osg::Vec2Array* tcoords = new osg::Vec2Array(4);
	(*tcoords)[0].set(left,top);
	(*tcoords)[1].set(left,bottom);
	(*tcoords)[2].set(right,bottom);
	(*tcoords)[3].set(right,top);
	m_geometry->setTexCoordArray(0,tcoords);
}

void OsgScreenSpaceQuadRepresentation::setLocation(double x, double y)
{
	SurgSim::Math::RigidTransform3d transform =
		SurgSim::Math::makeRigidTransform(SurgSim::Math::Quaterniond::Identity(), SurgSim::Math::Vector3d(x,y,0));
	setPose(transform);
}

void OsgScreenSpaceQuadRepresentation::getLocation(double* x, double* y)
{
	SURGSIM_ASSERT( x !=  nullptr && y != nullptr) << "Cannot use a nulptr as an output paramter.";
	SurgSim::Math::Vector3d position = getPose().translation();
	*x = position.x();
	*y = position.y();
}



}; // Graphics
}; // SurgSim
