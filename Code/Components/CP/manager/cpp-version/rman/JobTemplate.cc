/// @file JobTemplate.cc
///
/// @copyright (c) 2010 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include own header file first
#include "rman/JobTemplate.h"

// System includes
#include <string>
#include <vector>

// Local package includes
#include "IJob.h"

using namespace askap::cp::manager;

JobTemplate::JobTemplate(const std::string& name) : itsName(name)
{
}

JobTemplate::~JobTemplate()
{
}

void JobTemplate::setName(const std::string& name)
{
    itsName = name;
}

std::string JobTemplate::getName(void) const
{
    return itsName;
}

void JobTemplate::setScriptLocation(const std::string& script)
{
    itsPathToScript = script;
}

std::string JobTemplate::getScriptLocation(void) const
{
    return itsPathToScript;
}

void JobTemplate::addDependency(const IJob& dependency, DependType type)
{
    itsDependencies[dependency.getId()] = type;
}

void JobTemplate::removeDependency(const IJob& dependency)
{
    itsDependencies.erase(dependency.getId());
}

void JobTemplate::removeAllDependencies(void)
{
    itsDependencies.clear();
}

std::map<std::string, JobTemplate::DependType> JobTemplate::getDependencies(void) const
{
    return itsDependencies;
}
