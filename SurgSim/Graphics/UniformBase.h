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

#ifndef SURGSIM_GRAPHICS_UNIFORMBASE_H
#define SURGSIM_GRAPHICS_UNIFORMBASE_H

#include "SurgSim/Framework/Accessible.h"

namespace SurgSim
{

namespace Graphics
{

/// Common base class for all graphics uniforms.
///
/// Graphics uniforms act as parameters to shader programs.
/// \note
/// SurgSim::Graphics::Uniform is templated on the type of value, so this base class allows a pointer to any type
/// of Uniform.
class UniformBase : public SurgSim::Framework::Accessible
{
public:
	/// Destructor
	virtual ~UniformBase() = 0;
};

inline UniformBase::~UniformBase()
{
}

};  // namespace Graphics

};  // namespace SurgSim

#endif  // SURGSIM_GRAPHICS_UNIFORMBASE_H
