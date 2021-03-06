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

// Based on barrier.hpp from Boost 1.51
// Copyright (C) 2002-2003
// David Moore, William E. Kempf
// Copyright (C) 2007-8 Anthony Williams
//
// Which was distributed under the Boost Software License, Version 1.0.
// (See accomanying NOTICES or a copy at http://www.boost.org/LICENSE_1_0.txt)

#include "SurgSim/Framework/Barrier.h"

SurgSim::Framework::Barrier::Barrier(size_t count) :
	m_threshold(count),
	m_count(count),
	m_generation(0),
	m_success(true)
{
	SURGSIM_ASSERT(count != 0) << "Barrier constructor count cannot be zero";
}

bool SurgSim::Framework::Barrier::wait(bool success)
{
	boost::mutex::scoped_lock lock(m_mutex);
	size_t gen = m_generation;
	m_success = m_success && success;

	--m_count;

	if (m_count == 0)
	{
		m_generation++;
		m_count = m_threshold;
		m_successResult = m_success;
		m_success = true;
		m_cond.notify_all();
		return m_successResult;
	}

	while (gen == m_generation)
	{
		m_cond.wait(lock);
	}
	return m_successResult;
}

