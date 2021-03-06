# This file is a part of the OpenSurgSim project.
# Copyright 2012-2013, SimQuest Solutions Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include_directories (
	${gtest_SOURCE_DIR}/include
)

set(UNIT_TEST_SOURCES
	ImplicitSurfaceRenderTests.cpp
	OsgBoxRepresentationRenderTests.cpp
	OsgCameraRenderTests.cpp
	OsgCapsuleRepresentationRenderTests.cpp
	OsgCurveRepresentationRenderTests.cpp
	OsgCylinderRepresentationRenderTests.cpp
	OsgManagerRenderTests.cpp
	OsgMeshRepresentationRenderTests.cpp
	OsgOctreeRepresentationRenderTests.cpp
	OsgPlaneRepresentationRenderTests.cpp
	OsgPointCloudRepresentationRenderTests.cpp
	OsgProgramRenderTests.cpp
	OsgRepresentationRenderTests.cpp
	OsgSceneryRepresentationRenderTests.cpp
	OsgScreenSpaceQuadRenderTests.cpp
	OsgSkeletonRepresentationRenderTests.cpp
	OsgSphereRepresentationRenderTests.cpp
	OsgTextRepresentationRenderTests.cpp
	OsgVectorFieldRepresentationRenderTests.cpp
	OsgViewElementRenderTests.cpp
	PaintBehaviorRenderTests.cpp
	RenderTest.cpp
)

set(UNIT_TEST_HEADERS
	RenderTest.h
)

file(COPY Data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Configure the path for the data files
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.txt.in"
	"${CMAKE_CURRENT_BINARY_DIR}/config.txt"
)

set(LIBS
	SurgSimFramework
	SurgSimGraphics
	SurgSimMath
	SurgSimTesting
	SurgSimBlocks
)

surgsim_add_unit_tests(SurgSimGraphicsRenderTest)

set_target_properties(SurgSimGraphicsRenderTest PROPERTIES FOLDER "Graphics")
